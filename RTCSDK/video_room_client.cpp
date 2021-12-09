/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#include "video_room_client.h"
#include "string_utils.h"
#include "logger/logger.h"
#include "participant.h"
#include "Service/app_instance.h"
#include "video_room_api.h"
#include "video_room_subscriber.h"
#include "pc/media_stream.h"
#include "pc/media_stream_proxy.h"
#include "pc/media_stream_track_proxy.h"

namespace vi {
	VideoRoomClient::VideoRoomClient(std::shared_ptr<WebRTCServiceInterface> wrs)
		: PluginClient(wrs)
	{
		_pluginContext->plugin = "janus.plugin.videoroom";
		_pluginContext->opaqueId = "videoroom-" + StringUtils::randomString(12);
	}

	VideoRoomClient::~VideoRoomClient()
	{
		DLOG("~VideoRoom()");
		if (_pluginContext->webrtcContext->pc) {
			_pluginContext->webrtcContext->pc->Close();
		}
	}

	void VideoRoomClient::init()
	{
		_videoRoomApi = std::make_shared<VideoRoomApi>(shared_from_this());

		_subscriber = std::make_shared<VideoRoomSubscriber>(_pluginContext->webrtcService.lock(), _pluginContext->plugin, _pluginContext->opaqueId);
		_subscriber->setRoomApi(_videoRoomApi);
	}

	void VideoRoomClient::registerEventHandler(std::shared_ptr<IVideoRoomEventHandler> handler)
	{
		UniversalObservable<IVideoRoomEventHandler>::addWeakObserver(handler, std::string("main"));

		_subscriber->registerEventHandler(handler);
	}

	void VideoRoomClient::unregisterEventHandler(std::shared_ptr<IVideoRoomEventHandler> handler)
	{
		UniversalObservable<IVideoRoomEventHandler>::removeObserver(handler);

		_subscriber->unregisterEventHandler(handler);
	}

	std::shared_ptr<Participant> VideoRoomClient::getParticipant(int64_t pid)
	{
		return _participantsMap.find(pid) == _participantsMap.end() ? nullptr : _participantsMap[pid];
	}

	void VideoRoomClient::setRoomId(int64_t roomId)
	{
		_roomId = roomId;
		_subscriber->setRoomId(_roomId);
	}

	int64_t VideoRoomClient::getRoomId() const
	{
		return _roomId;
	}

	std::shared_ptr<IVideoRoomApi> VideoRoomClient::getVideoRoomApi()
	{
		return _videoRoomApi;
	}

	void VideoRoomClient::onAttached(bool success)
	{
		if (success) {
			DLOG("Plugin attached! ({}, id = {})", _pluginContext->plugin.c_str(), _id);
			DLOG("  -- This is a publisher/manager");
		}
		else {
			ELOG("  -- Error attaching plugin...");
		}
	}

	void VideoRoomClient::onHangup() {}

	void VideoRoomClient::onIceState(webrtc::PeerConnectionInterface::IceConnectionState iceState) {}

	void VideoRoomClient::onMediaState(const std::string& media, bool on, const std::string& mid)
	{
		DLOG("Janus {} receiving our {}", (on ? "started" : "stopped"), media.c_str());
	}

	void VideoRoomClient::onWebrtcState(bool isActive, const std::string& reason)
	{
		DLOG("Janus says our WebRTC PeerConnection is {} now", (isActive ? "up" : "down"));
		if (isActive) {
			if (auto webrtcService = _pluginContext->webrtcService.lock()) {
				auto request = std::make_shared<vr::PublishRequest>();
				request->request = "configure";
				request->bitrate = 256000;

				/*
				 * After debugging, the SFU does receive the display name set when we join,
				 * but the subscriber is not displayed. It should be a SFU bug.
				 * After setting once here, the subscriber of the later join will have the opportunity to display your display name.
				 */
				 //request.display = "input your display name here";
				_videoRoomApi->publish(request, [](std::shared_ptr<JanusResponse> response) {
					DLOG("response: {}", response->janus.value_or(""));
				});
			}

			UniversalObservable<IVideoRoomEventHandler>::notifyObservers([isActive, reason](const auto& observer) {
				observer->onMediaState(isActive, reason);
			});
		}
		unmuteVideo("");
		startStatsReport();
	}

	void VideoRoomClient::onSlowLink(bool uplink, bool lost, const std::string& mid) {}

