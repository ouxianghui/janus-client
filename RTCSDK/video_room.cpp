/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#include "video_room.h"
#include "string_utils.h"
#include "logger/logger.h"
#include "participant.h"
#include "x2struct.hpp"
#include "thread_manager.h"
#include "Service/app_instance.h"
#include "video_room_api.h"

namespace vi {
	VideoRoom::VideoRoom(std::shared_ptr<WebRTCServiceInterface> wrs)
		: PluginClient(wrs)
	{
		_pluginContext->plugin = "janus.plugin.videoroom";
		_pluginContext->opaqueId = "videoroom-" + StringUtils::randomString(12);
		_listeners = std::make_shared<std::vector<std::weak_ptr<IVideoRoomListener>>>();
	}

	VideoRoom::~VideoRoom()
	{
		if (_pluginContext->webrtcContext->pc) {
			_pluginContext->webrtcContext->pc->Close();
		}
	}

	void VideoRoom::init()
	{
		_videoRoomApi = std::make_shared<VideoRoomApi>(shared_from_this());
	}

	void VideoRoom::addListener(std::shared_ptr<IVideoRoomListener> listener)
	{
		addBizObserver<IVideoRoomListener>(*_listeners, listener);
	}

	void VideoRoom::removeListener(std::shared_ptr<IVideoRoomListener> listener)
	{
		removeBizObserver<IVideoRoomListener>(*_listeners, listener);
	}

	std::shared_ptr<PluginClient> VideoRoom::getParticipant(int64_t pid)
	{
		if (pid == this->_id) {
			return shared_from_this();
		}
		else {
			return _participantsMap.find(pid) == _participantsMap.end() ? nullptr : _participantsMap[pid];
		}
	}

	std::shared_ptr<IVideoRoomApi> VideoRoom::getVideoRoomApi()
	{
		return _videoRoomApi;
	}

	void VideoRoom::onAttached(bool success)
	{
		if (success) {
			DLOG("Plugin attached! ({}, id = {})", _pluginContext->plugin.c_str(), _id);
			DLOG("  -- This is a publisher/manager");
		}
		else {
			ELOG("  -- Error attaching plugin...");
		}
	}

	void VideoRoom::onHangup() {}

	void VideoRoom::onIceState(webrtc::PeerConnectionInterface::IceConnectionState iceState) {}

	void VideoRoom::onMediaState(const std::string& media, bool on) 
	{
		DLOG("Janus {} receiving our {}", (on ? "started" : "stopped"), media.c_str());
	}

	void VideoRoom::onWebrtcState(bool isActive, const std::string& reason) 
	{
		DLOG("Janus says our WebRTC PeerConnection is {} now", (isActive ? "up" : "down"));
		if (isActive) {
			vr::PublisherConfigureRequest request;
			request.request = "configure";
			request.bitrate = 256000;

			if (auto webrtcService = _pluginContext->webrtcService.lock()) {
				std::shared_ptr<SendMessageEvent> event = std::make_shared<vi::SendMessageEvent>();
				auto lambda = [](bool success, const std::string& response) {
					DLOG("response: {}", response.c_str());
				};
				std::shared_ptr<vi::EventCallback> callback = std::make_shared<vi::EventCallback>(lambda);
				event->message = x2struct::X::tojson(request);
				event->callback = callback;
				sendMessage(event);
			}
		}
		unmuteVideo();
	}

	void VideoRoom::onSlowLink(bool uplink, bool lost) {}

	void VideoRoom::onMessage(const EventData& data, const Jsep& jsep)
	{
		DLOG(" ::: Got a message (publisher).");
		if (!data.xhas("videoroom")) {
			return;
		}
		const auto& event = data.videoroom;
		if (event == "joined") {
			// Publisher/manager created, negotiate WebRTC and attach to existing feeds, if any
			_id = data.id;
			_privateId = data.private_id;
			DLOG("Successfully joined room {} with ID {}", data.room, _id);

			// TODO:
			publishOwnStream(true);

			// Any new feed to attach to
			if (data.xhas("publishers")) {
				const auto& publishers = data.publishers;
				DLOG("Got a list of available publishers/feeds:");
				for (const auto& pub : publishers) {
					DLOG("  >> [{}] {} (audio: {}, video: {}}", pub.id, pub.display.c_str(), pub.audio_codec.c_str(), pub.video_codec.c_str());
					// TODO:
					createParticipant(pub.id, pub.display, pub.audio_codec, pub.video_codec);
				}
			}
		}
		else if (event == "destroyed") {
			ELOG("The room has been destroyed!");
		}
		else if (event == "event") {
			// Any new feed to attach to
			if (data.xhas("publishers")) {
				const auto& publishers = data.publishers;
				DLOG("Got a list of available publishers/feeds:");
				for (const auto& pub : publishers) {
					DLOG("  >> [{}] {}, (audio: {}, video: {})", pub.id, pub.display.c_str(), pub.audio_codec.c_str(), pub.video_codec.c_str());
					// TODO:
					createParticipant(pub.id, pub.display, pub.audio_codec, pub.video_codec);
				}
			}
		}
		else if (event == "leaving") {
			//const auto& leaving = data.leaving;
			// TODO: Figure out the participant and detach it
		}
		else if (event == "unpublished") {
			const auto& unpublished = data.unpublished;
			DLOG("Publisher left: {}", unpublished);

			// TODO: |unpublished| can be int or string
			if (unpublished == 0) {
				// That's us
				this->hangup(true);
				return;
			}

			// TODO: Figure out the participant and detach it
			//remoteFeed.detach();
		}
		else if (event == "error") {
			if (data.error_code == 426) {
				DLOG("No such room");
			}
		}

		if (!jsep.type.empty() && !jsep.sdp.empty()) {
			DLOG("Handling SDP as well...");
			// TODO:
			//sfutest.handleRemoteJsep({ jsep: jsep });
			std::shared_ptr<PrepareWebRTCPeerEvent> event = std::make_shared<PrepareWebRTCPeerEvent>();
			auto lambda = [](bool success, const std::string& response) {
				DLOG("response: {}", response.c_str());
			};
			std::shared_ptr<vi::EventCallback> callback = std::make_shared<vi::EventCallback>(lambda);
			JsepConfig jst;
			jst.type = jsep.type;
			jst.sdp = jsep.sdp;
			event->jsep = jst;
			event->callback = callback;

			handleRemoteJsep(event);

			if (!_pluginContext) {
				return;
			}

			const auto& audio = data.audio_codec;
			if (_pluginContext->webrtcContext->myStream && _pluginContext->webrtcContext->myStream->GetAudioTracks().size() > 0 && audio.empty()) {
				WLOG("Our audio stream has been rejected, viewers won't hear us");
			}

			const auto& video = data.video_codec;
			if (_pluginContext->webrtcContext->myStream && _pluginContext->webrtcContext->myStream->GetVideoTracks().size() > 0 && video.empty()) {
				WLOG("Our video stream has been rejected, viewers won't see us");
			}
		}
	}

