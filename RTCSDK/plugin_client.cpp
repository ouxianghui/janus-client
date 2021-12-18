/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#include "plugin_client.h"
#include "utils/task_scheduler.h"
#include "utils/thread_provider.h"
#include "logger/logger.h"
#include "helper_utils.h"
#include "api/jsep.h"
#include "webrtc_utils.h"
#include "api/peer_connection_interface.h"
#include "api/rtp_transceiver_interface.h"
#include "api/rtp_sender_interface.h"
#include "api/rtp_receiver_interface.h"
#include "api/media_stream_interface.h"
#include "api/create_peerconnection_factory.h"
#include "api/video_codecs/builtin_video_decoder_factory.h"
#include "api/video_codecs/builtin_video_encoder_factory.h"
#include "api/audio_codecs/builtin_audio_decoder_factory.h"
#include "api/audio_codecs/builtin_audio_encoder_factory.h"
#include "api/media_types.h"
#include "api/rtp_transceiver_interface.h"
#include "modules/audio_device/include/audio_device.h"
#include "modules/audio_processing/include/audio_processing.h"
#include "modules/video_capture/video_capture_factory.h"
#include "pc/video_track_source.h"
#include "video_capture.h"
#include "service/app_instance.h"
#include "rtc_base/thread.h"
#include "logger/logger.h"
#include "service/app_instance.h"
#include "utils/thread_provider.h"
#include "utils/task_scheduler.h"
#include "message_models.h"
#include "sdp_utils.h"
#include "absl/types/optional.h"

namespace vi {
	PluginClient::PluginClient(std::shared_ptr<SignalingServiceInterface> ss)
	{
		_pluginContext = std::make_shared<PluginContext>(ss);

		_rtcStatsTaskScheduler = TaskScheduler::create();
	}

	PluginClient::~PluginClient()
	{
		DLOG("~PluginClient()");
		stopStatsMonitor();
		//TMgr->thread("")->PostTask(RTC_FROM_HERE, []() {

		//});
	}

	void PluginClient::init()
	{
		_eventHandlerThread = rtc::Thread::Current();

		_pluginContext->iceServers.emplace_back("stun:stun.l.google.com:19302");

		if (!_pluginContext->pcf) {
			_pluginContext->signaling = rtc::Thread::Create();
			_pluginContext->signaling->SetName("pc_signaling_thread", nullptr);
			_pluginContext->signaling->Start();
			_pluginContext->worker = rtc::Thread::Create();
			_pluginContext->worker->SetName("pc_worker_thread", nullptr);
			_pluginContext->worker->Start();
			_pluginContext->network = rtc::Thread::CreateWithSocketServer();
			_pluginContext->network->SetName("pc_network_thread", nullptr);
			_pluginContext->network->Start();
			_pluginContext->pcf = webrtc::CreatePeerConnectionFactory(
				_pluginContext->network.get() /* network_thread */,
				_pluginContext->worker.get() /* worker_thread */,
				_pluginContext->signaling.get() /* signaling_thread */,
				nullptr /* default_adm */,
				webrtc::CreateBuiltinAudioEncoderFactory(),
				webrtc::CreateBuiltinAudioDecoderFactory(),
				webrtc::CreateBuiltinVideoEncoderFactory(),
				webrtc::CreateBuiltinVideoDecoderFactory(),
				nullptr /* audio_mixer */,
				nullptr /* audio_processing */);
		}
	}

	void PluginClient::destroy()
	{
	}

	void PluginClient::setHandleId(int64_t handleId)
	{
		_pluginContext->handleId = handleId;
	}

	void PluginClient::attach()
	{
		if (auto ss = _pluginContext->signalingService.lock()) {
			if (ss->sessionStatus() == SessionStatus::CONNECTED) {
				ss->attach(_pluginContext->plugin, _pluginContext->opaqueId, shared_from_this());
			}
		}
	}

	void PluginClient::sendMessage(std::shared_ptr<MessageEvent> event)
	{
		if (auto ss = _pluginContext->signalingService.lock()) {
			if (ss->sessionStatus() == SessionStatus::CONNECTED) {
				ss->sendMessage(_pluginContext->handleId, event);
			}
		}
	}

	void PluginClient::hangup(bool sendRequest)
	{
		if (auto ss = _pluginContext->signalingService.lock()) {
			if (ss->sessionStatus() == SessionStatus::CONNECTED) {
				ss->hangup(_pluginContext->handleId, sendRequest);
			}
		}
	}

	void PluginClient::detach(std::shared_ptr<DetachEvent> event)
	{
		if (auto ss = _pluginContext->signalingService.lock()) {
			if (ss->sessionStatus() == SessionStatus::CONNECTED) {
				ss->detach(_pluginContext->handleId, event);
			}
		}
	}

	void PluginClient::startStatsMonitor()
	{
		_rtcStatsTaskId = _rtcStatsTaskScheduler->schedule([wself = weak_from_this()]() {
			auto self = wself.lock();
			if (!self) {
				return;
			}

			const auto& context = self->_pluginContext;
			if (!context->statsObserver) {
				context->statsObserver = StatsObserver::create();

				auto socb = std::make_shared<StatsCallback>([wself](const rtc::scoped_refptr<const webrtc::RTCStatsReport>& report) {
					DLOG("RTC Stats Report: {}", report->ToJson());
					auto self = wself.lock();
					if (!self) {
						return;
					}

					auto ss = self->pluginContext()->signalingService.lock();
					if (!ss) {
						return;
					}

					if (ss->sessionStatus() !=SessionStatus::CONNECTED) {
						return;
					}

					auto eventHandlerThread = TMgr->thread("message-transport");
					eventHandlerThread->PostTask(RTC_FROM_HERE, [wself, report]() {
						if (auto self = wself.lock()) {
							self->onStatsDelivered(report);
						}
					});
				});
			}
		}, 5000, true);
	}

	void PluginClient::stopStatsMonitor()
	{
		if (_rtcStatsTaskScheduler) {
			_rtcStatsTaskScheduler->cancelAll();
		}
	}

	void PluginClient::sendSdp()
	{
		DLOG("Sending offer/answer SDP...");
		const auto& context = _pluginContext;
		if (!context) {
			return;
		}
		if (!context->localSdp) {
			WLOG("Local SDP instance is invalid, not sending anything...");
			return;
		}

		if (!context->offerAnswerCallback) {
			DLOG("offerAnswerCallback == nullptr");
			return;
		}

		if (auto ld = context->pc->local_description()) {
			std::string sdp;
			ld->ToString(&sdp);
			context->localSdp = { ld->type(), sdp, context->trickle.value_or(false) };
			context->sdpSent = true;
			_eventHandlerThread->PostTask(RTC_FROM_HERE, [cb = context->offerAnswerCallback, wself = weak_from_this()]() {
				auto self = wself.lock();
				if (!self) {
					return;
				}
				auto context = self->_pluginContext;
				if (!context) {
					return;
				}
				(*cb)(true, "", context->localSdp.value());
			});
		}
	}