	void VideoRoomClient::onMessage(const std::string& data, const std::string& jsepString)
	{
		DLOG(" ::: Got a message (publisher).");

		std::string err;
		std::shared_ptr<vr::VideoRoomEvent> vrEvent = fromJsonString<vr::VideoRoomEvent>(data, err);
		if (!err.empty()) {
			DLOG("parse JanusResponse failed");
			return;
		}

		const auto& pluginData = vrEvent->plugindata;

		if (!pluginData->plugin) {
			return;
		}

		if (pluginData->plugin.value_or("") != "janus.plugin.videoroom") {
			return;
		}

		if (!pluginData->data->videoroom) {
			return;
		}

		const auto& event = pluginData->data->videoroom;

		if (event.value_or("") == "joined") {
			std::string err;
			std::shared_ptr<vr::PublisherJoinEvent> pjEvent = fromJsonString<vr::PublisherJoinEvent>(data, err);
			if (!err.empty()) {
				DLOG("parse JanusResponse failed");
				return;
			}

			const auto& pluginData = pjEvent->plugindata;
			// Publisher/manager created, negotiate WebRTC and attach to existing feeds, if any
			_id = pluginData->data->id.value();
			_privateId = pluginData->data->private_id.value();
			_subscriber->setPrivateId(_privateId);
			DLOG("Successfully joined room {} with ID {}", pluginData->data->room.value_or(0), _id);

			// TODO:
			publishOwnStream(true);

			// Any new feed to attach to
			if (pluginData->data->publishers && !pluginData->data->publishers->empty()) {
				const auto& publishers = pluginData->data->publishers.value();
				DLOG("Got a list of available publishers/feeds:");
        				for (const auto& pub : publishers) {
					DLOG("  >> [{}] {}", pub.id.value(), pub.display.value_or(""));

					ParticipantSt info{ pub.id.value(), pub.display.value_or("")};
					createParticipant(info);
				}
				_subscriber->subscribeTo(publishers);
			}
		}
		else if (event.value_or("") == "destroyed") {
			ELOG("The room has been destroyed!");
		}
		else if (event.value_or("") == "event") {
			// Any new feed to attach to
			if (pluginData->data->publishers && !pluginData->data->publishers->empty()) {
				const auto& publishers = pluginData->data->publishers.value();
				DLOG("Got a list of available publishers/feeds:");
				for (const auto& pub : publishers) {
					DLOG("  >> [{}] {})", pub.id.value(), pub.display.value_or(""));
					ParticipantSt info{ pub.id.value(), pub.display.value_or("") };
					createParticipant(info);
				}
				_subscriber->subscribeTo(publishers);
			}

			if (pluginData->data->leaving) {
				const auto& leaving = pluginData->data->leaving.value();

				// Figure out the participant and detach it
				for (const auto& pair : _participantsMap) {
					if (pair.first == leaving) {
						removeParticipant(leaving);
						break;
					}
				}
				//_subscriber->unsubscribeFrom(leaving);
			}
			else if (pluginData->data->unpublished) {
				const auto& unpublished = pluginData->data->unpublished.value();
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
				//_subscriber->unsubscribeFrom(unpublished);
			}
			else if (pluginData->data->error) {
				if (pluginData->data->error_code.value_or(0) == 426) {
					DLOG("No such room");
				}
			}
		}

		if (jsepString.empty()) {
			return;
		}
		err.clear();
		std::shared_ptr<Jsep> jsep = fromJsonString<Jsep>(jsepString, err);
		if (!err.empty()) {
			DLOG("parse JanusResponse failed");
			return;
		}

		if (jsep->type && jsep->sdp && !jsep->type.value().empty() && !jsep->sdp.value().empty()) {
			DLOG("Handling SDP as well...");
			// TODO:
			//sfutest.handleRemoteJsep({ jsep: jsep });
			std::shared_ptr<PrepareWebRTCPeerEvent> event = std::make_shared<PrepareWebRTCPeerEvent>();
			auto lambda = [](bool success, const std::string& response) {
				DLOG("response: {}", response.c_str());
			};
			std::shared_ptr<vi::EventCallback> callback = std::make_shared<vi::EventCallback>(lambda);
			JsepConfig jst;
			jst.type = jsep->type.value_or("");
			jst.sdp = jsep->sdp.value_or("");
			event->jsep = jst;
			event->callback = callback;

			handleRemoteJsep(event);

			if (!_pluginContext) {
				return;
			}

			const auto& audio = pluginData->data->audio_codec.value_or("");
			if (_pluginContext->webrtcContext->myStream && _pluginContext->webrtcContext->myStream->GetAudioTracks().size() > 0 && audio.empty()) {
				WLOG("Our audio stream has been rejected, viewers won't hear us");
			}

			const auto& video = pluginData->data->video_codec.value_or("");
			if (_pluginContext->webrtcContext->myStream && _pluginContext->webrtcContext->myStream->GetVideoTracks().size() > 0 && video.empty()) {
				WLOG("Our video stream has been rejected, viewers won't see us");
			}
		}
	}

