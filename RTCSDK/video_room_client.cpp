/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#include "video_room_client.h"
#include "utils/string_utils.h"
#include "logger/logger.h"
#include "participant.h"
#include "Service/rtc_engine.h"
#include "video_room_api.h"
#include "video_room_subscriber.h"
#include "pc/media_stream.h"
#include "pc/media_stream_proxy.h"
#include "pc/media_stream_track_proxy.h"
#include "media_controller.h"
#include "participants_controller.h"

namespace vi {
	VideoRoomClient::VideoRoomClient(std::shared_ptr<SignalingClientInterface> sc, rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> pcf)
		: PluginClient(sc, pcf)
	{
		_pluginContext->plugin = "janus.plugin.videoroom";
		_pluginContext->opaqueId = "videoroom-" + StringUtils::randomString(12);
	}

	VideoRoomClient::~VideoRoomClient()
	{
		DLOG("~VideoRoomClient()");
	}

	void VideoRoomClient::init()
	{
		PluginClient::init();

		_videoRoomApi = std::make_shared<VideoRoomApi>(shared_from_this());

		_mediaController = std::make_shared<MediaController>(std::dynamic_pointer_cast<VideoRoomClient>(PluginClient::shared_from_this()));
		_mediaControllerProxy = MediaControllerProxy::Create(TMgr->thread("plugin-client"), _mediaController);

		_participantsController = std::make_shared<ParticipantsContrller>();
		_participantsControllerProxy = ParticipantsContrllerProxy::Create(TMgr->thread("plugin-client"), _participantsController);

		_subscriber = std::make_shared<VideoRoomSubscriber>(_pluginContext->signalingClient.lock(), _pluginContext->pcf, _pluginContext->plugin, _pluginContext->opaqueId, _mediaController, _videoRoomApi);
		_subscriber->init();
	}