	void PluginClient::sendData(std::shared_ptr<ChannelDataEvent> event)
	{
		if (!event) {
			DLOG("event == nullptr");
			return;
		}

		if (event->label.empty() || event->text.empty()) {
			DLOG("handler->label.empty() || handler->text.empty()");
			if (event->callback) {
				_eventHandlerThread->PostTask(RTC_FROM_HERE, [cb = event->callback]() {
					(*cb)(false, "empty label or empty text");
				});
			}
			return;
		}

		const auto& context = _pluginContext;
		if (!context) {
			return;
		}
		if (context->dataChannels.find(event->label) != context->dataChannels.end()) {
			rtc::scoped_refptr<webrtc::DataChannelInterface> dc = context->dataChannels[event->label];
			if (dc->state() == webrtc::DataChannelInterface::DataState::kOpen) {
				webrtc::DataBuffer buffer(event->text);
				dc->Send(buffer);
			}
			else {
				DLOG("data channel doesn't open");
			}
		}
		else {
			DLOG("Create new data channel and wait for it to open");
			createDataChannel(event->label, nullptr);
		}
		if (event->callback) {
			_eventHandlerThread->PostTask(RTC_FROM_HERE, [cb = event->callback]() {
				(*cb)(false, "success");
			});
		}
	}

	void PluginClient::sendDtmf(std::shared_ptr<DtmfEvent> event)
	{
		if (!event) {
			DLOG("event == nullptr");
			return;
		}

		const auto& context = _pluginContext;
		if (!context) {
			return;
		}
		if (!context->dtmfSender) {
			if (context->pc) {
				auto senders = context->pc->GetSenders();
				rtc::scoped_refptr<webrtc::RtpSenderInterface> audioSender;
				for (auto sender : senders) {
					if (sender && sender->GetDtmfSender()) {
						audioSender = sender;
					}
				}
				if (audioSender) {
					DLOG("Invalid DTMF configuration (no audio track)");
					if (event->callback) {
						_eventHandlerThread->PostTask(RTC_FROM_HERE, [cb = event->callback]() {
							(*cb)(false, "Invalid DTMF configuration (no audio track)");
						});
					}
					return;
				}
				rtc::scoped_refptr<webrtc::DtmfSenderInterface> dtmfSender = audioSender->GetDtmfSender();
				context->dtmfSender = dtmfSender;
				if (context->dtmfSender) {
					DLOG("Created DTMF Sender");

					context->dtmfObserver = std::make_unique<DtmfObserver>();

					auto tccb = std::make_shared<ToneChangeCallback>([](const std::string& tone, const std::string& tone_buffer) {
						DLOG("Sent DTMF tone: {}", tone.c_str());
					});

					context->dtmfObserver->setMessageCallback(tccb);

					context->dtmfSender->RegisterObserver(context->dtmfObserver.get());
				}
			}
		}

		if (event->tones.empty()) {
			if (event->callback) {
				_eventHandlerThread->PostTask(RTC_FROM_HERE, [cb = event->callback]() {
					(*cb)(false, "Invalid DTMF parameters");
				});
			}
			return;
		}

		// We choose 500ms as the default duration for a tone
		int duration = event->duration > 0 ? event->duration : 500;

		// We choose 50ms as the default gap between tones
		int gap = event->interToneGap > 0 ? event->interToneGap : 50;

		DLOG("Sending DTMF string: {}, (duration: {} ms, gap: {} ms", event->tones.c_str(), duration, gap);
		context->dtmfSender->InsertDtmf(event->tones, duration, gap);

		if (event->callback) {
			_eventHandlerThread->PostTask(RTC_FROM_HERE, [cb = event->callback]() {
				(*cb)(false, "success");
			});
		}
	}

	void PluginClient::stopAllTracks(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) {
		try {
			for (const auto& track : stream->GetAudioTracks()) {
				if (track) {
					track->set_enabled(false);
					stream->RemoveTrack(track);
				}
			}
			for (const auto& track : stream->GetVideoTracks()) {
				if (track) {
					track->set_enabled(false);
					stream->RemoveTrack(track);
				}
			}
		}
		catch (...) {
			// Do nothing if this fails
		}
	}

