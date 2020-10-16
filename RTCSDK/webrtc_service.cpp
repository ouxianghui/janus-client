/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#include "webrtc_service.h"
#include "Service/unified_factory.h"
#include "janus_client.h"
#include "i_webrtc_event_handler.h"
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
#include "modules/audio_device/include/audio_device.h"
#include "modules/audio_processing/include/audio_processing.h"
#include "modules/video_capture/video_capture_factory.h"
#include "pc/video_track_source.h"
#include "local_video_capture.h"
#include "service/app_instance.h"
#include "task_scheduler.h"
#include "rtc_base/thread.h"
#include "sfu_listener_proxy.h"
#include "logger/logger.h"

namespace vi {

	static std::unordered_map<int64_t, std::weak_ptr<WebRTCService>> g_sessions;

	WebRTCService::WebRTCService()
	{
		_iceServers.emplace_back("stun:stun.l.google.com:19302");
	}

	WebRTCService::~WebRTCService()
	{
		_pcf = nullptr;

		if (_signaling) {
			_signaling->Stop();
		}

		if (_worker) {
			_worker->Stop();
		}

		if (_network) {
			_network->Stop();
		}

		if (_taskScheduler) {
			_taskScheduler->cancelAll();
		}

		DLOG("~WebRTCService");
	}

	void WebRTCService::init()
	{
		rtc::Thread* current = rtc::Thread::Current();

		auto listener = std::make_shared<SFUListener>();
		_proxy = vi::SFUListenerProxy::Create(current, listener);
		_proxy->attach(shared_from_this());

		_client = std::make_shared<vi::JanusClient>("ws://192.168.0.108:8188/ws");
		_client->addListener(_proxy);
		_client->init();

		_taskScheduler = std::make_shared<vi::TaskScheduler>();

		if (!_pcf) {
			_signaling = rtc::Thread::Create();
			_signaling->SetName("pc_signaling_thread", nullptr);
			_signaling->Start();
			_worker = rtc::Thread::Create();
			_worker->SetName("pc_worker_thread", nullptr);
			_worker->Start();
			_network = rtc::Thread::CreateWithSocketServer();
			_network->SetName("pc_network_thread", nullptr);
			_network->Start();
			_pcf = webrtc::CreatePeerConnectionFactory(
				_network.get() /* network_thread */,
				_worker.get() /* worker_thread */,
				_signaling.get() /* signaling_thread */,
				nullptr /* default_adm */,
				webrtc::CreateBuiltinAudioEncoderFactory(),
				webrtc::CreateBuiltinAudioDecoderFactory(),
				webrtc::CreateBuiltinVideoEncoderFactory(),
				webrtc::CreateBuiltinVideoDecoderFactory(),
				nullptr /* audio_mixer */,
				nullptr /* audio_processing */);
		}
	}

	void WebRTCService::cleanup()
	{
		auto event = std::make_shared<DestroySessionEvent>();
		event->notifyDestroyed = true;
		event->cleanupHandles = true;
		event->callback = std::make_shared<EventCallback>([](bool success, const std::string& reason) {
			DLOG("destroy, success = {}, reason = {}", success, reason.c_str());
		});
		this->destroy(event);
		//_pcf = nullptr;
	}

	// IWebRTCService implement

	void WebRTCService::addListener(std::shared_ptr<IWebRTCServiceListener> listener)
	{
		addBizObserver<IWebRTCServiceListener>(_listeners, listener);
	}

	void WebRTCService::removeListener(std::shared_ptr<IWebRTCServiceListener> listener)
	{
		removeBizObserver<IWebRTCServiceListener>(_listeners, listener);
	}

	ServiceStauts WebRTCService::status()
	{
		return _serviceStatus;
	}

	void WebRTCService::attach(const std::string& plugin, const std::string& opaqueId, std::shared_ptr<IWebRTCEventHandler> wreh)
	{
		auto wself = weak_from_this();
		auto lambda = [wself, wreh](std::shared_ptr<JanusResponse> model) {
			DLOG("model->janus = {}", model->janus);
			if (auto self = wself.lock()) {
				if (model->janus == "success") {
					int64_t handleId = model->data.id;
					if (wreh) {
						wreh->setHandleId(handleId);
						self->_wrehs[handleId] = wreh;
						wreh->onAttached(true);
					}
				}
				else if (model->janus == "error") {
					if (wreh) {
						wreh->onAttached(false);
					}
				}
			}
		};
		std::shared_ptr<JCCallback> callback = std::make_shared<JCCallback>(lambda);
		_client->attach(_sessionId, plugin, opaqueId, callback);
	}

	void WebRTCService::destroy(std::shared_ptr<DestroySessionEvent> event)
	{
		destroySession(event);
	}

	void WebRTCService::reconnect()
	{
		// TODO: ?
		std::shared_ptr<CreateSessionEvent> event = std::make_shared<CreateSessionEvent>();
		event->reconnect = true;
		auto lambda = [](bool success, const std::string& message) {
			std::string text = message;

		};
		event->callback = std::make_shared<vi::EventCallback>(lambda);
		createSession(event);
	}

	int32_t WebRTCService::getVolume(int64_t handleId, bool isRemote)
	{
		if (_wrehs.find(handleId) == _wrehs.end()) {
			DLOG("Invalid handle");
			return 0;
		}

		const auto& wreh = _wrehs[handleId];
		const auto& context = wreh->pluginContext()->webrtcContext;
		return 0;
	}

	int32_t WebRTCService::remoteVolume(int64_t handleId)
	{
		return getVolume(handleId, true);
	}

	int32_t WebRTCService::localVolume(int64_t handleId)
	{
		return getVolume(handleId, false);
	}

	bool WebRTCService::isAudioMuted(int64_t handleId) 
	{
		return isMuted(handleId, true);
	}

	bool WebRTCService::isVideoMuted(int64_t handleId)
	{
		return isMuted(handleId, false);
	}

