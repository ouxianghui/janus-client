/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#include "webrtc_service.h"
#include "Service/unified_factory.h"
#include "janus_api_client.h"
#include "plugin_client.h"
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
#include "logger/logger.h"
#include "service/app_instance.h"
#include "thread_manager.h"
#include "message_models.h"
#include "sdp_utils.h"

namespace vi {

	static std::unordered_map<int64_t, std::weak_ptr<WebRTCService>> g_sessions;

	WebRTCService::WebRTCService(rtc::Thread* callbackThread)
		: _callbackThread(callbackThread)
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
		//std::string url = "ws://192.168.0.108:8188/ws";
		_client = std::make_shared<vi::JanusApiClient>(rtc::Thread::Current());
		_client->addListener(shared_from_this());
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
		event->callback = std::make_shared<EventCallback>([](bool success, const std::string& response) {
			DLOG("destroy, success = {}, response = {}", success, response.c_str());
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

	void WebRTCService::connect(const std::string& url)
	{
		if (!_client) {
			DLOG("_client == nullptr");
			return;
		}
		DLOG("janus api client, connecting...");
		_client->connect(url);
	}

	ServiceStauts WebRTCService::status()
	{
		return _serviceStatus;
	}

	void WebRTCService::attach(const std::string& plugin, const std::string& opaqueId, std::shared_ptr<PluginClient> pluginClient)
	{
		if (!pluginClient) {
			return;
		}

		auto wself = weak_from_this();
		auto lambda = [wself, pluginClient](const std::string& json) {
			AttachResponse model;
			x2struct::X::loadjson(json, model, false, true);
			DLOG("model->janus = {}", model.janus);
			if (auto self = wself.lock()) {
				if (model.janus == "success") {
					int64_t handleId = model.data.id;
					pluginClient->setHandleId(handleId);
					self->_pluginClientMap[handleId] = pluginClient;
					self->_callbackThread->PostTask(RTC_FROM_HERE, [pluginClient]() {
						pluginClient->onAttached(true);
					});
				}
				else if (model.janus == "error") {
					self->_callbackThread->PostTask(RTC_FROM_HERE, [pluginClient]() {
						pluginClient->onAttached(false);
					});
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

	void WebRTCService::reconnectSession()
	{
		// TODO: ?
		std::shared_ptr<CreateSessionEvent> event = std::make_shared<CreateSessionEvent>();
		event->reconnect = true;
		auto lambda = [](bool success, const std::string& response) {
			DLOG("response: {}", response.c_str());
		};
		event->callback = std::make_shared<vi::EventCallback>(lambda);
		createSession(event);
	}

	int32_t WebRTCService::getVolume(int64_t handleId, bool isRemote)
	{
		const auto& pluginClient = getHandler(handleId);
		if (!pluginClient) {
			DLOG("Invalid handle");
			return 0;
		}

		const auto& context = pluginClient->pluginContext()->webrtcContext;
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
		const auto& pluginClient = getHandler(handleId);
		if (!pluginClient) {
			DLOG("Invalid handle");
			return true;
		}

		const auto& context = pluginClient->pluginContext()->webrtcContext;

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
		const auto& pluginClient = getHandler(handleId);
		if (!pluginClient) {
			DLOG("Invalid handle");
			return false;
		}

		const auto& context = pluginClient->pluginContext()->webrtcContext;

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
			if (const auto& pluginClient = getHandler(handleId)) {
				auto wself = weak_from_this();
				auto lambda = [wself, pluginClient, event](const std::string& json) {
					DLOG("janus = {}", json);
					if (auto self = wself.lock()) {
						if (!event) {
							return;
						}
						JanusResponse model;
						x2struct::X::loadjson(json, model, false, true);

						if (event->callback) {
							if (model.janus == "success" || model.janus == "ack") {
								self->_callbackThread->PostTask(RTC_FROM_HERE, [cb = event->callback, json]() {
									if (cb) {
										(*cb)(true, json);
									}
								});
							}
							else if (model.janus != "ack") {
								self->_callbackThread->PostTask(RTC_FROM_HERE, [cb = event->callback, json]() {
									if (cb) {
										(*cb)(false, json);
									}
								});
							}
						}
					}
				};
				std::shared_ptr<JCCallback> callback = std::make_shared<JCCallback>(lambda);
				_client->sendMessage(_sessionId, handleId, event->message, event->jsep, callback);
			}
		}
		else {
			if (event && event->callback) {
				_callbackThread->PostTask(RTC_FROM_HERE, [cb = event->callback]() {
					(*cb)(false, "service down!");
				});
			}
		}
	}

	void WebRTCService::sendData(int64_t handleId, std::shared_ptr<SendDataEvent> event) 
	{
		if (!event) {
			DLOG("handler == nullptr");
			return;
		}

		const auto& pluginClient = getHandler(handleId);
		if (!pluginClient) {
			DLOG("Invalid handle");
			if (event->callback) {
				_callbackThread->PostTask(RTC_FROM_HERE, [cb = event->callback]() {
					(*cb)(false, "Invalid handle");
				});
			}
			return;
		}

		if (event->label.empty() || event->text.empty()) {
			DLOG("handler->label.empty() || handler->text.empty()");
			if (event->callback) {
				_callbackThread->PostTask(RTC_FROM_HERE, [cb = event->callback]() {
					(*cb)(false, "empty label or empty text");
				});
			}
			return;
		}

		const auto& context = pluginClient->pluginContext()->webrtcContext;

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
			_callbackThread->PostTask(RTC_FROM_HERE, [cb = event->callback]() {
				(*cb)(false, "success");
			});
		}
	}

	void WebRTCService::sendDtmf(int64_t handleId, std::shared_ptr<SendDtmfEvent> event) 
	{
		if (!event) {
			DLOG("handler == nullptr");
			return;
		}

		const auto& pluginClient = getHandler(handleId);
		if (!pluginClient) {
			DLOG("Invalid handle");
			if (event->callback) {
				_callbackThread->PostTask(RTC_FROM_HERE, [cb = event->callback]() {
					(*cb)(false, "Invalid handle");
				});
			}
			return;
		}

		const auto& context = pluginClient->pluginContext()->webrtcContext;

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
						_callbackThread->PostTask(RTC_FROM_HERE, [cb = event->callback]() {
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
				_callbackThread->PostTask(RTC_FROM_HERE, [cb = event->callback]() {
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
			_callbackThread->PostTask(RTC_FROM_HERE, [cb = event->callback]() {
				(*cb)(false, "success");
			});
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
			
		const auto& pluginClient = getHandler(handleId);
		if (!pluginClient) {
			DLOG("Invalid handle");
				if (event->callback) {
					_callbackThread->PostTask(RTC_FROM_HERE, [cb = event->callback]() {
						(*cb)(false, "Invalid handle");
					});
				}
			return;
		}

		const auto& context = pluginClient->pluginContext()->webrtcContext;
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
						ELOG("Can't add audio stream, there already is one");
						if (event->callback) {
							_callbackThread->PostTask(RTC_FROM_HERE, [cb = event->callback]() {
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
						ELOG("Can't add video stream, there already is one");
						if (event->callback) {
							_callbackThread->PostTask(RTC_FROM_HERE, [cb = event->callback]() {
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
				// TODO: notify ?
				//streams done
				prepareStreams(handleId, event, context->myStream);
				return;
			}
		}
		// If we're updating, check if we need to remove/replace one of the tracks
		if (media.update && !context->streamExternal) {
			if (media.removeAudio || media.replaceAudio) {
				if (context->myStream && context->myStream->GetAudioTracks().size() > 0) {
					rtc::scoped_refptr<webrtc::AudioTrackInterface> at = context->myStream->GetAudioTracks()[0];
					DLOG("Removing audio track, id = {}", at->id());
					context->myStream->RemoveTrack(at);
					try {
						at->set_enabled(false);
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
								DLOG("Removing audio sender, id = {}, ssrc = {}", sender->id(), sender->ssrc());
								context->pc->RemoveTrack(sender);
							}
						}
					}
				}
			}
			if (media.removeVideo || media.replaceVideo) {
				if (context->myStream && context->myStream->GetVideoTracks().size() > 0) {
					rtc::scoped_refptr<webrtc::VideoTrackInterface> vt = context->myStream->GetVideoTracks()[0];
					DLOG("Removing video track, id = {}", vt->id());
					context->myStream->RemoveTrack(vt);
					try {
						vt->set_enabled(false);
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
				if (context->myStream && context->myStream != stream && !context->streamExternal) {
					// We're replacing a stream we captured ourselves with an external one
					stopAllTracks(context->myStream);
					context->myStream = nullptr;
				}
			}
			// Skip the getUserMedia part
			context->streamExternal = true;
			// TODO: notify ?
			// streams done
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
			// No need to do a getUserMedia, create offer/answer right away
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

		const auto& pluginClient = getHandler(handleId);
		if (!pluginClient) {
			DLOG("Invalid handle");
			if (event->callback) {
				_callbackThread->PostTask(RTC_FROM_HERE, [cb = event->callback]() {
					(*cb)(false, "Invalid handle");
				});
			}
			return;
		}

		const auto& context = pluginClient->pluginContext()->webrtcContext;

		if (event->jsep.has_value()) {
			if (!context->pc) {
				DLOG("No PeerConnection: if this is an answer, use createAnswer and not handleRemoteJsep");
				if (event->callback) {
					_callbackThread->PostTask(RTC_FROM_HERE, [cb = event->callback]() {
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
			ssdo->setSuccessCallback(std::make_shared<SetSessionDescSuccessCallback>([context, event, wself]() {
				context->remoteSdp = { event->jsep->type, event->jsep->sdp, false };

				for (const auto& candidate : context->candidates) {
					context->pc->AddIceCandidate(candidate.get());
				}
				context->candidates.clear();
				auto self = wself.lock();
				if (event->callback && self) {
					self->_callbackThread->PostTask(RTC_FROM_HERE, [cb = event->callback]() {
						(*cb)(true, "success");
					});
				}
			}));
			ssdo->setFailureCallback(std::make_shared<SetSessionDescFailureCallback>([event, wself](webrtc::RTCError error) {
				DLOG("SetRemoteDescription() failure: {}", error.message());
				auto self = wself.lock();
				if (event->callback && self) {
					self->_callbackThread->PostTask(RTC_FROM_HERE, [cb = event->callback]() {
						(*cb)(false, "failure");
					});
				}
			}));
			context->pc->SetRemoteDescription(ssdo, desc.release());
		}
		else {
			DLOG("Invalid JSEP");
			if (event->callback) {
				_callbackThread->PostTask(RTC_FROM_HERE, [cb = event->callback]() {
					(*cb)(false, "Invalid JSEP");
				});
			}
		}
	}

	void WebRTCService::handleRemoteJsep(int64_t handleId, std::shared_ptr<PrepareWebRTCPeerEvent> event) 
	{
		prepareWebrtcPeer(handleId, event);
	}

	void WebRTCService::cleanupWebrtc(int64_t handleId, bool hangupRequest)
	{
		DLOG("cleaning webrtc ...");

		const auto& pluginClient = getHandler(handleId);
		if (!pluginClient) {
			return;
		}

		const auto& context = pluginClient->pluginContext()->webrtcContext;
		if (context) {
			if (hangupRequest == true) {
				auto lambda = [wself = weak_from_this()](const std::string& json) {
					DLOG("janus = {}", json);
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
					stopAllTracks(context->myStream);
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
		_callbackThread->PostTask(RTC_FROM_HERE, [pluginClient]() {
			pluginClient->onCleanup();
		});
	}

	void WebRTCService::hangup(int64_t handleId, bool hangupRequest)
	{
		cleanupWebrtc(handleId, hangupRequest);
	}

	void WebRTCService::destroyHandle(int64_t handleId, std::shared_ptr<DetachEvent> event) 
	{
		cleanupWebrtc(handleId);

		const auto& pluginClient = getHandler(handleId);
		if (!pluginClient) {
			DLOG("Invalid handle");
			if (event && event->callback) {
				_callbackThread->PostTask(RTC_FROM_HERE, [cb = event->callback]() {
					(*cb)(true, "");
				});
			}
			return;
		}
		if (!event) {
			return;
		}
		if (event->noRequest) {
			// We're only removing the handle locally
			if (event->callback) {
				_callbackThread->PostTask(RTC_FROM_HERE, [cb = event->callback]() {
					(*cb)(true, "");
				});
			}
			return;
		}
		if (!_connected) {
			DLOG("Is the server down? (connected = false)");
			return;
		}

		auto wself = weak_from_this();
		auto lambda = [wself, handleId](const std::string& json) {
			DLOG("janus = {}", json);
			if (auto self = wself.lock()) {
				self->_pluginClientMap.erase(handleId);
			}
		};
		std::shared_ptr<JCCallback> callback = std::make_shared<JCCallback>(lambda);
		_client->detach(_sessionId, handleId, callback);
	}

	void WebRTCService::detach(int64_t handleId, std::shared_ptr<DetachEvent> event) 
	{
		destroyHandle(handleId, event);
	}

	// ISfuApiClientListener

	void WebRTCService::onOpened()
	{
		auto wself = weak_from_this();
		std::shared_ptr<CreateSessionEvent> event = std::make_shared<CreateSessionEvent>();
		event->reconnect = false;
		auto lambda = [wself](bool success, const std::string& response) {
			if (auto self = wself.lock()) {
				self->_connected = true;
			}
		};
		event->callback = std::make_shared<vi::EventCallback>(lambda);
		createSession(event);
	}

	void WebRTCService::onClosed()
	{
		_connected = false;
	}	
	
	void WebRTCService::onFailed(int errorCode, const std::string& reason)
	{
		_connected = false;
	}

	void WebRTCService::onMessage(const std::string& json)
	{
		JanusResponse response;
		x2struct::X::loadjson(json, response, false, true);

		int64_t sender = response.sender;
		auto& pluginClient = getHandler(sender);
		if (!pluginClient) {
			return;
		}

		int32_t retries = 0;

		if (response.janus == "keepalive") {
			DLOG("Got a keepalive on session: {}", _sessionId);
			return;
		}
		else if (response.janus == "server_info") {
			// Just info on the Janus instance
			DLOG("Got info on the Janus instance: {}", response.janus.c_str());
		}
		else if (response.janus == "trickle") {
			TrickleResponse model;
			x2struct::X::loadjson(json, model, false, true);
			// We got a trickle candidate from Janus
			bool hasCandidata = model.xhas("candidate");

			auto& context = pluginClient->pluginContext()->webrtcContext;
			if (context->pc && context->remoteSdp) {
				// Add candidate right now
				if (!hasCandidata || (hasCandidata && model.candidate.xhas("completed") && model.candidate.completed == true)) {
					// end-of-candidates
					context->pc->AddIceCandidate(nullptr);
				}
				else {
					if (hasCandidata && model.candidate.xhas("sdpMid") && model.candidate.xhas("sdpMLineIndex")) {
						const auto& candidate = model.candidate.candidate;
						DLOG("Got a trickled candidate on session: ", _sessionId);
						DLOG("Adding remote candidate: {}", candidate.c_str());
						DLOG("candidate: {}", candidate.c_str());
						webrtc::SdpParseError spError;
						std::unique_ptr<webrtc::IceCandidateInterface> ici(webrtc::CreateIceCandidate(model.candidate.sdpMid,
							model.candidate.sdpMLineIndex,
							model.candidate.candidate,
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

				if (hasCandidata &&  model.candidate.xhas("sdpMid") && model.candidate.xhas("sdpMLineIndex")) {
					const auto& candidate = model.candidate.candidate;
					DLOG("Got a trickled candidate on session: {}", _sessionId);
					DLOG("Adding remote candidate: {}", candidate.c_str());
					DLOG("candidate: {}", candidate.c_str());
					webrtc::SdpParseError spError;
					std::shared_ptr<webrtc::IceCandidateInterface> ici(webrtc::CreateIceCandidate(model.candidate.sdpMid,
						model.candidate.sdpMLineIndex,
						model.candidate.candidate,
						&spError));
					DLOG("candidate error: {}", spError.description.c_str());

					context->candidates.emplace_back(ici);
				}
			}
		}
		else if (response.janus == "webrtcup") {
			// The PeerConnection with the server is up! Notify this
			DLOG("Got a webrtcup event on session: {}", _sessionId);

			_callbackThread->PostTask(RTC_FROM_HERE, [pluginClient]() {
				pluginClient->onWebrtcState(true, "");
			});

			return;
		}
		else if (response.janus == "hangup") {
			// A plugin asked the core to hangup a PeerConnection on one of our handles
			DLOG("Got a hangup event on session: {}", _sessionId);

			HangupResponse model;
			x2struct::X::loadjson(json, model, false, true);
			_callbackThread->PostTask(RTC_FROM_HERE, [pluginClient, reason = model.reason]() {
				pluginClient->onWebrtcState(false, reason);
				pluginClient->onHangup();
			});
		}
		else if (response.janus == "detached") {
			// A plugin asked the core to detach one of our handles
			DLOG("Got a detached event on session: {}", _sessionId);

			_callbackThread->PostTask(RTC_FROM_HERE, [pluginClient]() {
				pluginClient->onDetached();
			});
		}
		else if (response.janus == "media") {
			// Media started/stopped flowing
			DLOG("Got a media event on session: {}", _sessionId);
			MediaResponse model;
			x2struct::X::loadjson(json, model, false, true);

			_callbackThread->PostTask(RTC_FROM_HERE, [pluginClient, model]() {
				pluginClient->onMediaState(model.type, model.receiving);
			});
		}
		else if (response.janus == "slowlink") {
			DLOG("Got a slowlink event on session: {}", _sessionId);
			SlowlinkResponse model;
			x2struct::X::loadjson(json, model, false, true);
			_callbackThread->PostTask(RTC_FROM_HERE, [pluginClient, model]() {
				pluginClient->onSlowLink(model.uplink, model.lost);
			});
		}
		else if (response.janus == "event") {
			DLOG("Got a plugin event on session: {}", _sessionId);

			JanusEvent event;
			x2struct::X::loadjson(json, event, false, true);

			if (!event.xhas("plugindata")) {
				ELOG("Missing plugindata...");
				return;
			}
			
			DLOG(" -- Event is coming from {} ({})", sender, event.plugindata.plugin.c_str());

			//std::string data = x2struct::X::tojson(event.plugindata);
			std::string jsep = x2struct::X::tojson(event.jsep);

			_callbackThread->PostTask(RTC_FROM_HERE, [pluginClient, json, jsep]() {
				pluginClient->onMessage(json, jsep);
			});
		}
		else if (response.janus == "timeout") {
			ELOG("Timeout on session: {}", _sessionId);
			// TODO:
			return;
		}
		else if (response.janus == "error") {
			// something wrong happened
			DLOG("Something wrong happened: {}", response.janus.c_str());
		}
		else {
			WLOG("Unknown message/event {} on session: {}'", response.janus.c_str(),  _sessionId);
		}
	}

	void WebRTCService::createSession(std::shared_ptr<CreateSessionEvent> event)
	{
		auto wself = weak_from_this();
		auto lambda = [wself, event](const std::string& json) {
			CreateSessionResponse model;
			x2struct::X::loadjson(json, model, false, true);
			DLOG("model.janus = {}", model.janus);
			if (auto self = wself.lock()) {
				self->_sessionId = model.session_id > 0 ? model.session_id : model.data.id;
				g_sessions[self->_sessionId] = self;
				self->startHeartbeat();
				self->_serviceStatus = ServiceStauts::UP;
				self->notifyObserver4Change<IWebRTCServiceListener>(self->_listeners, [](const std::shared_ptr<IWebRTCServiceListener>& listener) {
					listener->onStatus(ServiceStauts::UP);
				});
				if (event && event->callback) {
					//self->_callbackThread->PostTask(RTC_FROM_HERE, [cb = event->callback]() {
					const auto& cb = event->callback;
						(*cb)(true, "");
					//});
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
		auto wself = weak_from_this();
		_heartbeatTaskId = _taskScheduler->schedule([wself]() {
			if (auto self = wself.lock()) {
				DLOG("sessionHeartbeat() called");
				auto lambda = [](const std::string& json) {
					DLOG("janus = {}", json);
				};
				std::shared_ptr<JCCallback> callback = std::make_shared<JCCallback>(lambda);
				self->_client->keepAlive(self->_sessionId, callback);
			}
		}, 5000, true);
	}

	std::shared_ptr<PluginClient> WebRTCService::getHandler(int64_t handleId)
	{
		if (handleId == -1) {
			ELOG("Missing sender...");
			return nullptr;
		}
		if (_pluginClientMap.find(handleId) == _pluginClientMap.end()) {
			ELOG("This handle is not attached to this session");
			return nullptr;
		}
		return _pluginClientMap[handleId];
	}

	void WebRTCService::stopAllTracks(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) {
		try {
			for (const auto& track : stream->GetAudioTracks()) {
				if (track) {
					track->set_enabled(false);
				}
			}
			for (const auto& track : stream->GetVideoTracks()) {
				if (track) {
					track->set_enabled(false);
				}
			}
		}
		catch (...) {
			// Do nothing if this fails
		}
	}

	void WebRTCService::prepareStreams(int64_t handleId,
		std::shared_ptr<PrepareWebRTCEvent> event,
		rtc::scoped_refptr<webrtc::MediaStreamInterface> stream)
	{
		const auto& pluginClient = getHandler(handleId);
		if (!pluginClient) {
			ELOG("Invalid handle");
			// Close all tracks if the given stream has been created internally
			if (!event->stream) {
				stopAllTracks(event->stream);
			}
			if (event->callback) {
				_callbackThread->PostTask(RTC_FROM_HERE, [cb = event->callback]() {
					(*cb)(false, "Invalid handle");
				});
			}
			return;
		}

		auto wself = weak_from_this();

		const auto& context = pluginClient->pluginContext()->webrtcContext;

		if (stream) {
			DLOG("audio tracks: {}", stream->GetAudioTracks().size());
			DLOG("video tracks: {}", stream->GetVideoTracks().size());
		}

		// We're now capturing the new stream: check if we're updating or if it's a new thing
		bool addTracks = false;
		if (!context->myStream || !event->media->update || context->streamExternal) {
			context->myStream = stream;
			addTracks = true;
		}
		else {
			// We only need to update the existing stream
			if (((!event->media->update && HelperUtils::isAudioSendEnabled(event->media)) || 
				(event->media->update && (event->media->addAudio || event->media->replaceAudio))) &&
				stream->GetAudioTracks().size() > 0) {
				context->myStream->AddTrack(stream->GetAudioTracks()[0]);
				if (_unifiedPlan) {
					// Use Transceivers
					DLOG("{} audio track", (event->media->replaceAudio ? "Replacing" : "Adding"));
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
				context->myStream->AddTrack(stream->GetVideoTracks()[0]);
				if (_unifiedPlan) {
					// Use Transceivers
					DLOG("{} video track", (event->media->replaceVideo ? "Replacing" : "Adding"));
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
			
			DLOG("Creating PeerConnection");

			auto wself = weak_from_this();

			context->pcObserver = std::make_unique<PCObserver>();

			DLOG("Preparing local SDP and gathering candidates (trickle = {})", pluginClient->pluginContext()->webrtcContext->trickle ? "true" : "false");

			auto icccb = std::make_shared<IceConnectionChangeCallback>([pluginClient, wself](webrtc::PeerConnectionInterface::IceConnectionState newState) {
				if (auto self = wself.lock()) {
					self->_callbackThread->PostTask(RTC_FROM_HERE, [pluginClient, newState]() {
						pluginClient->onIceState(newState);
					});
				}
			});
			context->pcObserver->setIceConnectionChangeCallback(icccb);

			auto igccb = std::make_shared<IceGatheringChangeCallback>([](webrtc::PeerConnectionInterface::IceGatheringState newState) {

			});
			context->pcObserver->setIceGatheringChangeCallback(igccb);

			auto iccb = std::make_shared<IceCandidateCallback>([pluginClient, wself, event](const webrtc::IceCandidateInterface* candidate) {
				auto self = wself.lock();
				if (!self) {
					return;
				}

				auto handleId = pluginClient->pluginContext()->handleId;
				if (candidate) {
					if (pluginClient->pluginContext()->webrtcContext->trickle) {
						CandidateData data;
						candidate->ToString(&data.candidate);
						data.sdpMid = candidate->sdp_mid();
						data.sdpMLineIndex = (int)candidate->sdp_mline_index();
						data.completed = false;
						self->_client->sendTrickleCandidate(self->_sessionId, handleId, data, nullptr);
					}
				} 
				else {
					DLOG("End of candidates.");
					pluginClient->pluginContext()->webrtcContext->iceDone = true;
					if (pluginClient->pluginContext()->webrtcContext->trickle) {
						CandidateData data;
						data.completed = true;
						self->_client->sendTrickleCandidate(self->_sessionId, handleId, data, nullptr);
					}
					else {
						// should be called in SERVICE thread
						DLOG("send candidates.");
						TMgr->thread(ThreadName::SERVICE)->PostTask(RTC_FROM_HERE, [wself, handleId, event]() {
							if (auto self = wself.lock()) {
								self->sendSDP(handleId, event);
							}
						});
					}
				}

			});

			context->pcObserver->setIceCandidateCallback(iccb);

			auto atcb = std::make_shared<AddTrackCallback>([pluginClient, wself, event](rtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver) {
				DLOG("Adding Remote Track");
				auto self = wself.lock();
				if (!self) {
					return;
				}
				if (transceiver->media_type() == cricket::MediaType::MEDIA_TYPE_VIDEO) {
					auto& context = pluginClient->pluginContext()->webrtcContext;
					context->remoteStream = transceiver->receiver()->streams()[0];
					self->_callbackThread->PostTask(RTC_FROM_HERE, [pluginClient, context]() {
						pluginClient->onCreateRemoteStream(context->remoteStream);
					});
				}
			});

			context->pcObserver->setAddTrackCallback(atcb);

			auto rtcb = std::make_shared<RemoveTrackCallback>([pluginClient, wself, event](rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver) {
				DLOG("Removing Remote Track");
				auto self = wself.lock();
				if (!self) {
					return;
				}

				auto& context = pluginClient->pluginContext()->webrtcContext;
				if (context->remoteStream && !receiver->streams().empty() && (receiver->streams()[0]->id() == context->remoteStream->id())) {
					self->_callbackThread->PostTask(RTC_FROM_HERE, [pluginClient, context]() {
						pluginClient->onRemoveRemoteStream(context->remoteStream);
						context->remoteStream = nullptr;
					});
				}
			});

			context->pcObserver->setRemoveTrackCallback(rtcb);

			context->pc = _pcf->CreatePeerConnection(pcConfig, nullptr, nullptr, context->pcObserver.get());

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
				auto dccb = std::make_shared<DataChannelCallback>([wself = weak_from_this(), handleId](rtc::scoped_refptr<webrtc::DataChannelInterface> dataChannel) {
					DLOG("Data channel created by Janus.");
					if (auto self = wself.lock()) {
						// should be called in SERVICE thread
						TMgr->thread(ThreadName::SERVICE)->PostTask(RTC_FROM_HERE, [wself, handleId, dataChannel]() {
							if (auto self = wself.lock()) {
								self->createDataChannel(handleId, dataChannel->label(), dataChannel);
							}
						});
					}
				});
				context->pcObserver->setDataChannelCallback(dccb);
			}

			if (context->myStream) {
				_callbackThread->PostTask(RTC_FROM_HERE, [pluginClient, context]() {
					pluginClient->onCreateLocalStream(context->myStream);
				});
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

				auto wself = weak_from_this();

				SetSessionDescObserver* ssdo(new rtc::RefCountedObject<SetSessionDescObserver>());

				ssdo->setSuccessCallback(std::make_shared<SetSessionDescSuccessCallback>([wself, context, handleId, event]() {
					context->remoteSdp = { event->jsep->type, event->jsep->sdp, false };

					for (const auto& candidate : context->candidates) {
						context->pc->AddIceCandidate(candidate.get());
					}
					context->candidates.clear();
					if (auto self = wself.lock()) {
						// should be called in SERVICE thread
						TMgr->thread(ThreadName::SERVICE)->PostTask(RTC_FROM_HERE, [wself, handleId, event]() {
							if (auto self = wself.lock()) {
								self->_createAnswer(handleId, event);
							}
						});
					}
				}));

				ssdo->setFailureCallback(std::make_shared<SetSessionDescFailureCallback>([event, wself](webrtc::RTCError error) {
					DLOG("SetRemoteDescription() failure: {}", error.message());
					auto self = wself.lock();
					if (event->callback && self) {
						self->_callbackThread->PostTask(RTC_FROM_HERE, [cb = event->callback]() {
							(*cb)(false, "failure");
						});
					}
				}));

				context->pc->SetRemoteDescription(ssdo, desc.release());
			}
		}
	}

	void WebRTCService::sendSDP(int64_t handleId, std::shared_ptr<PrepareWebRTCEvent> event)
	{
		const auto& pluginClient = getHandler(handleId);
		if (!pluginClient) {
			ELOG("Invalid handle, not sending anything");
			return;
		}

		DLOG("Sending offer/answer SDP...");
		const auto& context = pluginClient->pluginContext()->webrtcContext;
		if (!context->mySdp) {
			WLOG("Local SDP instance is invalid, not sending anything...");
			return;
		}

		if (auto ld = context->pc->local_description()) {
			std::string sdp;
			ld->ToString(&sdp);
			context->mySdp = { ld->type(), sdp, context->trickle.value_or(false) };
			context->sdpSent = true;
			if (event && event->answerOfferCallback) {
				_callbackThread->PostTask(RTC_FROM_HERE, [cb = event->answerOfferCallback, context]() {
					(*cb)(true, "", context->mySdp.value());
				});
			}
		}
	}

	void WebRTCService::createDataChannel(int64_t handleId, const std::string& dcLabel, rtc::scoped_refptr<webrtc::DataChannelInterface> incoming)
	{
		const auto& pluginClient = getHandler(handleId);
		if (!pluginClient) {
			ELOG("Invalid handle");
			return;
		}

		const auto& context = pluginClient->pluginContext()->webrtcContext;
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

		auto scc = std::make_shared<StateChangeCallback>([pluginClient, dcLabel, context, wself]() {
			if (context->dataChannels.find(dcLabel) != context->dataChannels.end()) {
				auto self = wself.lock();
				if (!self) {
					return;
				}
				auto dc = context->dataChannels[dcLabel];
				if (dc->state() == webrtc::DataChannelInterface::DataState::kOpen) {
					self->_callbackThread->PostTask(RTC_FROM_HERE, [pluginClient, dcLabel]() {
						pluginClient->onDataOpen(dcLabel);
					});
				}
			}
		});
		observer->setStateChangeCallback(scc);

		auto mc = std::make_shared<MessageCallback>([pluginClient, dcLabel, wself](const webrtc::DataBuffer& buffer) {
			auto self = wself.lock();
			if (!self) {
				return;
			}
			self->_callbackThread->PostTask(RTC_FROM_HERE, [pluginClient, buffer, dcLabel]() {
				size_t size = buffer.data.size();
				char* msg = new char[size + 1];
				memcpy(msg, buffer.data.data(), size);
				msg[size] = 0;
				pluginClient->onData(std::string(msg, size), dcLabel);
				delete[] msg;
			});
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
		const auto& pluginClient = getHandler(handleId);
		if (!pluginClient) {
			ELOG("Invalid handle");
			return;
		}

		const auto& context = pluginClient->pluginContext()->webrtcContext;
		bool simulcast = event->simulcast.value_or(false);
		if (!simulcast) {
			DLOG("Creating offer (iceDone = {})", context->iceDone ? "true" : "false");
		}
		else {
			DLOG("Creating offer (iceDone = {}, simulcast = {})", context->iceDone ? "true" : "false", simulcast ? "enabled" : "disabled");
		}

		webrtc::PeerConnectionInterface::RTCOfferAnswerOptions options;
		auto& media = event->media.value();
		if (_unifiedPlan) {
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

		auto wself = weak_from_this();
		std::shared_ptr<CreateSessionDescSuccessCallback> success = std::make_shared<CreateSessionDescSuccessCallback>([event, context, options, wself, sendVideo, simulcast](webrtc::SessionDescriptionInterface* desc) {
			if (!desc) {
				ELOG("Invalid description.");
				return;
			}

			SetSessionDescObserver* ssdo(new rtc::RefCountedObject<SetSessionDescObserver>());

			ssdo->setSuccessCallback(std::make_shared<SetSessionDescSuccessCallback>([context, event]() {
				DLOG("Set session description success.");
			}));

			ssdo->setFailureCallback(std::make_shared<SetSessionDescFailureCallback>([event, wself](webrtc::RTCError error) {
				DLOG("SetLocalDescription() failure: {}", error.message());
				auto self = wself.lock();
				if (event->callback && self) {
					self->_callbackThread->PostTask(RTC_FROM_HERE, [cb = event->callback]() {
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

			context->mySdp = jsep;
			context->pc->SetLocalDescription(ssdo, desc);
			context->options = options;
			if (!context->iceDone && !context->trickle.value_or(false)) {
				// Don't do anything until we have all candidates
				DLOG("Waiting for all candidates...");
				return;
			}

			auto self = wself.lock();
			if (event->answerOfferCallback && self) {
				self->_callbackThread->PostTask(RTC_FROM_HERE, [cb = event->answerOfferCallback, jsep]() {
					(*cb)(true, "", jsep);
				});
			}
		});

		std::shared_ptr<CreateSessionDescFailureCallback> failure = std::make_shared<CreateSessionDescFailureCallback>([event, wself](webrtc::RTCError error) {
			DLOG("createOfferObserver() failure: {}", error.message());
			auto self = wself.lock();
			if (event->callback && self) {
				self->_callbackThread->PostTask(RTC_FROM_HERE, [cb = event->callback]() {
					(*cb)(false, "failure");
				});
			}
		});

		createOfferObserver->setSuccessCallback(success);
		createOfferObserver->setFailureCallback(failure);

		context->pc->CreateOffer(createOfferObserver.release(), options);
	}

	void WebRTCService::_createAnswer(int64_t handleId, std::shared_ptr<PrepareWebRTCEvent> event)
	{
		const auto& pluginClient = getHandler(handleId);
		if (!pluginClient) {
			DLOG("Invalid handle");
			return;
		}

		const auto& context = pluginClient->pluginContext()->webrtcContext;
		bool simulcast = event->simulcast.value_or(false);
		if (!simulcast) {
			DLOG("Creating offer (iceDone = {})", context->iceDone ? "true" : "false");
		}
		else {
			DLOG("Creating offer (iceDone = {}, simulcast = {})", context->iceDone ? "true" : "false", simulcast ? "enabled" : "disabled");
		}

		webrtc::PeerConnectionInterface::RTCOfferAnswerOptions options;
		auto& media = event->media.value();
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

		std::shared_ptr<CreateSessionDescSuccessCallback> success = std::make_shared<CreateSessionDescSuccessCallback>([event, context, options, wself, sendVideo, simulcast](webrtc::SessionDescriptionInterface* desc) {
			if (!desc) {
				ELOG("Invalid description.");
				return;
			}

			SetSessionDescObserver* ssdo(new rtc::RefCountedObject<SetSessionDescObserver>());

			ssdo->setSuccessCallback(std::make_shared<SetSessionDescSuccessCallback>([context, event]() {
				DLOG("Set session description success.");
			}));

			ssdo->setFailureCallback(std::make_shared<SetSessionDescFailureCallback>([event, wself](webrtc::RTCError error) {
				DLOG("SetLocalDescription() failure: {}", error.message());
				auto self = wself.lock();
				if (event->callback && self) {
					self->_callbackThread->PostTask(RTC_FROM_HERE, [cb = event->callback]() {
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

			context->mySdp = jsep;
			context->pc->SetLocalDescription(ssdo, desc);
			context->options = options;
			if (!context->iceDone && !context->trickle.value_or(false)) {
				// Don't do anything until we have all candidates
				DLOG("Waiting for all candidates...");
				return;
			}

			auto self = wself.lock();
			if (event->answerOfferCallback && self) {
				self->_callbackThread->PostTask(RTC_FROM_HERE, [cb = event->answerOfferCallback, jsep]() {
					(*cb)(true, "", jsep);
				});
			}
		});

		std::shared_ptr<CreateSessionDescFailureCallback> failure = std::make_shared<CreateSessionDescFailureCallback>([event, wself](webrtc::RTCError error) {
			DLOG("CreateAnswer() failure: {}", error.message());
			if (event->callback) {
				auto self = wself.lock();
				if (event->callback && self) {
					self->_callbackThread->PostTask(RTC_FROM_HERE, [cb = event->callback]() {
						(*cb)(false, "failure");
					});
				}
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
				_callbackThread->PostTask(RTC_FROM_HERE, [cb = event->callback]() {
					(*cb)(true, "");
				});
			}
			if (event->notifyDestroyed) {
				// TODO:
			}
			return;
		}
		if (!event) {
			return;
		}
		if (event->cleanupHandles) {
			for (auto pair : _pluginClientMap) {
				std::shared_ptr<DetachEvent> dh = std::make_shared<DetachEvent>();
				dh->noRequest = true;
				int64_t hId = pair.first;
				auto lambda = [hId](bool success, const std::string& response) {
					DLOG("destroyHandle, handleId = {}, success = {}, response = {}", hId, success, response.c_str());
				};
				dh->callback = std::make_shared<vi::EventCallback>(lambda);
				destroyHandle(hId, dh);
			}

			_pluginClientMap.clear();
		}
		if (!_connected) {
			DLOG("Is the server down? (connected = false)");
			if(event->callback) {
				//_callbackThread->PostTask(RTC_FROM_HERE, [cb = event->callback]() {
				const auto& cb = event->callback;
				(*cb)(true, "");
				//});
			}
			return;
		}

		// TODO: destroy session from janus 
		auto wself = weak_from_this();
		auto lambda = [wself](const std::string& json) {
			DLOG("janus = {}", json);
			if (auto self = wself.lock()) {
				self->_client->removeListener(self);
			}
		};
		std::shared_ptr<JCCallback> callback = std::make_shared<JCCallback>(lambda);
		_client->destroySession(_sessionId, callback);
	}
}