	void PluginClient::prepareStreams(std::shared_ptr<PrepareWebrtcEvent> event, rtc::scoped_refptr<webrtc::MediaStreamInterface> stream)
	{
		if (!event) {
			DLOG("event == nullptr");
			return;
		}

		auto wself = weak_from_this();

		const auto& context = _pluginContext;
		if (!context) {
			return;
		}

		if (stream) {
			DLOG("audio tracks: {}", stream->GetAudioTracks().size());
			DLOG("video tracks: {}", stream->GetVideoTracks().size());
		}

		// We're now capturing the new stream: check if we're updating or if it's a new thing
		bool addTracks = false;
		if (!context->localStream || !event->media->update || context->streamExternal) {
			context->localStream = stream;
			addTracks = true;
		}
		else {
			// We only need to update the existing stream
			if (((!event->media->update && HelperUtils::isAudioSendEnabled(event->media)) ||
				(event->media->update && (event->media->addAudio || event->media->replaceAudio))) &&
				stream->GetAudioTracks().size() > 0) {
				context->localStream->AddTrack(stream->GetAudioTracks()[0]);
				if (_pluginContext->unifiedPlan) {
					// Use Transceivers
					DLOG("{} audio track", (event->media->replaceAudio ? "Replacing" : "Adding"));
					rtc::scoped_refptr<webrtc::RtpTransceiverInterface> audioTransceiver = nullptr;
					auto transceivers = context->pc->GetTransceivers();
					for (const auto& t : transceivers) {
						if ((t->sender() && t->sender()->track() && t->sender()->track()->kind() == webrtc::MediaStreamTrackInterface::kAudioKind) ||
							(t->receiver() && t->receiver()->track() && t->receiver()->track()->kind() == webrtc::MediaStreamTrackInterface::kAudioKind)) {
							audioTransceiver = t;
							break;
						}
					}
					if (audioTransceiver && audioTransceiver->sender()) {
						// TODO:
						DLOG("Replacing audio track");
						audioTransceiver->sender()->SetTrack(stream->GetAudioTracks()[0]);
					}
					else {
						DLOG("Adding audio track");
						context->pc->AddTrack(stream->GetAudioTracks()[0], { stream->id() });
					}
				}
				else {
					DLOG("{} audio track", (event->media->replaceAudio ? "Replacing" : "Adding"));
					context->pc->AddTrack(stream->GetAudioTracks()[0], { stream->id() });
				}
			}
			if (((!event->media->update && HelperUtils::isVideoSendEnabled(event->media)) ||
				(event->media->update && (event->media->addVideo || event->media->replaceVideo))) &&
				stream->GetVideoTracks().size() > 0) {
				context->localStream->AddTrack(stream->GetVideoTracks()[0]);
				if (_pluginContext->unifiedPlan) {
					// Use Transceivers
					DLOG("{} video track", (event->media->replaceVideo ? "Replacing" : "Adding"));
					rtc::scoped_refptr<webrtc::RtpTransceiverInterface> videoTransceiver = nullptr;
					auto transceivers = context->pc->GetTransceivers();
					for (const auto& t : transceivers) {
						if ((t->sender() && t->sender()->track() && t->sender()->track()->kind() == webrtc::MediaStreamTrackInterface::kVideoKind) ||
							(t->receiver() && t->receiver()->track() && t->receiver()->track()->kind() == webrtc::MediaStreamTrackInterface::kVideoKind)) {
							videoTransceiver = t;
							break;
						}
					}
					if (videoTransceiver && videoTransceiver->sender()) {
						// TODO:
						DLOG("Replacing video track");
						videoTransceiver->sender()->SetTrack(stream->GetVideoTracks()[0]);
					}
					else {
						DLOG("Adding video track");
						context->pc->AddTrack(stream->GetVideoTracks()[0], { stream->id() });
					}
				}
				else {
					DLOG("{} video track", (event->media->replaceVideo ? "Replacing" : "Adding"));
					context->pc->AddTrack(stream->GetVideoTracks()[0], { stream->id() });
				}
			}
		}

		// If we still need to create a PeerConnection, let's do that
		if (!context->pc) {
			webrtc::PeerConnectionInterface::RTCConfiguration pcConfig;
			for (const auto& server : context->iceServers) {
				webrtc::PeerConnectionInterface::IceServer stunServer;
				stunServer.uri = server;
				pcConfig.servers.emplace_back(stunServer);
			}
			// TODO:
			//pcConfig.enable_rtp_data_channel = true;
			//pcConfig.enable_dtls_srtp = true;
			pcConfig.sdp_semantics = webrtc::SdpSemantics::kUnifiedPlan;
			//pcConfig.bundle_policy = webrtc::PeerConnectionInterface::kBundlePolicyMaxBundle;
			//pcConfig.type = webrtc::PeerConnectionInterface::kRelay;
			//pcConfig.use_media_transport = true;

			DLOG("Creating PeerConnection");

			context->pc = _pluginContext->pcf->CreatePeerConnection(pcConfig, nullptr, nullptr, static_cast<webrtc::PeerConnectionObserver*>(this));
		}
		if (addTracks && stream) {
			DLOG("Adding local stream");
			bool simulcast2 = event->simulcast2.value_or(false);
			for (auto track : stream->GetAudioTracks()) {
				std::string id = stream->id();
				webrtc::RTCErrorOr<rtc::scoped_refptr<webrtc::RtpSenderInterface>> result = context->pc->AddTrack(track, { stream->id() });
				if (!result.ok()) {
					DLOG("Add track error message: {}", result.error().message());
				}
			}
			for (auto track : stream->GetVideoTracks()) {
				if (!simulcast2) {
					//context->pc->AddTrack(track, { stream->id() });
					webrtc::RTCErrorOr<rtc::scoped_refptr<webrtc::RtpSenderInterface>> result = context->pc->AddTrack(track, { stream->id() });
					if (!result.ok()) {
						DLOG("Add track error message: {}", result.error().message());
					}
				}
				else {
					DLOG("Enabling rid-based simulcasting, track-id: {}", track->id());
					webrtc::RtpTransceiverInit init;
					init.direction = webrtc::RtpTransceiverDirection::kSendRecv;
					init.stream_ids = { stream->id() };

					webrtc::RtpEncodingParameters ph;
					ph.rid = "h";
					ph.active = true;
					ph.max_bitrate_bps = 900000;

					webrtc::RtpEncodingParameters pm;
					pm.rid = "m";
					pm.active = true;
					pm.max_bitrate_bps = 300000;
					pm.scale_resolution_down_by = 2;

					webrtc::RtpEncodingParameters pl;
					pl.rid = "m";
					pl.active = true;
					pl.max_bitrate_bps = 100000;
					pl.scale_resolution_down_by = 4;

					init.send_encodings.emplace_back(ph);
					init.send_encodings.emplace_back(pm);
					init.send_encodings.emplace_back(pl);

					context->pc->AddTransceiver(track, init);
				}
			}
		}

		if (HelperUtils::isDataEnabled(event->media.value()) &&
			context->dataChannels.find("JanusDataChannel") == context->dataChannels.end()) {
			DLOG("Creating default data channel");
			auto dccb = std::make_shared<DataChannelCallback>([wself = weak_from_this()](rtc::scoped_refptr<webrtc::DataChannelInterface> dataChannel) {
				DLOG("Data channel created by Janus.");
				if (auto self = wself.lock()) {
					// should be called in SERVICE thread
					TMgr->thread("plugin-client")->PostTask(RTC_FROM_HERE, [wself, dataChannel]() {
						if (auto self = wself.lock()) {
							self->createDataChannel(dataChannel->label(), dataChannel);
						}
					});
				}
			});
			//context->pcObserver->setDataChannelCallback(dccb);
		}

		if (context->localStream) {
			_eventHandlerThread->PostTask(RTC_FROM_HERE, [wself]() {
				auto self = wself.lock();
				if (!self) {
					return;
				}

				auto context = self->_pluginContext;
				if (!context) {
					return;
				}
				if (context->localStream->GetVideoTracks().size() > 0) {
					auto track = context->localStream->GetVideoTracks()[0];
					self->onLocalTrack(track, true);
				}
			});
		}

		if (event->jsep == absl::nullopt) {
			// TODO:
			_createOffer(event);
		}
		else {
			absl::optional<webrtc::SdpType> type = webrtc::SdpTypeFromString(event->jsep->type);
			if (!type) {
				DLOG("Invalid JSEP type");
				return;
			}
			webrtc::SdpParseError spError;
			std::unique_ptr<webrtc::SessionDescriptionInterface> desc = webrtc::CreateSessionDescription(type.value(),
				event->jsep->sdp, &spError);
			DLOG("spError: description: {}, line: {}", spError.description.c_str(), spError.line.c_str());

			SetSessionDescObserver* ssdo(new rtc::RefCountedObject<SetSessionDescObserver>());

			ssdo->setSuccessCallback(std::make_shared<SetSessionDescSuccessCallback>([wself, event]() {
				auto self = wself.lock();
				if (!self) {
					return;
				}

				auto context = self->_pluginContext;
				if (!context) {
					return;
				}
				context->remoteSdp = { event->jsep->type, event->jsep->sdp, false };
				for (const auto& candidate : context->candidates) {
					context->pc->AddIceCandidate(candidate.get());
				}
				context->candidates.clear();
				if (auto self = wself.lock()) {
					// should be called in SERVICE thread
					TMgr->thread("plugin-client")->PostTask(RTC_FROM_HERE, [wself, event]() {
						if (auto self = wself.lock()) {
							self->_createAnswer(event);
						}
					});
				}
			}));

			ssdo->setFailureCallback(std::make_shared<SetSessionDescFailureCallback>([event, wself](webrtc::RTCError error) {
				DLOG("SetRemoteDescription() failure: {}", error.message());
				auto self = wself.lock();
				if (event->callback && self) {
					self->_eventHandlerThread->PostTask(RTC_FROM_HERE, [cb = event->callback]() {
						(*cb)(false, "failure");
					});
				}
			}));

			context->pc->SetRemoteDescription(ssdo, desc.release());
		}
	}