	void VideoRoom::onCreateLocalStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream)
	{
		notifyObserver4Change<IVideoRoomListener>(*_listeners, [pid = _id, stream](const std::shared_ptr<IVideoRoomListener>& listener) {
			listener->onCreateStream(pid, stream);
		});
	}

	void VideoRoom::onDeleteLocalStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream)
	{
		notifyObserver4Change<IVideoRoomListener>(*_listeners, [pid = _id, stream](const std::shared_ptr<IVideoRoomListener>& listener) {
			listener->onDeleteStream(pid, stream);
		});
	}

	void VideoRoom::onCreateRemoteStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) {}

	void VideoRoom::onDeleteRemoteStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) {}

	void VideoRoom::onData(const std::string& data, const std::string& label) {}

	void VideoRoom::onDataOpen(const std::string& label) {}

	void VideoRoom::onCleanup() 
	{
		_pluginContext->webrtcContext->myStream = nullptr;
	}

	void VideoRoom::onDetached() {}

	void VideoRoom::publishOwnStream(bool audioOn)
	{
		auto wself = weak_from_this();
		std::shared_ptr<PrepareWebRTCEvent> event = std::make_shared<PrepareWebRTCEvent>();
		auto callback = std::make_shared<CreateAnswerOfferCallback>([wself, audioOn](bool success, const std::string& reason, const JsepConfig& jsep) {
			auto self = wself.lock();
			if (!self) {
				return;
			}
			if (success) {
				vr::PublisherConfigureRequest request;
				request.audio = audioOn;
				request.video = true;
				if (auto webrtcService = self->pluginContext()->webrtcService.lock()) {
					std::shared_ptr<SendMessageEvent> event = std::make_shared<vi::SendMessageEvent>();
					auto lambda = [](bool success, const std::string& response) {
						DLOG("publishOwnStream: {}", response.c_str());
					};
					std::shared_ptr<vi::EventCallback> callback = std::make_shared<vi::EventCallback>(lambda);
					event->message = x2struct::X::tojson(request);
					Jsep jp;
					jp.type = jsep.type;
					jp.sdp = jsep.sdp;
					event->jsep = x2struct::X::tojson(jp);
					event->callback = callback;
					self->sendMessage(event);
				}
			}
			else {
				DLOG("WebRTC error: {}", reason.c_str());
			}
		});
		event->answerOfferCallback = callback;
		MediaConfig media;
		media.audioRecv = true;
		media.videoRecv = true;
		media.audioSend = audioOn;
		media.videoSend = true;
		event->media = media;
		event->simulcast = false;
		event->simulcast2 = false;
		createOffer(event);
	}

	void VideoRoom::unpublishOwnStream()
	{
		vr::UnpublishRequest request;
		if (auto webrtcService = pluginContext()->webrtcService.lock()) {
			std::shared_ptr<SendMessageEvent> event = std::make_shared<vi::SendMessageEvent>();
			auto lambda = [](bool success, const std::string& response) {
				DLOG("response: {}", response.c_str());
			};
			std::shared_ptr<vi::EventCallback> callback = std::make_shared<vi::EventCallback>(lambda);
			event->message = x2struct::X::tojson(request);
			event->callback = callback;
			sendMessage(event);
		}
	}

	void VideoRoom::createParticipant(int64_t id, const std::string& displayName, const std::string& audio, const std::string& video)
	{
		//return;
		auto participant = std::make_shared<Participant>(_pluginContext->plugin, 
														 _pluginContext->opaqueId, 
														 id,
														 _privateId,
														 displayName,
														 _pluginContext->webrtcService.lock(),
														 _listeners);
		participant->attach();
		_participantsMap[id] = participant;
		notifyObserver4Change<IVideoRoomListener>(*_listeners, [participant](const std::shared_ptr<IVideoRoomListener>& listener) {
			listener->onCreateParticipant(participant);
		});
	}
}
