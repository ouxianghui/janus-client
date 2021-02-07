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

	std::shared_ptr<Participant> VideoRoom::getParticipant(int64_t pid)
	{
		//if (pid == this->_id) {
		//	return shared_from_this();
		//}
		//else {
			return _participantsMap.find(pid) == _participantsMap.end() ? nullptr : _participantsMap[pid];
		//}
	}

	void VideoRoom::setRoomId(int64_t roomId)
	{
		_roomId = roomId;
	}

	int64_t VideoRoom::getRoomId() const
	{
		return _roomId;
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
			if (auto webrtcService = _pluginContext->webrtcService.lock()) {
				vr::PublishRequest request;
				request.request = "configure";
				request.bitrate = 256000;

				/* 
				 * After debugging, the SFU does receive the display name set when we join, 
				 * but the subscriber is not displayed. It should be a SFU bug. 
				 * After setting once here, the subscriber of the later join will have the opportunity to display your display name.
				 */
				//request.display = "input your display name here";
				_videoRoomApi->publish(request, [](std::shared_ptr<JanusResponse> response) {
					DLOG("response: {}", response->janus);
				});
			}

			notifyObserver4Change<IVideoRoomListener>(*_listeners, [isActive, reason](const std::shared_ptr<IVideoRoomListener>& listener) {
				listener->onMediaState(isActive, reason);
			});
		}
		unmuteVideo();
		startStatsReport();
	}

	void VideoRoom::onSlowLink(bool uplink, bool lost) {}

	void VideoRoom::onMessage(const std::string& data, const std::string& jsepString)
	{
		DLOG(" ::: Got a message (publisher).");

		vr::VideoRoomEvent vrEvent;
		x2struct::X::loadjson(data, vrEvent, false, true);

		const auto& pluginData = vrEvent.plugindata;

		if (!pluginData.xhas("plugin")) {
			return;
		}

		if (pluginData.plugin != "janus.plugin.videoroom") {
			return;
		}

		if (!pluginData.data.xhas("videoroom")) {
			return;
		}

		const auto& event = pluginData.data.videoroom;

		if (event == "joined") {
			vr::PublisherJoinEvent pjEvent;
			x2struct::X::loadjson(data, pjEvent, false, true);

			const auto& pluginData = pjEvent.plugindata;
			// Publisher/manager created, negotiate WebRTC and attach to existing feeds, if any
			_id = pluginData.data.id;
			_privateId = pluginData.data.private_id;
			DLOG("Successfully joined room {} with ID {}", pluginData.data.room, _id);

			// TODO:
			publishOwnStream(true);

			// Any new feed to attach to
			if (pluginData.data.xhas("publishers")) {
				const auto& publishers = pluginData.data.publishers;
				DLOG("Got a list of available publishers/feeds:");
				for (const auto& pub : publishers) {
					DLOG("  >> [{}] {} (audio: {}, video: {}}", pub.id, pub.display.c_str(), pub.audio_codec.c_str(), pub.video_codec.c_str());

					ParticipantSt info{ pub.id, pub.display, pub.audio_codec, pub.video_codec };
					createParticipant(info);
				}
			}
		}
		else if (event == "destroyed") {
			ELOG("The room has been destroyed!");
		}
		else if (event == "event") {
			// Any new feed to attach to
			if (pluginData.data.xhas("publishers")) {
				const auto& publishers = pluginData.data.publishers;
				DLOG("Got a list of available publishers/feeds:");
				for (const auto& pub : publishers) {
					DLOG("  >> [{}] {}, (audio: {}, video: {})", pub.id, pub.display.c_str(), pub.audio_codec.c_str(), pub.video_codec.c_str());
					ParticipantSt info{ pub.id, pub.display, pub.audio_codec, pub.video_codec };
					createParticipant(info);
				}
			}

			if (pluginData.data.xhas("leaving")) {
				const auto& leaving = pluginData.data.leaving;

				// Figure out the participant and detach it
				for (const auto& pair : _participantsMap) {
					if (pair.first == leaving) {
						removeParticipant(leaving);
						break;
					}
				}
			}
			else if (pluginData.data.xhas("unpublished")) {
				const auto& unpublished = pluginData.data.unpublished;
				DLOG("Publisher left: {}", unpublished);

				// TODO: |unpublished| can be int or string
				if (unpublished == 0) {
					// That's us
					this->hangup(true);
					return;
				}

				// Figure out the participant and detach it
				for (const auto& pair : _participantsMap) {
					if (pair.first == unpublished) {
						removeParticipant(unpublished);
						break;
					}
				}
			}
			else if (pluginData.data.xhas("error")) {
				if (pluginData.data.error_code == 426) {
					DLOG("No such room");
				}
			}
		}

		if (!jsepString.empty()) {
			Jsep jsep;
			x2struct::X::loadjson(jsepString, jsep, false, true);
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

				const auto& audio = pluginData.data.audio_codec;
				if (_pluginContext->webrtcContext->myStream && _pluginContext->webrtcContext->myStream->GetAudioTracks().size() > 0 && audio.empty()) {
					WLOG("Our audio stream has been rejected, viewers won't hear us");
				}

				const auto& video = pluginData.data.video_codec;
				if (_pluginContext->webrtcContext->myStream && _pluginContext->webrtcContext->myStream->GetVideoTracks().size() > 0 && video.empty()) {
					WLOG("Our video stream has been rejected, viewers won't see us");
				}
			}
		}
	}

	void VideoRoom::onCreateLocalStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream)
	{
		notifyObserver4Change<IVideoRoomListener>(*_listeners, [pid = _id, stream](const std::shared_ptr<IVideoRoomListener>& listener) {
			listener->onCreateStream(pid, stream);
		});
	}

	void VideoRoom::onRemoveLocalStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream)
	{
		notifyObserver4Change<IVideoRoomListener>(*_listeners, [pid = _id, stream](const std::shared_ptr<IVideoRoomListener>& listener) {
			listener->onRemoveStream(pid, stream);
		});
	}

	void VideoRoom::onCreateRemoteStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) {}

	void VideoRoom::onRemoveRemoteStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) {}

	void VideoRoom::onData(const std::string& data, const std::string& label) {}

	void VideoRoom::onDataOpen(const std::string& label) {}

	void VideoRoom::onCleanup() 
	{
		_pluginContext->webrtcContext->myStream = nullptr;
	}

	void VideoRoom::onDetached() {}

	void VideoRoom::onStatsReport(const rtc::scoped_refptr<const webrtc::RTCStatsReport>& report) {}

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
		event->simulcast = true;
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

	void VideoRoom::createParticipant(const ParticipantSt& info)
	{
		//return;
		auto participant = std::make_shared<Participant>(_pluginContext->plugin, 
			_pluginContext->opaqueId, 
			info.id,
			_privateId,
			_roomId,
			info.displayName,
			_pluginContext->webrtcService.lock(),
			_listeners);

		participant->attach();
		_participantsMap[info.id] = participant;
		notifyObserver4Change<IVideoRoomListener>(*_listeners, [participant](const std::shared_ptr<IVideoRoomListener>& listener) {
			listener->onCreateParticipant(participant);
		});
	}

	void VideoRoom::removeParticipant(int64_t id)
	{
		if (_participantsMap.find(id) != _participantsMap.end()) {

			notifyObserver4Change<IVideoRoomListener>(*_listeners, [participant = _participantsMap[id]](const std::shared_ptr<IVideoRoomListener>& listener) {
				listener->onRemoveParticipant(participant);
			});
		}
	}
}