	void PluginClient::prepareWebrtc(bool isOffer, std::shared_ptr<PrepareWebrtcEvent> event)
	{
		if (!event) {
			DLOG("event == nullptr");
			return;
		}

		if (isOffer && event->jsep) {
			DLOG("Provided a JSEP to a createOffer");
			return;
		}
		else if (!isOffer && (!event->jsep.has_value() || event->jsep.value().type.empty() || (event->jsep.value().sdp.empty()))) {
			DLOG("A valid JSEP is required for createAnswer");
			return;
		}

		const auto& context = _pluginContext;
		if (!context) {
			return;
		}
		context->trickle = HelperUtils::isTrickleEnabled(event->trickle);
		if (!event->media.has_value()) {
			return;
		}
		auto& media = event->media.value();
		if (!context->pc) {
			// new PeerConnection
			media.update = false;
			media.keepAudio = false;
			media.keepVideo = false;
		}
		else {
			DLOG("Updating existing media session");
			media.update = true;
			// Check if there's anything to add/remove/replace, or if we
			// can go directly to preparing the new SDP offer or answer
			if (event->stream) {
				// External stream: is this the same as the one we were using before?
				if (event->stream != context->localStream) {
					DLOG("Renegotiation involves a new external stream");
				}
			}
			else {
				// Check if there are changes on audio
				if (media.addAudio) {
					media.keepAudio = false;
					media.replaceAudio = false;
					media.removeAudio = false;
					media.audioSend = true;
					if (context->localStream && context->localStream->GetAudioTracks().size() > 0) {
						ELOG("Can't add audio stream, there already is one");
						if (event->callback) {
							_eventHandlerThread->PostTask(RTC_FROM_HERE, [cb = event->callback]() {
								(*cb)(false, "Can't add audio stream, there already is one");
							});
						}
						return;
					}
				}
				else if (media.removeAudio) {
					media.keepAudio = false;
					media.replaceAudio = false;
					media.addAudio = false;
					media.audioSend = false;
				}
				else if (media.replaceAudio) {
					media.keepAudio = false;
					media.addAudio = false;
					media.removeAudio = false;
					media.audioSend = true;
				}
				if (!context->localStream) {
					// No media stream: if we were asked to replace, it's actually an "add"
					if (media.replaceAudio) {
						media.keepAudio = false;
						media.replaceAudio = false;
						media.addAudio = true;
						media.audioSend = true;
					}
					if (HelperUtils::isAudioSendEnabled(media)) {
						media.keepAudio = false;
						media.addAudio = true;
					}
				}
				else {
					if (context->localStream->GetAudioTracks().size() == 0) {
						// No audio track: if we were asked to replace, it's actually an "add"
						if (media.replaceAudio) {
							media.keepAudio = false;
							media.replaceAudio = false;
							media.addAudio = true;
							media.audioSend = true;
						}
						if (HelperUtils::isAudioSendEnabled(media)) {
							media.keepVideo = false;
							media.addAudio = true;
						}
					}
					else {
						// We have an audio track: should we keep it as it is?
						if (HelperUtils::isAudioSendEnabled(media) && !media.removeAudio && !media.replaceAudio) {
							media.keepAudio = true;
						}
					}
				}

				// Check if there are changes on video
				if (media.addVideo) {
					media.keepVideo = false;
					media.replaceVideo = false;
					media.removeVideo = false;
					media.videoSend = true;
					if (context->localStream && context->localStream->GetVideoTracks().size() > 0) {
						ELOG("Can't add video stream, there already is one");
						if (event->callback) {
							_eventHandlerThread->PostTask(RTC_FROM_HERE, [cb = event->callback]() {
								(*cb)(false, "Can't add video stream, there already is one");
							});
						}
						return;
					}
				}
				else if (media.removeVideo) {
					media.keepVideo = false;
					media.replaceVideo = false;
					media.addVideo = false;
					media.videoSend = false;
				}
				else if (media.replaceVideo) {
					media.keepVideo = false;
					media.addVideo = false;
					media.removeVideo = false;
					media.videoSend = true;
				}
				if (!context->localStream) {
					// No media stream: if we were asked to replace, it's actually an "add"
					if (media.replaceVideo) {
						media.keepVideo = false;
						media.replaceVideo = false;
						media.addVideo = true;
						media.videoSend = true;
					}
					if (HelperUtils::isVideoSendEnabled(media)) {
						media.keepVideo = false;
						media.addVideo = true;
					}
				}
				else {
					if (context->localStream->GetVideoTracks().size() == 0) {
						// No video track: if we were asked to replace, it's actually an "add"
						if (media.replaceVideo) {
							media.keepVideo = false;
							media.replaceVideo = false;
							media.addVideo = true;
							media.videoSend = true;
						}
						if (HelperUtils::isVideoSendEnabled(media)) {
							media.keepVideo = false;
							media.addVideo = true;
						}
					}
					else {
						// We have a video track: should we keep it as it is?
						if (HelperUtils::isVideoSendEnabled(media) && !media.removeVideo && !media.replaceVideo) {
							media.keepVideo = true;
						}
					}
				}
				// Data channels can only be added
				if (media.addData) {
					media.data = true;
				}
			}
			// If we're updating and keeping all tracks, let's skip the getUserMedia part
			if ((HelperUtils::isAudioSendEnabled(media) && media.keepAudio) &&
				(HelperUtils::isVideoSendEnabled(media) && media.keepVideo)) {
				// TODO: notify ?
				//streams done
				prepareStreams(event, context->localStream);
				return;
			}
		}
		// If we're updating, check if we need to remove/replace one of the tracks
		if (media.update && !context->streamExternal) {
			if (media.removeAudio || media.replaceAudio) {
				if (context->localStream && context->localStream->GetAudioTracks().size() > 0) {
					rtc::scoped_refptr<webrtc::AudioTrackInterface> at = context->localStream->GetAudioTracks()[0];
					DLOG("Removing audio track, id = {}", at->id());
					context->localStream->RemoveTrack(at);
					try {
						onLocalTrack(at, false);
						at->set_enabled(false);
					}
					catch (...) {
					}
				}
				if (context->pc->GetSenders().size() > 0) {
					bool ra = true;
					if (media.replaceAudio && _pluginContext->unifiedPlan) {
						// We can use replaceTrack
						ra = false;
					}
					if (ra) {
						for (const auto& sender : context->pc->GetSenders()) {
							if (sender && sender->track() && sender->track()->kind() == webrtc::MediaStreamTrackInterface::kAudioKind) {
								DLOG("Removing audio sender, id = {}, ssrc = {}", sender->id(), sender->ssrc());
								context->pc->RemoveTrack(sender);
							}
						}
					}
				}
			}
			if (media.removeVideo || media.replaceVideo) {
				if (context->localStream && context->localStream->GetVideoTracks().size() > 0) {
					rtc::scoped_refptr<webrtc::VideoTrackInterface> vt = context->localStream->GetVideoTracks()[0];
					DLOG("Removing video track, id = {}", vt->id());
					context->localStream->RemoveTrack(vt);
					try {
						onLocalTrack(vt, false);
						vt->set_enabled(false);
					}
					catch (...) {
					}
				}
				if (context->pc->GetSenders().size() > 0) {
					bool ra = true;
					if (media.replaceVideo && _pluginContext->unifiedPlan) {
						// We can use replaceTrack
						ra = false;
					}
					if (ra) {
						for (const auto& sender : context->pc->GetSenders()) {
							if (sender && sender->track() && sender->track()->kind() == webrtc::MediaStreamTrackInterface::kVideoKind) {
								DLOG("Removing video sender, id = {}, ssrc = {}", sender->id(), sender->ssrc());
								context->pc->RemoveTrack(sender);
							}
						}
					}
				}
			}
		}
		// Was a MediaStream object passed, or do we need to take care of that?
		if (event->stream) {
			const auto& stream = event->stream;
			DLOG("MediaStream provided by the application");

			// If this is an update, let's check if we need to release the previous stream
			if (media.update) {
				if (context->localStream && context->localStream != stream && !context->streamExternal) {
					// We're replacing a stream we captured ourselves with an external one
					stopAllTracks(context->localStream);
					context->localStream = nullptr;
				}
			}
			// Skip the getUserMedia part
			context->streamExternal = true;
			// TODO: notify ?
			// streams done
			prepareStreams(event, stream);
			return;
		}
		if (HelperUtils::isAudioSendEnabled(media) || HelperUtils::isVideoSendEnabled(media)) {
			rtc::scoped_refptr<webrtc::MediaStreamInterface> mstream = _pluginContext->pcf->CreateLocalMediaStream("stream_id");
			rtc::scoped_refptr<webrtc::AudioTrackInterface> audioTrack(_pluginContext->pcf->CreateAudioTrack("audio_label", _pluginContext->pcf->CreateAudioSource(cricket::AudioOptions())));
			std::string id = audioTrack->id();
			if (!mstream->AddTrack(audioTrack)) {
				DLOG("Add audio track failed.");
			}

			rtc::scoped_refptr<CapturerTrackSource> capturerSource = CapturerTrackSource::Create();
			DLOG("create capture source");
			if (capturerSource) {
				rtc::scoped_refptr<VideoTrackInterface> captureTrack = _pluginContext->pcf->CreateVideoTrack("video_label", capturerSource);

				if (!mstream->AddTrack(captureTrack.release())) {
					DLOG("Add video track failed.");
				}
			}

			prepareStreams(event, mstream);
		}
		else {
			// No need to do a getUserMedia, create offer/answer right away
			prepareStreams(event, nullptr);
		}
	}