	void VideoRoomClient::onLocalTrack(rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> track, bool on)
	{
		if (on) {
			UniversalObservable<IVideoRoomEventHandler>::notifyObservers([wself = weak_from_this(), pid = _id, track](const auto& observer) {
				auto self = wself.lock();
				if (!self) {
					return;
				}
				if (!track) {
					return;
				}

				auto vrc = std::dynamic_pointer_cast<VideoRoomClient>(self);

				if (track->kind() == webrtc::MediaStreamTrackInterface::kVideoKind) {
					vrc->_localStreams[track->id()] = webrtc::MediaStream::Create(track->id());
					auto vt = dynamic_cast<webrtc::VideoTrackInterface*>(track.get());
					vrc->_localStreams[track->id()]->AddTrack(vt);
					auto t = vrc->_localStreams[track->id()]->GetVideoTracks()[0];
					//auto vt = dynamic_cast<webrtc::VideoTrackInterface*>(track.get());
					observer->onCreateVideoTrack(pid, vt);
				}
			}); 

		}
		else {
			UniversalObservable<IVideoRoomEventHandler>::notifyObservers([wself = weak_from_this(), pid = _id, track](const auto& observer) {
				auto self = wself.lock();
				if (!self) {
					return;
				}
				if (!track) {
					return;
				}

				auto vrc = std::dynamic_pointer_cast<VideoRoomClient>(self);
				if (track->kind() == webrtc::MediaStreamTrackInterface::kVideoKind) {
					if (vrc->_localStreams.find(track->id()) != vrc->_localStreams.end()) {
						auto vt = vrc->_localStreams[track->id()]->GetVideoTracks()[0];
						//auto vt = dynamic_cast<webrtc::VideoTrackInterface*>(track.get());
						observer->onRemoveVideoTrack(pid, vt);

						vrc->_localStreams[track->id()]->RemoveTrack(vt.get());
						auto it = vrc->_localStreams.find(track->id());
						vrc->_localStreams.erase(it);
					}
				}
			});
		}
	}

	void VideoRoomClient::onRemoteTrack(rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> track, const std::string& mid, bool on) {}

	void VideoRoomClient::onData(const std::string& data, const std::string& label) {}

	void VideoRoomClient::onDataOpen(const std::string& label) {}

	void VideoRoomClient::onCleanup()
	{
		_pluginContext->webrtcContext->myStream = nullptr;
	}

	void VideoRoomClient::onDetached() {}

	void VideoRoomClient::onStatsReport(const rtc::scoped_refptr<const webrtc::RTCStatsReport>& report) {}

	void VideoRoomClient::publishOwnStream(bool audioOn)
	{
		auto wself = weak_from_this();
		auto event = std::make_shared<PrepareWebRTCEvent>();
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
					auto event = std::make_shared<vi::SendMessageEvent>();
					auto lambda = [](bool success, const std::string& response) {
						DLOG("publishOwnStream: {}", response.c_str());
					};
					auto callback = std::make_shared<vi::EventCallback>(lambda);
					event->message = request.toJsonStr();
					Jsep jp; 
					jp.type = jsep.type;
					jp.sdp = jsep.sdp;
					event->jsep = jp.toJsonStr();
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

	void VideoRoomClient::unpublishOwnStream()
	{
		vr::UnpublishRequest request;
		if (auto webrtcService = pluginContext()->webrtcService.lock()) {
			auto event = std::make_shared<vi::SendMessageEvent>();
			auto lambda = [](bool success, const std::string& response) {
				DLOG("response: {}", response.c_str());
			};
			auto callback = std::make_shared<vi::EventCallback>(lambda);
			event->message = request.toJsonStr();
			event->callback = callback;
			sendMessage(event);
		}
	}

	void VideoRoomClient::createParticipant(const ParticipantSt& info)
	{
		auto participant = std::make_shared<Participant>(info.id, info.displayName);

		_participantsMap[info.id] = participant;
		UniversalObservable<IVideoRoomEventHandler>::notifyObservers([wself = weak_from_this(), participant](const auto& observer) {
			observer->onCreateParticipant(participant);
		});
	}

	void VideoRoomClient::removeParticipant(int64_t id)
	{
		if (_participantsMap.find(id) != _participantsMap.end()) {
			UniversalObservable<IVideoRoomEventHandler>::notifyObservers([wself = weak_from_this(), participant = _participantsMap[id]](const auto& observer) {
				observer->onRemoveParticipant(participant);
			});
		}
	}
}