	bool WebRTCService::isMuted(int64_t handleId, bool isVideo) 
	{
		if (_wrehs.find(handleId) == _wrehs.end()) {
			DLOG("Invalid handle");
			return true;
		}

		const auto& wreh = _wrehs[handleId];
		const auto& context = wreh->pluginContext()->webrtcContext;

		if (!context->pc) {
			DLOG("Invalid PeerConnection");
			return true;
		}
		if (!context->myStream) {
			DLOG("Invalid local MediaStream");
			return true;
		}
		if (isVideo) {
			// Check video track
			if (context->myStream->GetVideoTracks().size() == 0) {
				DLOG("No video track");
				return true;
			}
			return !context->myStream->GetVideoTracks()[0]->enabled();
		}
		else {
			// Check audio track
			if (context->myStream->GetAudioTracks().size() == 0) {
				DLOG("No audio track");
				return true;
			}
			return !context->myStream->GetAudioTracks()[0]->enabled();
		}
		return true;
	}

	bool WebRTCService::muteAudio(int64_t handleId)
	{
		return mute(handleId, false, true);
	}

	bool WebRTCService::muteVideo(int64_t handleId)
	{
		return mute(handleId, true, true);
	}

	bool WebRTCService::unmuteAudio(int64_t handleId)
	{
		return mute(handleId, false, false);
	}

	bool WebRTCService::unmuteVideo(int64_t handleId) 
	{
		return mute(handleId, true, false);
	}

	bool WebRTCService::mute(int64_t handleId, bool isVideo, bool mute)
	{
		if (_wrehs.find(handleId) == _wrehs.end()) {
			DLOG("Invalid handle");
			return false;
		}

		const auto& wreh = _wrehs[handleId];
		const auto& context = wreh->pluginContext()->webrtcContext;

		if (!context->pc) {
			DLOG("Invalid PeerConnection");
			return false;
		}
		if (!context->myStream) {
			DLOG("Invalid local MediaStream");
			return false;
		}
		if (isVideo) {
			// Mute/unmute video track
			if (context->myStream->GetVideoTracks().size() == 0) {
				DLOG("No video track");
				return false;
			}
			bool enabled = mute ? false : true;
			context->myStream->GetVideoTracks()[0]->set_enabled(enabled);
			return true;
		}
		else {
			// Mute/unmute audio track
			if (context->myStream->GetAudioTracks().size() == 0) {
				DLOG("No audio track");
				return false;
			}
			bool enabled = mute ? false : true;
			context->myStream->GetAudioTracks()[0]->set_enabled(enabled);
			return true;
		}
		return false;
	}

	std::string WebRTCService::getBitrate(int64_t handleId)
	{
		return "";
	}

	void WebRTCService::sendMessage(int64_t handleId, std::shared_ptr<SendMessageEvent> event)
	{
		if (status() == ServiceStauts::UP) {
			if (_wrehs.find(handleId) != _wrehs.end()) {
				const auto& wreh = _wrehs[handleId];
				auto wself = weak_from_this();
				auto lambda = [wself, wreh, event](std::shared_ptr<JanusResponse> model) {
					DLOG("model->janus = {}", model->janus);
					if (auto self = wself.lock()) {
						if (!event || !event->callback) {
							return;
						}
						if (model->janus == "success" || model->janus == "ack") {
							if (model->xhas("plugindata")) {
								//std::string data = model->plugindata.data;
							}
							const auto& cb = event->callback;
							(*cb)(true, "success");
						}
						else if (model->janus != "ack") {
							const auto& cb = event->callback;
							(*cb)(false, "error");
						}
					}
				};
				std::shared_ptr<JCCallback> callback = std::make_shared<JCCallback>(lambda);
				_client->sendMessage(_sessionId, handleId, event->message, event->jsep, callback);
			}
		}
		else {
			if (event && event->callback) {
				const auto& cb = event->callback;
				(*cb)(false, "service down!");
			}
		}
	}