	void PluginClient::createOffer(std::shared_ptr<PrepareWebrtcEvent> event)
	{
		prepareWebrtc(true, event);
	}

	void PluginClient::createAnswer( std::shared_ptr<PrepareWebrtcEvent> event)
	{
		prepareWebrtc(false, event);
	}

	void PluginClient::handleRemoteJsep(std::shared_ptr<PrepareWebrtcPeerEvent> event)
	{
		if (!event) {
			DLOG("event == nullptr");
			return;
		}

		const auto& context = _pluginContext;
		if (!context) {
			return;
		}
		if (event->jsep.has_value()) {
			if (!context->pc) {
				DLOG("No PeerConnection: if this is an answer, use createAnswer and not handleRemoteJsep");
				if (event->callback) {
					_eventHandlerThread->PostTask(RTC_FROM_HERE, [cb = event->callback]() {
						(*cb)(false, "No PeerConnection: if this is an answer, use createAnswer and not handleRemoteJsep");
					});
				}
				return;
			}
			absl::optional<webrtc::SdpType> type = webrtc::SdpTypeFromString(event->jsep->type);
			if (!type) {
				DLOG("Invalid JSEP type");
				return;
			}
			webrtc::SdpParseError spError;
			std::unique_ptr<webrtc::SessionDescriptionInterface> desc = webrtc::CreateSessionDescription(type.value(),
				event->jsep->sdp, &spError);
			DLOG("spError: description: {}, line: {}", spError.description.c_str(), spError.line.c_str());

			auto wself = weak_from_this();
			SetSessionDescObserver* ssdo(new rtc::RefCountedObject<SetSessionDescObserver>());
			ssdo->setSuccessCallback(std::make_shared<SetSessionDescSuccessCallback>([event, wself]() {
				auto self = wself.lock();
				if (!self) {
					return;
				}

				auto context = self->_pluginContext;
				if (!context) {
					return;
				}
				context->remoteSdp = { event->jsep->type, event->jsep->sdp, false };

				for (const auto& candidate : context->candidates) {
					context->pc->AddIceCandidate(candidate.get());
				}
				context->candidates.clear();
				if (event->callback && self) {
					self->_eventHandlerThread->PostTask(RTC_FROM_HERE, [cb = event->callback]() {
						(*cb)(true, "success");
					});
				}
			}));
			ssdo->setFailureCallback(std::make_shared<SetSessionDescFailureCallback>([event, wself](webrtc::RTCError error) {
				DLOG("SetRemoteDescription() failure: {}", error.message());
				auto self = wself.lock();
				if (event->callback && self) {
					self->_eventHandlerThread->PostTask(RTC_FROM_HERE, [cb = event->callback]() {
						(*cb)(false, "failure");
					});
				}
			}));
			context->pc->SetRemoteDescription(ssdo, desc.release());
		}
		else {
			DLOG("Invalid JSEP");
			if (event->callback) {
				_eventHandlerThread->PostTask(RTC_FROM_HERE, [cb = event->callback]() {
					(*cb)(false, "Invalid JSEP");
				});
			}
		}
	}

	void PluginClient::createDataChannel(const std::string& dcLabel, rtc::scoped_refptr<webrtc::DataChannelInterface> incoming)
	{
		const auto& context = _pluginContext;
		if (!context) {
			return;
		}
		if (!context->pc) {
			ELOG("Invalid peerconnection");
		}

		if (incoming) {
			context->dataChannels[dcLabel] = incoming;
		}
		else {
			webrtc::DataChannelInit init;
			auto dataChannel = context->pc->CreateDataChannel(dcLabel, &init);
			context->dataChannels[dcLabel] = dataChannel;
		}

		auto wself = weak_from_this();

		std::shared_ptr<DCObserver> observer = std::make_shared<DCObserver>();

		auto scc = std::make_shared<StateChangeCallback>([dcLabel, wself]() {
			auto self = wself.lock();
			if (!self) {
				return;
			}

			auto context = self->_pluginContext;
			if (!context) {
				return;
			}
			if (context->dataChannels.find(dcLabel) != context->dataChannels.end()) {
				auto dc = context->dataChannels[dcLabel];
				if (dc->state() == webrtc::DataChannelInterface::DataState::kOpen) {
					self->_eventHandlerThread->PostTask(RTC_FROM_HERE, [wself, dcLabel]() {
						auto self = wself.lock();
						if (!self) {
							return;
						}
						self->onChannelOpened(dcLabel);
					});
				}
				else if (dc->state() == webrtc::DataChannelInterface::DataState::kClosed) {
					self->_eventHandlerThread->PostTask(RTC_FROM_HERE, [wself, dcLabel]() {
						auto self = wself.lock();
						if (!self) {
							return;
						}
						self->onChannelClosed(dcLabel);
					});
				}
			}
		});
		observer->setStateChangeCallback(scc);

		auto mc = std::make_shared<MessageCallback>([dcLabel, wself](const webrtc::DataBuffer& buffer) {
			auto self = wself.lock();
			if (!self) {
				return;
			}
			self->_eventHandlerThread->PostTask(RTC_FROM_HERE, [wself, buffer, dcLabel]() {
				size_t size = buffer.data.size();
				char* msg = new char[size + 1];
				memcpy(msg, buffer.data.data(), size);
				msg[size] = 0;
				auto self = wself.lock();
				if (!self) {
					return;
				}
				
				self->onChannelData(dcLabel, std::string(msg, size));

				delete[] msg;
			});
		});
		observer->setMessageCallback(mc);

		context->dataChannelObservers[dcLabel] = observer;

		auto dc = context->dataChannels[dcLabel];
		dc->RegisterObserver(observer.get());
	}