	void VideoRoomClient::destroy()
	{

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

	void VideoRoomClient::attach()
	{
		PluginClient::attach();
	}

	void VideoRoomClient::detach()
	{
		auto event = std::make_shared<DetachEvent>();
		PluginClient::detach(event);
		_subscriber->detach(event);
	}

	void VideoRoomClient::create(std::shared_ptr<vr::CreateRoomRequest> request)
	{
		if (_videoRoomApi) {
			_videoRoomApi->create(request, [this, request](std::shared_ptr<vr::RoomCurdResponse> response) {
				if (response->janus == "success") {
					UniversalObservable<IVideoRoomEventHandler>::notifyObservers([request, response](const auto& observer) {
						if (response->plugindata->data.videoroom.value_or("") == "created") {
							auto result = std::make_shared<CreateRoomResult>();
							result->roomId = response->plugindata->data.room.value_or(0);
							result->description = request->description;
							result->secret = request->secret;
							result->pin = request->pin;
							observer->onCreateRoom(result, 0);
						}
						else {
							observer->onCreateRoom(nullptr, response->plugindata->data.error_code.value_or(0));
						}
					});
				}
				else {
					UniversalObservable<IVideoRoomEventHandler>::notifyObservers([request](const auto& observer) {
						// TODO: replace it with enum, a global error code
						observer->onCreateRoom(nullptr, 1);
					});
				}
			});
		}
	}

	void VideoRoomClient::join(std::shared_ptr<vr::PublisherJoinRequest> request)
	{
		_roomId = request->room.value();

		_subscriber->setRoomId(_roomId);

		if (_videoRoomApi) {
			_videoRoomApi->join(request, [this](std::shared_ptr<JanusResponse> response) {
				if (response->janus == "ack") {
					UniversalObservable<IVideoRoomEventHandler>::notifyObservers([roomId = _roomId](const auto& observer) {
						observer->onJoinRoom(roomId, 0);
					});
				}
				else {
					UniversalObservable<IVideoRoomEventHandler>::notifyObservers([roomId = _roomId](const auto& observer) {
						// TODO: replace it with enum, a global error code
						observer->onJoinRoom(roomId, 1);
					});
				}
			});
		}
	}

	void VideoRoomClient::leave(std::shared_ptr<vr::LeaveRequest> request)
	{
		if (_videoRoomApi) {
			_videoRoomApi->leave(request, [this](std::shared_ptr<JanusResponse> response) {
				if (response->janus == "ack") {
					UniversalObservable<IVideoRoomEventHandler>::notifyObservers([roomId = _roomId](const auto& observer) {
						observer->onLeaveRoom(roomId, 0);
					});
				}
				else {
					UniversalObservable<IVideoRoomEventHandler>::notifyObservers([roomId = _roomId](const auto& observer) {
						// TODO: replace it with enum, a global error code
						observer->onLeaveRoom(roomId, 1);
					});
				}
			});
		}
	}

	std::shared_ptr<ParticipantsContrllerInterface> VideoRoomClient::participantsController()
	{
		return _participantsControllerProxy;
	}

	std::shared_ptr<MediaControllerInterface> VideoRoomClient::mediaContrller()
	{
		return _mediaControllerProxy;
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

	void VideoRoomClient::onHangup() 
	{
		hangup(true);
	}

	void VideoRoomClient::onMediaStatus(const std::string& media, bool on, const std::string& mid)
	{
		DLOG("Janus {} receiving our {}", (on ? "started" : "stopped"), media.c_str());
	}

	void VideoRoomClient::onWebrtcStatus(bool isActive, const std::string& reason)
	{
		DLOG("Janus says our WebRTC PeerConnection is {} now", (isActive ? "up" : "down"));
		if (isActive) {
			if (_mediaController) {
				_mediaController->onWebrtcStatus(isActive, reason);
			}
		}
		// TODO:-----------------------
		//unmuteVideo("");
		startRtcStatsReport();
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
			publishStream(true);

			// Any new feed to attach to
			if (pluginData->data->publishers && !pluginData->data->publishers->empty()) {
				const auto& publishers = pluginData->data->publishers.value();
				DLOG("Got a list of available publishers/feeds:");
        				for (const auto& pub : publishers) {
					DLOG("  >> [{}] {}", pub.id.value(), pub.display.value_or(""));

					auto participant = std::make_shared<Participant>(pub.id.value(), pub);
					createParticipant(participant);
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
					auto participant = std::make_shared<Participant>(pub.id.value(), pub);
					createParticipant(participant);
				}
				_subscriber->subscribeTo(publishers);
			}

			if (pluginData->data->leaving) {
				const auto& leaving = pluginData->data->leaving.value();

				// Figure out the participant and detach it
				removeParticipant(leaving);

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
				removeParticipant(unpublished);

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

			std::shared_ptr<PrepareWebrtcPeerEvent> event = std::make_shared<PrepareWebrtcPeerEvent>();
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

			//const auto& audio = pluginData->data->audio_codec.value_or("");
			//if (_pluginContext->localStream && _pluginContext->localStream->GetAudioTracks().size() > 0 && audio.empty()) {
			//	WLOG("Our audio stream has been rejected, viewers won't hear us");
			//}

			//const auto& video = pluginData->data->video_codec.value_or("");
			//if (_pluginContext->localStream && _pluginContext->localStream->GetVideoTracks().size() > 0 && video.empty()) {
			//	WLOG("Our video stream has been rejected, viewers won't see us");
			//}
		}
	}

	void VideoRoomClient::onTimeout()
	{

	}

	void VideoRoomClient::onError(const std::string& desc)
	{

	}

	void VideoRoomClient::onLocalTrack(rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> track, bool on)
	{
		_mediaController->onLocalTrack(track, _id, on);
	}

	void VideoRoomClient::onCleanup()
	{
		PluginClient::onCleanup();
	}

	void VideoRoomClient::onDetached() {}

	void VideoRoomClient::publishStream(bool audioOn)
	{
		auto event = std::make_shared<PrepareWebrtcEvent>();
		_pluginContext->offerAnswerCallback = std::make_shared<CreateOfferAnswerCallback>([wself = weak_from_this(), audioOn](bool success, const std::string& reason, const JsepConfig& jsep) {
			auto self = wself.lock();
			if (!self) {
				return;
			}
			if (success) {
				vr::PublisherConfigureRequest request;
				auto event = std::make_shared<vi::MessageEvent>();
				auto lambda = [](bool success, const std::string& response) {
					DLOG("publishStream: {}", response.c_str());
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
			else {
				DLOG("WebRTC error: {}", reason.c_str());
			}
		});
		MediaConfig media;
		media.audioRecv = false;
		media.videoRecv = false;
		media.audioSend = audioOn;
		media.videoSend = true;
		event->media = media;
		event->simulcast = true;
		event->simulcast2 = false;
		createOffer(event);
	}

	void VideoRoomClient::unpublishStream()
	{
		vr::UnpublishRequest request;
		if (auto sc = _pluginContext->signalingClient.lock()) {
			auto event = std::make_shared<vi::MessageEvent>();
			auto lambda = [](bool success, const std::string& response) {
				DLOG("response: {}", response.c_str());
			};
			auto callback = std::make_shared<vi::EventCallback>(lambda);
			event->message = request.toJsonStr();
			event->callback = callback;
			sendMessage(event);
		}
	}

	void VideoRoomClient::createParticipant(std::shared_ptr<Participant> participant)
	{
		if (_participantsController) {
			_participantsController->createParticipant(participant);
		}
	}

	void VideoRoomClient::removeParticipant(int64_t id)
	{
		if (_participantsController) {
			_participantsController->removeParticipant(id);
		}
	}

	void VideoRoomClient::onStatsDelivered(const rtc::scoped_refptr<const webrtc::RTCStatsReport>& report)
	{
		DLOG("RTC Stats Report: {}", report->ToJson());
	}
}