	void WebRTCService::sendData(int64_t handleId, std::shared_ptr<SendDataEvent> event) 
	{
		if (!event) {
			DLOG("handler == nullptr");
			return;
		}

		if (_wrehs.find(handleId) == _wrehs.end()) {
			DLOG("Invalid handle");
			if (event->callback) {
				const auto& cb = event->callback;
				(*cb)(false, "Invalid handle");
			}
			return;
		}

		if (event->label.empty() || event->text.empty()) {
			DLOG("handler->label.empty() || handler->text.empty()");
			if (event->callback) {
				const auto& cb = event->callback;
				(*cb)(false, "empty label or empty text");
			}
			return;
		}

		const auto& wreh = _wrehs[handleId];
		const auto& context = wreh->pluginContext()->webrtcContext;

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
			this->createDataChannel(handleId, event->label, nullptr);
		}
		if (event->callback) {
			const auto& cb = event->callback;
			(*cb)(false, "success");
		}
	}

	void WebRTCService::sendDtmf(int64_t handleId, std::shared_ptr<SendDtmfEvent> event) 
	{
		if (!event) {
			DLOG("handler == nullptr");
			return;
		}

		if (_wrehs.find(handleId) == _wrehs.end()) {
			DLOG("Invalid handle");
			if (event->callback) {
				const auto& cb = event->callback;
				(*cb)(false, "Invalid handle");
			}
			return;
		}

		const auto& wreh = _wrehs[handleId];
		const auto& context = wreh->pluginContext()->webrtcContext;

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
						const auto& cb = event->callback;
						(*cb)(false, "Invalid DTMF configuration (no audio track)");
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
				const auto& cb = event->callback;
				(*cb)(false, "Invalid DTMF parameters");
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
			const auto& cb = event->callback;
			(*cb)(false, "success");
		}
	}

	void WebRTCService::prepareWebrtc(int64_t handleId, bool isOffer, std::shared_ptr<PrepareWebRTCEvent> event)
	{
		if (!event) {
			DLOG("handler == nullptr");
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
			
		if (_wrehs.find(handleId) == _wrehs.end()) {
			DLOG("Invalid handle");
				if (event->callback) {
					const auto& cb = event->callback;
					(*cb)(false, "Invalid handle");
				}
			return;
		}

		const auto& wreh = _wrehs[handleId];
		const auto& context = wreh->pluginContext()->webrtcContext;
		context->trickle = HelperUtils::isTrickleEnabled(event->trickle);
		if (!event->media.has_value()) {
			return;
		}
		auto& media = event->media.value();
		if (!context->pc) {
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
				if (event->stream != context->myStream) {
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
					if (context->myStream && context->myStream->GetAudioTracks().size() > 0) {
						DLOG("Can't add audio stream, there already is one");
						if (event->callback) {
							const auto& cb = event->callback;
							(*cb)(false, "Can't add audio stream, there already is one");
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
				if (!context->myStream) {
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
					if (context->myStream->GetAudioTracks().size() == 0) {
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
					if (context->myStream && context->myStream->GetVideoTracks().size() > 0) {
						DLOG("Can't add video stream, there already is one");
						if (event->callback) {
							const auto& cb = event->callback;
							(*cb)(false, "Can't add video stream, there already is one");
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
				if (!context->myStream) {
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
					if (context->myStream->GetVideoTracks().size() == 0) {
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
				// TODO:
				//pluginHandle.consentDialog(false);
				//streamsDone(handleId, jsep, media, callbacks, config.myStream);
				prepareStreams(handleId, event, context->myStream);
				return;
			}
		}
		// If we're updating, check if we need to remove/replace one of the tracks
		if (media.update && !context->streamExternal) {
			if (media.removeAudio || media.replaceAudio) {
				if (context->myStream && context->myStream->GetAudioTracks().size() > 0) {
					rtc::scoped_refptr<webrtc::AudioTrackInterface> s = context->myStream->GetAudioTracks()[0];
					DLOG("Removing audio track");
					context->myStream->RemoveTrack(s);
					try {
						s->set_enabled(false);
					}
					catch (...) {
					}
				}
				if (context->pc->GetSenders().size() > 0) {
					bool ra = true;
					if (media.replaceAudio && _unifiedPlan) {
						// We can use replaceTrack
						ra = false;
					}
					if (ra) {
						for (const auto& sender : context->pc->GetSenders()) {
							if (sender && sender->track() && sender->track()->kind() == "audio") {
								DLOG("Removing audio sender");
								context->pc->RemoveTrack(sender);
							}
						}
					}
				}
			}
			if (media.removeVideo || media.replaceVideo) {
				if (context->myStream && context->myStream->GetVideoTracks().size() > 0) {
					rtc::scoped_refptr<webrtc::VideoTrackInterface> s = context->myStream->GetVideoTracks()[0];
					DLOG("Removing video track");
					context->myStream->RemoveTrack(s);
					try {
						s->set_enabled(false);
					}
					catch (...) {
					}
				}
				if (context->pc->GetSenders().size() > 0) {
					bool ra = true;
					if (media.replaceVideo && _unifiedPlan) {
						// We can use replaceTrack
						ra = false;
					}
					if (ra) {
						for (const auto& sender : context->pc->GetSenders()) {
							if (sender && sender->track() && sender->track()->kind() == "video") {
								DLOG("Removing video sender");
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
				if (context->myStream && context->myStream != stream && !context->streamExternal) {
					// We're replacing a stream we captured ourselves with an external one
					try {
						// Try a MediaStreamTrack.stop() for each track
						const auto& aTracks = context->myStream->GetAudioTracks();
						for (const auto& track : aTracks) {
							track->set_enabled(false);
						}
						const auto& vTracks = context->myStream->GetVideoTracks();
						for (const auto& track : vTracks) {
							track->set_enabled(false);
						}
					}
					catch (...) {
						// Do nothing if this fails
					}
					context->myStream = nullptr;
				}
			}
			// Skip the getUserMedia part
			context->streamExternal = true;
			//pluginHandle.consentDialog(false);
			//streamsDone(handleId, jsep, media, callbacks, stream);
			prepareStreams(handleId, event, stream);
			return;
		}
		if (HelperUtils::isAudioSendEnabled(media) || HelperUtils::isVideoSendEnabled(media)) {
			rtc::scoped_refptr<webrtc::MediaStreamInterface> mstream = _pcf->CreateLocalMediaStream("stream_id");
			rtc::scoped_refptr<webrtc::AudioTrackInterface> audioTrack(_pcf->CreateAudioTrack("audio_label", _pcf->CreateAudioSource(cricket::AudioOptions())));
			std::string id = audioTrack->id();
			if (!mstream->AddTrack(audioTrack)) {
				DLOG("Add audio track failed.");
			}

			// TODO: hold the videoDevice
			_videoDevice = CapturerTrackSource::Create();
			if (_videoDevice) {
				rtc::scoped_refptr<webrtc::VideoTrackInterface> videoTrack(_pcf->CreateVideoTrack("video_label", _videoDevice));

				if (!mstream->AddTrack(videoTrack)) {
					DLOG("Add video track failed.");
				}
			}

			//context->pc->AddStream(mstream);
			prepareStreams(handleId, event, mstream);
		}
		else {
			//streamsDone(handleId, jsep, media, callbacks, stream);
			prepareStreams(handleId, event, nullptr);
		}
	}

	void WebRTCService::createOffer(int64_t handleId, std::shared_ptr<PrepareWebRTCEvent> event)
	{
		prepareWebrtc(handleId, true, event);
	}

	void WebRTCService::createAnswer(int64_t handleId, std::shared_ptr<PrepareWebRTCEvent> event)
	{
		prepareWebrtc(handleId, false, event);
	}

	void WebRTCService::prepareWebrtcPeer(int64_t handleId, std::shared_ptr<PrepareWebRTCPeerEvent> event)
	{
		if (!event) {
			DLOG("handler == nullptr");
			return;
		}

		if (_wrehs.find(handleId) == _wrehs.end()) {
			DLOG("Invalid handle");
			if (event->callback) {
				const auto& cb = event->callback;
				(*cb)(false, "Invalid handle");
			}
			return;
		}

		const auto& wreh = _wrehs[handleId];
		const auto& context = wreh->pluginContext()->webrtcContext;

		if (event->jsep.has_value()) {
			if (!context->pc) {
				DLOG("No PeerConnection: if this is an answer, use createAnswer and not handleRemoteJsep");
				if (event->callback) {
					const auto& cb = event->callback;
					(*cb)(false, "No PeerConnection: if this is an answer, use createAnswer and not handleRemoteJsep");
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
			SetSessionDescObserver* ssdo(new rtc::RefCountedObject<SetSessionDescObserver>());
			ssdo->setSuccessCallback(std::make_shared<SetSessionDescSuccessCallback>([context, event]() {
				context->remoteSdp = { event->jsep->type, event->jsep->sdp, false };

				for (const auto& candidate : context->candidates) {
					context->pc->AddIceCandidate(candidate.get());
				}
				context->candidates.clear();
				if (event->callback) {
					const auto& cb = event->callback;
					(*cb)(true, "success");
				}
			}));
			ssdo->setFailureCallback(std::make_shared<SetSessionDescFailureCallback>([event](webrtc::RTCError error) {
				DLOG("SetRemoteDescription() failure: {}", error.message());
				if (event->callback) {
					const auto& cb = event->callback;
					(*cb)(false, "failure");
				}
			}));
			context->pc->SetRemoteDescription(ssdo, desc.release());
		}
		else {
			DLOG("Invalid JSEP");
			if (event->callback) {
				const auto& cb = event->callback;
				(*cb)(false, "Invalid JSEP");
			}
		}
	}

	void WebRTCService::handleRemoteJsep(int64_t handleId, std::shared_ptr<PrepareWebRTCPeerEvent> event) 
	{
		prepareWebrtcPeer(handleId, event);
	}

	void WebRTCService::cleanupWebrtc(int64_t handleId, bool sendRequest)
	{
		DLOG("cleaning ...");
		if (_wrehs.find(handleId) == _wrehs.end()) {
			return;
		}
		const auto& wreh = _wrehs[handleId];
		const auto& context = wreh->pluginContext()->webrtcContext;
		if (context) {
			if (sendRequest == true) {
				auto lambda = [wself = weak_from_this()](std::shared_ptr<JanusResponse> model) {
					DLOG("model->janus = {}", model->janus);
					if (auto self = wself.lock()) {
					}
				};
				std::shared_ptr<JCCallback> callback = std::make_shared<JCCallback>(lambda);
				_client->hangup(_sessionId, handleId, callback);
			}
			context->remoteStream = nullptr;
			try {
				// Try a MediaStreamTrack.stop() for each track
				if (!context->streamExternal && context->myStream) {
					DLOG("Stopping local stream tracks");
					auto audioTracks = context->myStream->GetAudioTracks();
					for (auto t : audioTracks) {
						
					}
					auto videoTracks = context->myStream->GetVideoTracks();
					for (auto t : videoTracks) {

					}
				}
			}
			catch (...) {
				// Do nothing if this fails
			}
			context->streamExternal = false;
			context->myStream = nullptr;
			// Close PeerConnection
			try {
				if (context->pc) {
					context->pc->Close();
				}
			}
			catch (...) {
				// Do nothing
			}
			context->pc = nullptr;
			context->candidates.clear();
			context->mySdp = absl::nullopt;
			context->remoteSdp = absl::nullopt;
			context->iceDone = false;
			context->dataChannels.clear();
			context->dtmfSender = nullptr;
		}
		wreh->onCleanup();
	}

	void WebRTCService::hangup(int64_t handleId, bool sendRequest) 
	{
		cleanupWebrtc(handleId, sendRequest);
	}

	void WebRTCService::destroyHandle(int64_t handleId, std::shared_ptr<DetachEvent> event) 
	{
		cleanupWebrtc(handleId);
		if (_wrehs.find(handleId) == _wrehs.end()) {
			DLOG("Invalid handle");
			if (event && event->callback) {
				const auto& cb = event->callback;
				(*cb)(true, "");
			}
			return;
		}
		if (!event) {
			return;
		}
		if (event->noRequest) {
			// We're only removing the handle locally
			if (event->callback) {
				const auto& cb = event->callback;
				(*cb)(true, "");
			}
			return;
		}
		if (!_connected) {
			DLOG("Is the server down? (connected=false)");
			return;
		}

		const auto& wreh = _wrehs[handleId];
		auto wself = std::weak_ptr<WebRTCService>(shared_from_this());
		auto lambda = [wself](std::shared_ptr<JanusResponse> model) {
			DLOG("model->janus = {}", model->janus);
			if (auto self = wself.lock()) {
			}
		};
		std::shared_ptr<JCCallback> callback = std::make_shared<JCCallback>(lambda);
		_client->detach(_sessionId, handleId, callback);
	}

	void WebRTCService::detach(int64_t handleId, std::shared_ptr<DetachEvent> event) 
	{
		destroyHandle(handleId, event);
	}

	// ISFUClientListener
	void WebRTCService::onOpened()
	{
		std::shared_ptr<CreateSessionEvent> event = std::make_shared<CreateSessionEvent>();
		event->reconnect = false;
		auto lambda = [](bool success, const std::string& message) {
			std::string text = message;

		};
		event->callback = std::make_shared<vi::EventCallback>(lambda);
		createSession(event);
	}

	void WebRTCService::onClosed()
	{

	}	
	
	void WebRTCService::onFailed(int errorCode, const std::string& reason)
	{

	}

	void WebRTCService::onMessage(std::shared_ptr<JanusResponse> model)
	{
		int32_t retries = 0;

		if (model->janus == "keepalive") {
			DLOG("Got a keepalive on session: {}", _sessionId);
			return;
		}
		else if (model->janus == "trickle") {
			// We got a trickle candidate from Janus
			int64_t sender = model->sender;
			auto& wreh = getHandler(sender);
			if (!wreh) {
				return;
			}

			bool hasCandidata = model->xhas("candidate");

			auto& context = wreh->pluginContext()->webrtcContext;
			if (context->pc && context->remoteSdp) {
				// Add candidate right now
				if (!hasCandidata 
					|| (hasCandidata && model->xhas("completed") && model->candidate.completed == true)) {
					// end-of-candidates
					context->pc->AddIceCandidate(nullptr);
				}
				else {
					if (hasCandidata &&  model->candidate.xhas("sdpMid") && model->candidate.xhas("sdpMLineIndex")) {
						const auto& candidate = model->candidate.candidate;
						DLOG("Got a trickled candidate on session: ", _sessionId);
						DLOG("Adding remote candidate: {}", candidate.c_str());
						DLOG("candidate: {}", candidate.c_str());
						webrtc::SdpParseError spError;
						std::unique_ptr<webrtc::IceCandidateInterface> ici(webrtc::CreateIceCandidate(model->candidate.sdpMid,
							model->candidate.sdpMLineIndex,
							model->candidate.candidate,
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

				if (hasCandidata &&  model->candidate.xhas("sdpMid") && model->candidate.xhas("sdpMLineIndex")) {
					const auto& candidate = model->candidate.candidate;
					DLOG("Got a trickled candidate on session: {}", _sessionId);
					DLOG("Adding remote candidate: {}", candidate.c_str());
					DLOG("candidate: {}", candidate.c_str());
					webrtc::SdpParseError spError;
					std::shared_ptr<webrtc::IceCandidateInterface> ici(webrtc::CreateIceCandidate(model->candidate.sdpMid,
						model->candidate.sdpMLineIndex,
						model->candidate.candidate,
						&spError));
					DLOG("candidate error: {}", spError.description.c_str());

					context->candidates.emplace_back(ici);
				}
			}
		}
		else if (model->janus == "webrtcup") {
			// The PeerConnection with the server is up! Notify this
			DLOG("Got a webrtcup event on session: {}", _sessionId);
			int64_t sender = model->sender;
			auto& wreh = getHandler(sender);
			if (!wreh) {
				return;
			}
			wreh->onWebrtcState(true, "");
			return;
		}
		else if (model->janus == "hangup") {
			// A plugin asked the core to hangup a PeerConnection on one of our handles
			DLOG("Got a hangup event on session: {}", _sessionId);
			int64_t sender = model->sender;
			auto& wreh = getHandler(sender);
			if (!wreh) {
				return;
			}
			wreh->onWebrtcState(false, model->reason);
			wreh->onHangup();
		}
		else if (model->janus == "detached") {
			// A plugin asked the core to detach one of our handles
			DLOG("Got a detached event on session: {}", _sessionId);
			int64_t sender = model->sender;
			auto& wreh = getHandler(sender);
			if (!wreh) {
				return;
			}
			wreh->onDetached();
		}
		else if (model->janus == "media") {
			// Media started/stopped flowing
			DLOG("Got a media event on session: {}", _sessionId);
			int64_t sender = model->sender;
			auto& wreh = getHandler(sender);
			if (!wreh) {
				return;
			}
			wreh->onMediaState(model->type, model->receiving);
		}
		else if (model->janus == "slowlink") {
			DLOG("Got a slowlink event on session: {}", _sessionId);
			int64_t sender = model->sender;
			auto& wreh = getHandler(sender);
			if (!wreh) {
				return;
			}
			wreh->onSlowLink(model->uplink, model->lost);
		}
		else if (model->janus == "event") {
			DLOG("Got a plugin event on session: {}", _sessionId);
			int64_t sender = model->sender;
			auto& wreh = getHandler(sender);
			if (!wreh) {
				return;
			}

			if (!model->xhas("plugindata")) {
				ELOG("Missing plugindata...");
				return;
			}
			
			const auto& plugindata = model->plugindata;

			DLOG(" -- Event is coming from {} ({})", sender, plugindata.plugin.c_str());

			std::string dataString = x2struct::X::tojson(plugindata.data);
			EventData data;
			x2struct::X::loadjson(dataString, data, false, true);

			Jsep jsep;
			if (model->xhas("jsep")) {
				DLOG("Handling SDP as well...");
				jsep = model->jsep;
			}

			wreh->onMessage(data, jsep);
		}
		else if (model->janus == "timeout") {
			ELOG("Timeout on session: {}", _sessionId);
			//if (websockets) {
			//	ws.close(3504, "Gateway timeout");
			//}
			return;
		}
		else {
			WLOG("Unknown message/event {} on session: {}'", model->janus.c_str(),  _sessionId);
		}
	}

	void WebRTCService::createSession(std::shared_ptr<CreateSessionEvent> event)
	{
		auto wself = std::weak_ptr<WebRTCService>(shared_from_this());
		auto lambda = [wself, event](std::shared_ptr<JanusResponse> model) {
			DLOG("model->janus = {}", model->janus);
			if (auto self = wself.lock()) {
				self->_sessionId = model->session_id > 0 ? model->session_id : model->data.id;
				g_sessions[self->_sessionId] = self;
				self->startHeartbeat();
				self->_serviceStatus = ServiceStauts::UP;
				self->notifyObserver4Change<IWebRTCServiceListener>(self->_listeners, [](const std::shared_ptr<IWebRTCServiceListener>& listener) {
					listener->onStatus(ServiceStauts::UP);
				});
				if (event && event->callback) {
					const auto& cb = event->callback;
					(*cb)(true, "");
				}
			}
		};
		std::shared_ptr<JCCallback> callback = std::make_shared<JCCallback>(lambda);
		if (event && event->reconnect) {
			_client->reconnectSession(_sessionId, callback);
		}
		else {
			_client->createSession(callback);
		}
	}

	void WebRTCService::startHeartbeat()
	{
		auto wself = std::weak_ptr<WebRTCService>(shared_from_this());
		_heartbeatTaskId = _taskScheduler->schedule([wself]() {
			if (auto self = wself.lock()) {
				DLOG("sessionHeartbeat() called");
				auto lambda = [](std::shared_ptr<JanusResponse> model) {
					DLOG("model->janus = {}", model->janus);
				};
				std::shared_ptr<JCCallback> callback = std::make_shared<JCCallback>(lambda);
				self->_client->keepAlive(self->_sessionId, callback);
			}
		}, 5000, true);
	}

	std::shared_ptr<IWebRTCEventHandler> WebRTCService::getHandler(int64_t handleId)
	{
		if (handleId == -1) {
			ELOG("Missing sender...");
			return nullptr;
		}
		if (_wrehs.find(handleId) == _wrehs.end()) {
			ELOG("This handle is not attached to this session");
			return nullptr;
		}
		return _wrehs[handleId];
	}

	void WebRTCService::prepareStreams(int64_t handleId,
		std::shared_ptr<PrepareWebRTCEvent> event,
		rtc::scoped_refptr<webrtc::MediaStreamInterface> stream)
	{
		if (_wrehs.find(handleId) == _wrehs.end()) {
			ELOG("Invalid handle");
			if (event->callback) {
				const auto& cb = event->callback;
				(*cb)(false, "Invalid handle");
			}
			return;
		}

		const auto& wreh = _wrehs[handleId];
		const auto& context = wreh->pluginContext()->webrtcContext;

		if (stream) {
			DLOG("audio tracks: {}", stream->GetAudioTracks().size());
			DLOG("video tracks: {}", stream->GetVideoTracks().size());
		}

		bool addTracks = false;
		if (!context->myStream || !event->media->update || context->streamExternal) {
			context->myStream = stream;
			addTracks = true;
		}
		else {
			if ((!event->media->update && HelperUtils::isAudioSendEnabled(event->media)) ||
				(event->media->update && (event->media->addAudio || event->media->replaceAudio)) ||
				stream->GetAudioTracks().size() > 0) {
				if (_unifiedPlan) {
					// Use Transceivers
					rtc::scoped_refptr<webrtc::RtpTransceiverInterface> audioTransceiver = nullptr;
					auto transceivers = context->pc->GetTransceivers();
					for (const auto& t : transceivers) {
						if ((t->sender() && t->sender()->track() && t->sender()->track()->kind() == "audio") ||
							(t->receiver() && t->receiver()->track() && t->receiver()->track()->kind() == "audio")) {
							audioTransceiver = t;
							break;
						}
					}
					if (audioTransceiver && audioTransceiver->sender()) {
						// TODO:
						//audioTransceiver->sender()->ReplaceTrack(stream->GetAudioTracks()[0]);
						audioTransceiver->sender()->SetTrack(stream->GetAudioTracks()[0]);
					}
					else {
						context->pc->AddTrack(stream->GetAudioTracks()[0], { stream->id() });
					}
				}
				else {
					context->pc->AddTrack(stream->GetAudioTracks()[0], { stream->id() });
				}
			}
			if ((!event->media->update && HelperUtils::isVideoSendEnabled(event->media)) ||
				(event->media->update && (event->media->addVideo || event->media->replaceVideo)) ||
				stream->GetVideoTracks().size() > 0) {
				if (_unifiedPlan) {
					// Use Transceivers
					rtc::scoped_refptr<webrtc::RtpTransceiverInterface> videoTransceiver = nullptr;
					auto transceivers = context->pc->GetTransceivers();
						for (const auto& t : transceivers) {
						if ((t->sender() && t->sender()->track() && t->sender()->track()->kind() == "video") ||
							(t->receiver() && t->receiver()->track() && t->receiver()->track()->kind() == "video")) {
							videoTransceiver = t;
							break;
						}
					}
					if (videoTransceiver && videoTransceiver->sender()) {
						// TODO:
						//videoTransceiver->sender()->ReplaceTrack(stream->GetVideoTracks()[0]);
						videoTransceiver->sender()->SetTrack(stream->GetVideoTracks()[0]);
					}
					else {
						context->pc->AddTrack(stream->GetVideoTracks()[0], { stream->id() });
					}
				}
				else {
					context->pc->AddTrack(stream->GetVideoTracks()[0], { stream->id() });
				}
			}
		}
		if (!context->pc) {
			webrtc::PeerConnectionInterface::RTCConfiguration pcConfig;
			for (const auto& server : _iceServers) {
				webrtc::PeerConnectionInterface::IceServer stunServer;
				stunServer.uri = server;
				pcConfig.servers.emplace_back(stunServer);
			}
			pcConfig.enable_rtp_data_channel = true;
			pcConfig.enable_dtls_srtp = true;
			pcConfig.sdp_semantics = webrtc::SdpSemantics::kUnifiedPlan;
			//pcConfig.bundle_policy = webrtc::PeerConnectionInterface::kBundlePolicyMaxBundle;
			//pcConfig.type = webrtc::PeerConnectionInterface::kRelay;
			//pcConfig.use_media_transport = true;

			std::weak_ptr<IWebRTCEventHandler> wwreh(wreh);
			std::weak_ptr<WebRTCService> wself(weak_from_this());

			context->pcObserver = std::make_unique<PCObserver>();

			auto icccb = std::make_shared<IceConnectionChangeCallback>([wwreh](webrtc::PeerConnectionInterface::IceConnectionState newState) {
				if (auto wreh = wwreh.lock()) {
					wreh->onIceState(newState);
				}
			});
			context->pcObserver->setIceConnectionChangeCallback(icccb);

			auto igccb = std::make_shared<IceGatheringChangeCallback>([wwreh](webrtc::PeerConnectionInterface::IceGatheringState newState) {
				if (auto wreh = wwreh.lock()) {
					
				}
			});
			context->pcObserver->setIceGatheringChangeCallback(igccb);

			auto iccb = std::make_shared<IceCandidateCallback>([wwreh, wself, event](const webrtc::IceCandidateInterface* candidate) {
				auto self = wself.lock();
				auto wreh = wwreh.lock();
				if (!self || !wreh) {
					return;
				}
				auto handleId = wreh->pluginContext()->handleId;
				CandidateData data;
				candidate->ToString(&data.candidate);
				data.sdpMid = candidate->sdp_mid();
				data.sdpMLineIndex = (int)candidate->sdp_mline_index();
				data.completed = false;

				if (wreh->pluginContext()->webrtcContext->trickle) {
					self->_client->sendTrickleCandidate(self->_sessionId, handleId, data, nullptr);
				}
				else {
					self->sendSDP(handleId, event);
				}
			});
			context->pcObserver->setIceCandidateCallback(iccb);

			auto atcb = std::make_shared<AddTrackCallback>([wwreh, wself, event](rtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver) {
				auto self = wself.lock();
				auto wreh = wwreh.lock();
				if (!self || !wreh) {
					return;
				}
				//auto handleId = wreh->pluginContext()->handleId;
				auto& context = wreh->pluginContext()->webrtcContext;
				context->remoteStream = transceiver->receiver()->streams()[0];
				wreh->onCreateRemoteStream(context->remoteStream);
			});
			context->pcObserver->setAddTrackCallback(atcb);

			auto rtcb = std::make_shared<RemoveTrackCallback>([wwreh, wself, event](rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver) {
				auto self = wself.lock();
				auto wreh = wwreh.lock();
				if (!self || !wreh) {
					return;
				}
				//auto handleId = wreh->pluginContext()->handleId;
				auto& context = wreh->pluginContext()->webrtcContext;
				if (context->remoteStream && !receiver->streams().empty() && (receiver->streams()[0]->id() == context->remoteStream->id())) {
					wreh->onDeleteRemoteStream(context->remoteStream);
					context->remoteStream = nullptr;
				}
			});
			context->pcObserver->setRemoveTrackCallback(rtcb);

			context->pc = _pcf->CreatePeerConnection(pcConfig, nullptr, nullptr, context->pcObserver.get());

			if (addTracks && stream) {
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

			if (event->media
				&& HelperUtils::isDataEnabled(event->media.value())
				&& context->dataChannels.find("JanusDataChannel") == context->dataChannels.end()) {
				auto dccb = std::make_shared<DataChannelCallback>([wself = weak_from_this(), handleId](rtc::scoped_refptr<webrtc::DataChannelInterface> dataChannel) {
					DLOG("Data channel created by Janus.");
					if (auto self = wself.lock()) {
						self->createDataChannel(handleId, dataChannel->label(), dataChannel);
					}
				});
				context->pcObserver->setDataChannelCallback(dccb);
			}

			if (context->myStream) {
				wreh->onCreateLocalStream(context->myStream);
			}

			if (event->jsep == absl::nullopt) {
				// TODO:
				_createOffer(handleId, event);
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
				auto wself = weak_from_this();
				ssdo->setSuccessCallback(std::make_shared<SetSessionDescSuccessCallback>([wself, context, handleId, event]() {
					context->remoteSdp = { event->jsep->type, event->jsep->sdp, false };

					for (const auto& candidate : context->candidates) {
						context->pc->AddIceCandidate(candidate.get());
					}
					context->candidates.clear();
					if (auto self = wself.lock()) {
						self->_createAnswer(handleId, event);
					}
				}));
				ssdo->setFailureCallback(std::make_shared<SetSessionDescFailureCallback>([event](webrtc::RTCError error) {
					DLOG("SetRemoteDescription() failure: {}", error.message());
					if (event->callback) {
						const auto& cb = event->callback;
						(*cb)(false, "failure");
					}
				}));
				context->pc->SetRemoteDescription(ssdo, desc.release());
			}
		}
	}

	void WebRTCService::sendSDP(int64_t handleId, std::shared_ptr<PrepareWebRTCEvent> event)
	{
		if (_wrehs.find(handleId) == _wrehs.end()) {
			DLOG("Invalid handle, not sending anything");
			return;
		}

		const auto& wreh = _wrehs[handleId];
		const auto& context = wreh->pluginContext()->webrtcContext;
		if (auto ld = context->pc->local_description()) {
			std::string sdp;
			ld->ToString(&sdp);
			context->mySdp = { ld->type(), sdp, context->trickle.value_or(false) };
			context->sdpSent = true;
			if (event->answerOfferCallback) {
				const auto& cb = event->answerOfferCallback;
				(*cb)(true, "", context->mySdp.value());
			}
		}
	}

	void WebRTCService::createDataChannel(int64_t handleId, const std::string& dcLabel, rtc::scoped_refptr<webrtc::DataChannelInterface> incoming)
	{
		if (_wrehs.find(handleId) == _wrehs.end()) {
			DLOG("Invalid handle");
			return;
		}

		const auto& wreh = _wrehs[handleId];
		const auto& context = wreh->pluginContext()->webrtcContext;

		if (incoming) {
			context->dataChannels[dcLabel] = incoming;
		}
		else {
			webrtc::DataChannelInit init;
			auto dataChannel = context->pc->CreateDataChannel(dcLabel, &init);
			context->dataChannels[dcLabel] = dataChannel;
		}

		std::shared_ptr<DCObserver> observer = std::make_shared<DCObserver>();

		std::weak_ptr<IWebRTCEventHandler> wwreh(wreh);

		auto scc = std::make_shared<StateChangeCallback>([wwreh, dcLabel, context]() {
			if (context->dataChannels.find(dcLabel) != context->dataChannels.end()) {
				auto dc = context->dataChannels[dcLabel];
				if (dc->state() == webrtc::DataChannelInterface::DataState::kOpen) {
					if (auto wreh = wwreh.lock()) {
						wreh->onDataOpen(dcLabel);
					}
				}
			}
		});
		observer->setStateChangeCallback(scc);

		auto mc = std::make_shared<MessageCallback>([wwreh, dcLabel](const webrtc::DataBuffer& buffer) {
			if (auto wreh = wwreh.lock()) {
				size_t size = buffer.data.size();
				char* msg = new char[size + 1];
				memcpy(msg, buffer.data.data(), size);
				msg[size] = 0;
				wreh->onData(std::string(msg, size), dcLabel);
				delete[] msg;
			}
		});
		observer->setMessageCallback(mc);

		context->dataChannelObservers[dcLabel] = observer;

		auto dc = context->dataChannels[dcLabel];
		dc->RegisterObserver(observer.get());
	}

	void WebRTCService::configTracks(const MediaConfig& media, rtc::scoped_refptr<webrtc::PeerConnectionInterface> pc)
	{
		if (!pc) {
			return;
		}
		rtc::scoped_refptr<webrtc::RtpTransceiverInterface> audioTransceiver = nullptr;
		rtc::scoped_refptr<webrtc::RtpTransceiverInterface> videoTransceiver = nullptr;
		std::vector<rtc::scoped_refptr<webrtc::RtpTransceiverInterface>> transceivers = pc->GetTransceivers();
		for (auto t : transceivers) {
			if ((t->sender() && t->sender()->track() && t->sender()->track()->kind() == "audio") ||
				(t->receiver() && t->receiver()->track() && t->receiver()->track()->kind() == "audio")) {
				if (!audioTransceiver)
					audioTransceiver = t;
				continue;
			}
			if ((t->sender() && t->sender()->track() && t->sender()->track()->kind() == "video") ||
				(t->receiver() && t->receiver()->track() && t->receiver()->track()->kind() == "video")) {
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

	void WebRTCService::_createOffer(int64_t handleId, std::shared_ptr<PrepareWebRTCEvent> event)
	{
		if (_wrehs.find(handleId) == _wrehs.end()) {
			ELOG("Invalid handle");
			return;
		}

		const auto& wreh = _wrehs[handleId];
		const auto& context = wreh->pluginContext()->webrtcContext;

		auto& media = event->media.value();

		webrtc::PeerConnectionInterface::RTCOfferAnswerOptions options;

		if (_unifiedPlan) {
			configTracks(media, context->pc);
		}
		else {
			options.offer_to_receive_audio = HelperUtils::isAudioRecvEnabled(media);
			options.offer_to_receive_video = HelperUtils::isVideoRecvEnabled(media);
		}

		options.ice_restart = event->iceRestart.value_or(false);

		bool sendVideo = HelperUtils::isVideoSendEnabled(media);
		bool simulcast = event->simulcast.value_or(false);

		if (sendVideo && simulcast) {
			std::vector<rtc::scoped_refptr<webrtc::RtpSenderInterface>> senders = context->pc->GetSenders();
			rtc::scoped_refptr<webrtc::RtpSenderInterface> sender;
			for (auto& s : senders) {
				if (s->track()->kind() == "video") {
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

		std::shared_ptr<CreateSessionDescSuccessCallback> success = std::make_shared<CreateSessionDescSuccessCallback>([event, context, options](webrtc::SessionDescriptionInterface* desc) {
			if (!desc) {
				ELOG("Invalid description.");
				return;
			}

			SetSessionDescObserver* ssdo(new rtc::RefCountedObject<SetSessionDescObserver>());
			ssdo->setSuccessCallback(std::make_shared<SetSessionDescSuccessCallback>([context, event]() {
				DLOG("Set session description success.");
			}));
			ssdo->setFailureCallback(std::make_shared<SetSessionDescFailureCallback>([event](webrtc::RTCError error) {
				DLOG("SetLocalDescription() failure: {}", error.message());
				if (event->callback) {
					const auto& cb = event->callback;
					(*cb)(false, "failure");
				}
			}));
			context->pc->SetLocalDescription(ssdo, desc);
			context->options = options;
			if (!context->iceDone && !context->trickle.value_or(false)) {
				// Don't do anything until we have all candidates
				DLOG("Waiting for all candidates...");
				return;
			}

			std::string sdp;
			desc->ToString(&sdp);
			JsepConfig jsep{ desc->type(), sdp, false };
			if (event->answerOfferCallback) {
				const auto& cb = event->answerOfferCallback;
				(*cb)(true, "", jsep);
			}
		});

		std::shared_ptr<CreateSessionDescFailureCallback> failure = std::make_shared<CreateSessionDescFailureCallback>([event](webrtc::RTCError error) {
			DLOG("createOfferObserver() failure: {}", error.message());
			if (event->callback) {
				const auto& cb = event->callback;
				(*cb)(false, "failure");
			}
		});

		createOfferObserver->setSuccessCallback(success);
		createOfferObserver->setFailureCallback(failure);

		context->pc->CreateOffer(createOfferObserver.release(), options);
	}

	void WebRTCService::_createAnswer(int64_t handleId, std::shared_ptr<PrepareWebRTCEvent> event)
	{
		if (_wrehs.find(handleId) == _wrehs.end()) {
			DLOG("Invalid handle");
			return;
		}

		const auto& wreh = _wrehs[handleId];
		const auto& context = wreh->pluginContext()->webrtcContext;

		auto& media = event->media.value();

		webrtc::PeerConnectionInterface::RTCOfferAnswerOptions options;

		if (_unifiedPlan) {
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
		bool simulcast = event->simulcast.value_or(false);

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
		std::unique_ptr<CreateSessionDescObserver> createAnswerObserver;
		createAnswerObserver.reset(new rtc::RefCountedObject<CreateSessionDescObserver>());

		std::shared_ptr<CreateSessionDescSuccessCallback> success = std::make_shared<CreateSessionDescSuccessCallback>([event, context, options](webrtc::SessionDescriptionInterface* desc) {
			if (!desc) {
				ELOG("Invalid description.");
				return;
			}
			SetSessionDescObserver* ssdo(new rtc::RefCountedObject<SetSessionDescObserver>());
			ssdo->setSuccessCallback(std::make_shared<SetSessionDescSuccessCallback>([context, event]() {
				DLOG("Set session description success.");
			}));
			ssdo->setFailureCallback(std::make_shared<SetSessionDescFailureCallback>([event](webrtc::RTCError error) {
				DLOG("SetLocalDescription() failure: {}", error.message());
				if (event->callback) {
					const auto& cb = event->callback;
					(*cb)(false, "failure");
				}
			}));
			context->pc->SetLocalDescription(ssdo, desc);
			context->options = options;
			if (!context->iceDone && !context->trickle.value_or(false)) {
				// Don't do anything until we have all candidates
				DLOG("Waiting for all candidates...");
				return;
			}

			std::string sdp;
			desc->ToString(&sdp);
			JsepConfig jsep{ desc->type(), sdp, false };
			if (event->answerOfferCallback) {
				const auto& cb = event->answerOfferCallback;
				(*cb)(true, "", jsep);
			}
		});

		std::shared_ptr<CreateSessionDescFailureCallback> failure = std::make_shared<CreateSessionDescFailureCallback>([event](webrtc::RTCError error) {
			DLOG("CreateAnswer() failure: {}", error.message());
			if (event->callback) {
				const auto& cb = event->callback;
				(*cb)(false, "failure");
			}
		});

		createAnswerObserver->setSuccessCallback(success);
		createAnswerObserver->setFailureCallback(failure);

		context->pc->CreateAnswer(createAnswerObserver.release(), options);
	}

	void WebRTCService::destroySession(std::shared_ptr<DestroySessionEvent> event)
	{
		DLOG("Destroying session: {}", _sessionId);
		if (_sessionId == -1) {
			DLOG("No session to destroy");
			if (event && event->callback) {
				const auto& cb = event->callback;
				(*cb)(true, "");
			}
			if (event->notifyDestroyed) {
				// TODO:
				//gatewayCallbacks.destroyed();
			}
			return;
		}
		if (!event) {
			return;
		}
		if (event->cleanupHandles) {
			for (auto pair : _wrehs) {
				std::shared_ptr<DetachEvent> dh = std::make_shared<DetachEvent>();
				dh->noRequest = true;
				int64_t hId = pair.first;
				auto lambda = [hId](bool success, const std::string& message) {
					DLOG("destroyHandle, handleId = {}, success = {}, message = {}", hId, success, message.c_str());
				};
				dh->callback = std::make_shared<vi::EventCallback>(lambda);
				destroyHandle(hId, dh);
			}
			//_client->removeListener(shared_from_this());
			_wrehs.clear();
		}
		if (!_connected) {
			DLOG("Is the server down? (connected=false)");
			if(event->callback) {
				const auto& cb = event->callback;
				(*cb)(true, "");
			}
			return;
		}
	}
}