	void PluginClient::configTracks(const MediaConfig& media, rtc::scoped_refptr<webrtc::PeerConnectionInterface> pc)
	{
		if (!pc) {
			return;
		}
		rtc::scoped_refptr<webrtc::RtpTransceiverInterface> audioTransceiver = nullptr;
		rtc::scoped_refptr<webrtc::RtpTransceiverInterface> videoTransceiver = nullptr;
		std::vector<rtc::scoped_refptr<webrtc::RtpTransceiverInterface>> transceivers = pc->GetTransceivers();
		for (auto t : transceivers) {
			if ((t->sender() && t->sender()->track() && t->sender()->track()->kind() == webrtc::MediaStreamTrackInterface::kAudioKind) ||
				(t->receiver() && t->receiver()->track() && t->receiver()->track()->kind() == webrtc::MediaStreamTrackInterface::kAudioKind)) {
				if (!audioTransceiver)
					audioTransceiver = t;
				continue;
			}
			if ((t->sender() && t->sender()->track() && t->sender()->track()->kind() == webrtc::MediaStreamTrackInterface::kVideoKind) ||
				(t->receiver() && t->receiver()->track() && t->receiver()->track()->kind() == webrtc::MediaStreamTrackInterface::kVideoKind)) {
				if (!videoTransceiver)
					videoTransceiver = t;
				continue;
			}
		}
		// Handle audio (and related changes, if any)
		bool audioSend = HelperUtils::isAudioSendEnabled(media);
		bool audioRecv = HelperUtils::isAudioRecvEnabled(media);
		if (!audioSend && !audioRecv) {
			// Audio disabled: have we removed it?
			if (media.removeAudio && audioTransceiver) {
				audioTransceiver->SetDirection(webrtc::RtpTransceiverDirection::kInactive);
				DLOG("Setting audio transceiver to inactive");
			}
		}
		else {
			// Take care of audio m-line
			if (audioSend && audioRecv) {
				if (audioTransceiver) {
					audioTransceiver->SetDirection(webrtc::RtpTransceiverDirection::kSendRecv);
					DLOG("Setting audio transceiver to sendrecv");
				}
			}
			else if (audioSend && !audioRecv) {
				if (audioTransceiver) {
					audioTransceiver->SetDirection(webrtc::RtpTransceiverDirection::kSendOnly);
					DLOG("Setting audio transceiver to sendonly");
				}
			}
			else if (!audioSend && audioRecv) {
				if (audioTransceiver) {
					audioTransceiver->SetDirection(webrtc::RtpTransceiverDirection::kRecvOnly);
					DLOG("Setting audio transceiver to recvonly");
				}
				else {
					// In theory, this is the only case where we might not have a transceiver yet
					webrtc::RtpTransceiverInit init;
					init.direction = webrtc::RtpTransceiverDirection::kRecvOnly;
					webrtc::RTCErrorOr<rtc::scoped_refptr<webrtc::RtpTransceiverInterface>> result = pc->AddTransceiver(cricket::MediaType::MEDIA_TYPE_AUDIO, init);
					if (result.ok()) {
						audioTransceiver = result.value();
					}
					DLOG("Adding recvonly audio transceiver");
				}
			}
		}
		// Handle video (and related changes, if any)
		bool videoSend = HelperUtils::isVideoSendEnabled(media);
		bool videoRecv = HelperUtils::isVideoRecvEnabled(media);
		if (!videoSend && !videoRecv) {
			// Video disabled: have we removed it?
			if (media.removeVideo && videoTransceiver) {
				videoTransceiver->SetDirection(webrtc::RtpTransceiverDirection::kInactive);
				DLOG("Setting video transceiver to inactive");
			}
		}
		else {
			// Take care of video m-line
			if (videoSend && videoRecv) {
				if (videoTransceiver) {
					videoTransceiver->SetDirection(webrtc::RtpTransceiverDirection::kSendRecv);
					DLOG("Setting video transceiver to sendrecv");
				}
			}
			else if (videoSend && !videoRecv) {
				if (videoTransceiver) {
					videoTransceiver->SetDirection(webrtc::RtpTransceiverDirection::kSendOnly);
					DLOG("Setting video transceiver to sendonly");
				}
			}
			else if (!videoSend && videoRecv) {
				if (videoTransceiver) {
					videoTransceiver->SetDirection(webrtc::RtpTransceiverDirection::kRecvOnly);
					DLOG("Setting video transceiver to recvonly");
				}
				else {
					// In theory, this is the only case where we might not have a transceiver yet
					webrtc::RtpTransceiverInit init;
					init.direction = webrtc::RtpTransceiverDirection::kRecvOnly;
					webrtc::RTCErrorOr<rtc::scoped_refptr<webrtc::RtpTransceiverInterface>> result = pc->AddTransceiver(cricket::MediaType::MEDIA_TYPE_VIDEO, init);
					if (result.ok()) {
						videoTransceiver = result.value();
					}
					DLOG("Adding recvonly video transceiver");
				}
			}
		}
	}

	void PluginClient::_createOffer(std::shared_ptr<PrepareWebrtcEvent> event)
	{
		if (!event) {
			DLOG("event == nullptr");
			return;
		}

		const auto& context = _pluginContext;
		if (!context) {
			return;
		}

		bool simulcast = event->simulcast.value_or(false);
		if (!simulcast) {
			DLOG("Creating offer (iceDone = {})", context->iceDone ? "true" : "false");
		}
		else {
			DLOG("Creating offer (iceDone = {}, simulcast = {})", context->iceDone ? "true" : "false", simulcast ? "enabled" : "disabled");
		}

		webrtc::PeerConnectionInterface::RTCOfferAnswerOptions options;
		auto& media = event->media.value();
		if (_pluginContext->unifiedPlan) {
			configTracks(media, context->pc);
		}
		else {
			options.offer_to_receive_audio = HelperUtils::isAudioRecvEnabled(media);
			options.offer_to_receive_video = HelperUtils::isVideoRecvEnabled(media);
		}

		options.ice_restart = event->iceRestart.value_or(false);

		bool sendVideo = HelperUtils::isVideoSendEnabled(media);

		if (sendVideo && simulcast) {
			std::vector<rtc::scoped_refptr<webrtc::RtpSenderInterface>> senders = context->pc->GetSenders();
			rtc::scoped_refptr<webrtc::RtpSenderInterface> sender;
			for (auto& s : senders) {
				if (s->track()->kind() == webrtc::MediaStreamTrackInterface::kVideoKind) {
					sender = s;
				}
			}
			if (sender) {
				webrtc::RtpParameters params = sender->GetParameters();
				webrtc::RtpEncodingParameters ph;
				ph.rid = "h";
				ph.active = true;
				ph.max_bitrate_bps = 900000;

				webrtc::RtpEncodingParameters pm;
				pm.rid = "m";
				pm.active = true;
				pm.max_bitrate_bps = 300000;
				pm.scale_resolution_down_by = 2;

				webrtc::RtpEncodingParameters pl;
				pl.rid = "m";
				pl.active = true;
				pl.max_bitrate_bps = 100000;
				pl.scale_resolution_down_by = 4;

				params.encodings.emplace_back(ph);
				params.encodings.emplace_back(pm);
				params.encodings.emplace_back(pl);

				sender->SetParameters(params);
			}
		}
		std::unique_ptr<CreateSessionDescObserver> createOfferObserver;
		createOfferObserver.reset(new rtc::RefCountedObject<CreateSessionDescObserver>());

		auto wself = weak_from_this();
		std::shared_ptr<CreateSessionDescSuccessCallback> success = std::make_shared<CreateSessionDescSuccessCallback>([event, options, wself, sendVideo, simulcast](webrtc::SessionDescriptionInterface* desc) {
			auto self = wself.lock();
			if (!self) {
				return;
			}

			auto context = self->_pluginContext;
			if (!context) {
				return;
			}
			if (!desc) {
				ELOG("Invalid description.");
				return;
			}

			SetSessionDescObserver* ssdo(new rtc::RefCountedObject<SetSessionDescObserver>());

			ssdo->setSuccessCallback(std::make_shared<SetSessionDescSuccessCallback>([]() {
				DLOG("Set session description success.");
			}));

			ssdo->setFailureCallback(std::make_shared<SetSessionDescFailureCallback>([event, wself](webrtc::RTCError error) {
				DLOG("SetLocalDescription() failure: {}", error.message());
				auto self = wself.lock();
				if (event->callback && self) {
					self->_eventHandlerThread->PostTask(RTC_FROM_HERE, [cb = event->callback]() {
						(*cb)(false, "failure");
					});
				}
			}));

			std::string sdp;
			desc->ToString(&sdp);

			if (sendVideo && simulcast) {
				std::vector<std::string> lines = SDPUtils::split(sdp, '\n');
				SDPUtils::injectSimulcast(2, lines);
				sdp = SDPUtils::join(lines);
			}

			JsepConfig jsep{ desc->type(), sdp, false };

			context->localSdp = jsep;
			context->pc->SetLocalDescription(ssdo, desc);
			context->options = options;
			if (!context->iceDone && !context->trickle.value_or(false)) {
				// Don't do anything until we have all candidates
				DLOG("Waiting for all candidates...");
				return;
			}

			if (self && context->offerAnswerCallback) {
				self->_eventHandlerThread->PostTask(RTC_FROM_HERE, [cb = context->offerAnswerCallback, jsep]() {
					(*cb)(true, "", jsep);
				});
			}
		});

		std::shared_ptr<CreateSessionDescFailureCallback> failure = std::make_shared<CreateSessionDescFailureCallback>([event, wself](webrtc::RTCError error) {
			DLOG("createOfferObserver() failure: {}", error.message());
			auto self = wself.lock();
			if (event->callback && self) {
				self->_eventHandlerThread->PostTask(RTC_FROM_HERE, [cb = event->callback]() {
					(*cb)(false, "failure");
				});
			}
		});

		createOfferObserver->setSuccessCallback(success);
		createOfferObserver->setFailureCallback(failure);

		context->pc->CreateOffer(createOfferObserver.release(), options);
	}

	void PluginClient::_createAnswer(std::shared_ptr<PrepareWebrtcEvent> event)
	{
		if (!event) {
			DLOG("event == nullptr");
			return;
		}

		const auto& context = _pluginContext;
		if (!context) {
			return;
		}
		bool simulcast = event->simulcast.value_or(false);
		if (!simulcast) {
			DLOG("Creating offer (iceDone = {})", context->iceDone ? "true" : "false");
		}
		else {
			DLOG("Creating offer (iceDone = {}, simulcast = {})", context->iceDone ? "true" : "false", simulcast ? "enabled" : "disabled");
		}

		webrtc::PeerConnectionInterface::RTCOfferAnswerOptions options;
		auto& media = event->media.value();
		if (_pluginContext->unifiedPlan) {
			configTracks(media, context->pc);
		}
		else {
			options.offer_to_receive_audio = HelperUtils::isAudioRecvEnabled(media);
			options.offer_to_receive_video = HelperUtils::isVideoRecvEnabled(media);
		}

		options.offer_to_receive_audio = HelperUtils::isAudioRecvEnabled(media);
		options.offer_to_receive_video = HelperUtils::isVideoRecvEnabled(media);
		options.ice_restart = event->iceRestart.value_or(false);

		bool sendVideo = HelperUtils::isVideoSendEnabled(media);

		if (sendVideo && simulcast) {
			DLOG("Enabling Simulcasting");
			std::vector<rtc::scoped_refptr<webrtc::RtpSenderInterface>> senders = context->pc->GetSenders();
			rtc::scoped_refptr<webrtc::RtpSenderInterface> sender = senders[1];

			if (sender) {
				webrtc::RtpParameters params = sender->GetParameters();
				webrtc::RtpEncodingParameters ph;
				ph.rid = "h";
				ph.active = true;
				ph.max_bitrate_bps = 900000;

				webrtc::RtpEncodingParameters pm;
				pm.rid = "m";
				pm.active = true;
				pm.max_bitrate_bps = 300000;
				pm.scale_resolution_down_by = 2;

				webrtc::RtpEncodingParameters pl;
				pl.rid = "m";
				pl.active = true;
				pl.max_bitrate_bps = 100000;
				pl.scale_resolution_down_by = 4;

				params.encodings.emplace_back(ph);
				params.encodings.emplace_back(pm);
				params.encodings.emplace_back(pl);

				sender->SetParameters(params);
			}
		}

		auto wself = weak_from_this();

		std::unique_ptr<CreateSessionDescObserver> createAnswerObserver;
		createAnswerObserver.reset(new rtc::RefCountedObject<CreateSessionDescObserver>());

		std::shared_ptr<CreateSessionDescSuccessCallback> success = std::make_shared<CreateSessionDescSuccessCallback>([event, options, wself, sendVideo, simulcast](webrtc::SessionDescriptionInterface* desc) {
			auto self = wself.lock();
			if (!self) {
				return;
			}

			auto context = self->_pluginContext;
			if (!context) {
				return;
			}
			if (!desc) {
				ELOG("Invalid description.");
				return;
			}

			SetSessionDescObserver* ssdo(new rtc::RefCountedObject<SetSessionDescObserver>());

			ssdo->setSuccessCallback(std::make_shared<SetSessionDescSuccessCallback>([]() {
				DLOG("Set session description success.");
			}));

			ssdo->setFailureCallback(std::make_shared<SetSessionDescFailureCallback>([event, wself](webrtc::RTCError error) {
				DLOG("SetLocalDescription() failure: {}", error.message());
				auto self = wself.lock();
				if (event->callback && self) {
					self->_eventHandlerThread->PostTask(RTC_FROM_HERE, [cb = event->callback]() {
						(*cb)(false, "failure");
					});
				}
			}));

			std::string sdp;
			desc->ToString(&sdp);

			if (sendVideo && simulcast) {
				std::vector<std::string> lines = SDPUtils::split(sdp, '\n');
				SDPUtils::injectSimulcast(2, lines);
				sdp = SDPUtils::join(lines);
			}

			JsepConfig jsep{ desc->type(), sdp, false };

			context->localSdp = jsep;
			context->pc->SetLocalDescription(ssdo, desc);
			context->options = options;
			if (!context->iceDone && !context->trickle.value_or(false)) {
				// Don't do anything until we have all candidates
				DLOG("Waiting for all candidates...");
				return;
			}

			if (self && context->offerAnswerCallback) {
				self->_eventHandlerThread->PostTask(RTC_FROM_HERE, [cb = context->offerAnswerCallback, jsep]() {
					(*cb)(true, "", jsep);
				});
			}
		});

		std::shared_ptr<CreateSessionDescFailureCallback> failure = std::make_shared<CreateSessionDescFailureCallback>([event, wself](webrtc::RTCError error) {
			DLOG("CreateAnswer() failure: {}", error.message());
			if (event->callback) {
				auto self = wself.lock();
				if (event->callback && self) {
					self->_eventHandlerThread->PostTask(RTC_FROM_HERE, [cb = event->callback]() {
						(*cb)(false, "failure");
					});
				}
			}
		});

		createAnswerObserver->setSuccessCallback(success);
		createAnswerObserver->setFailureCallback(failure);
		
		context->pc->CreateAnswer(createAnswerObserver.release(), options);
	}

	void PluginClient::cleanupWebrtc(bool hangupRequest)
	{
		DLOG("cleaning webrtc ...");

		const auto& context = _pluginContext;

		if (!context->streamExternal && context->localStream) {
			DLOG("Stopping local stream tracks");
			stopAllTracks(context->localStream);
		}

		context->streamExternal = false;
		context->localStream = nullptr;

		// Close PeerConnection
		if (context->pc) {
			context->pc->Close();
			context->pc = nullptr;
		}

		context->candidates.clear();
		context->localSdp = absl::nullopt;
		context->remoteSdp = absl::nullopt;
		context->iceDone = false;
		context->dataChannels.clear();
		context->dtmfSender = nullptr;
	}


	void PluginClient::OnStandardizedIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState newState)
	{

	}

	void PluginClient::OnConnectionChange(webrtc::PeerConnectionInterface::PeerConnectionState new_state)
	{

	}

	void PluginClient::OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state)
	{

	}

	void PluginClient::OnIceCandidate(const webrtc::IceCandidateInterface* candidate)
	{
		if (candidate) {
			if (_pluginContext->trickle) {
				std::string candidateStr;
				candidate->ToString(&candidateStr);

				auto event = std::make_shared<TrickleCandidateEvent>();
				event->candidate.candidate = candidateStr;
				event->candidate.sdpMid = candidate->sdp_mid();
				event->candidate.sdpMLineIndex = (int)candidate->sdp_mline_index();
				event->candidate.completed = false;

				if (auto ss = _pluginContext->signalingService.lock()) {
					if (ss->sessionStatus() == SessionStatus::CONNECTED) {
						ss->sendTrickleCandidate(_pluginContext->handleId, event);
					}
				}
			}
		}
		else {
			DLOG("End of candidates.");
			_pluginContext->iceDone = true;
			if (_pluginContext->trickle) {
				auto event = std::make_shared<TrickleCandidateEvent>();
				event->candidate.completed = true;
				if (auto ss = _pluginContext->signalingService.lock()) {
					if (ss->sessionStatus() == SessionStatus::CONNECTED) {
						ss->sendTrickleCandidate(_pluginContext->handleId, event);
					}
				}
			}
			else {
				// should be called in SERVICE thread
				DLOG("send candidates.");
				TMgr->thread("plugin-client")->PostTask(RTC_FROM_HERE, [wself = weak_from_this()]() {
					if (auto self = wself.lock()) {
						self->sendSdp();
					}
				});
			}
		}
	}

	void PluginClient::OnTrack(rtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver)
	{
		_eventHandlerThread->PostTask(RTC_FROM_HERE, [transceiver, wself = weak_from_this()]() {
			auto self = wself.lock();
			if (!self) {
				return;
			}
			if (!transceiver) {
				return;
			}
			if (!transceiver->receiver()) {
				return;
			}
			auto track = transceiver->receiver()->track();
			if (!track) {
				return;
			}
			self->_trackIdsMap[track->id()] = transceiver->mid().value_or("");
			self->onRemoteTrack(track, transceiver->mid().value_or(""), true);
		});
	}

	void PluginClient::OnRemoveTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver)
	{
		_eventHandlerThread->PostTask(RTC_FROM_HERE, [receiver, wself = weak_from_this()]() {
			auto self = wself.lock();
			if (!self) {
				return;
			}
			if (!receiver) {
				return;
			}
			auto track = receiver->track();
			if (!track) {
				return;
			}
			auto context = self->_pluginContext;
			if (!context) {
				return;
			}
			if (self->_trackIdsMap.find(track->id()) != self->_trackIdsMap.end()) {
				self->onRemoteTrack(track, self->_trackIdsMap[track->id()], false);
				auto it = self->_trackIdsMap.find(track->id());
				self->_trackIdsMap.erase(it);
			}
		});
	}

	void PluginClient::onTrickle(const std::string& trickle)
	{
		std::string err;
		std::shared_ptr<TrickleResponse> model = fromJsonString<TrickleResponse>(trickle, err);
		if (!err.empty()) {
			DLOG("parse JanusResponse failed");
			return;
		}

		// We got a trickle candidate from Janus
		bool hasCandidata = model->candidate.has_value();

		auto& context = _pluginContext;
		if (!context) {
			return;
		}
		if (context->pc && context->remoteSdp) {
			// Add candidate right now
			if (!hasCandidata || (hasCandidata && model->candidate->completed && model->candidate->completed.value_or(false) == true)) {
				// end-of-candidates
				context->pc->AddIceCandidate(nullptr);
			}
			else {
				if (hasCandidata && model->candidate->sdpMid && model->candidate->sdpMLineIndex) {
					const auto& candidate = model->candidate->candidate;

					DLOG("Adding remote candidate: {}", candidate.value_or(""));

					webrtc::SdpParseError spError;
					std::unique_ptr<webrtc::IceCandidateInterface> ici(webrtc::CreateIceCandidate(model->candidate->sdpMid.value(),
						model->candidate->sdpMLineIndex.value(),
						model->candidate->candidate.value(),
						&spError));
					DLOG("candidate error: {}", spError.description.c_str());
					// New candidate
					context->pc->AddIceCandidate(ici.get());
				}
			}
		}
		else {
			// We didn't do setRemoteDescription (trickle got here before the offer?)
			DLOG("We didn't do setRemoteDescription (trickle got here before the offer?), caching candidate");

			if (hasCandidata && model->candidate->sdpMid && model->candidate->sdpMLineIndex) {
				const auto& candidate = model->candidate->candidate;

				DLOG("Adding remote candidate: {}", candidate.value_or(""));

				webrtc::SdpParseError spError;
				std::shared_ptr<webrtc::IceCandidateInterface> ici(webrtc::CreateIceCandidate(model->candidate->sdpMid.value(),
					model->candidate->sdpMLineIndex.value(),
					model->candidate->candidate.value(),
					&spError));
				DLOG("candidate error: {}", spError.description.c_str());

				context->candidates.emplace_back(ici);
			}
		}
	}

	void PluginClient::onCleanup()
	{
		cleanupWebrtc();
	}
}


