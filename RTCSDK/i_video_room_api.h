#pragma once

#include <memory>
#include "video_room_models.h"
#include "signaling_events.h"

namespace vi {

	class IVideoRoomApi {
	public:
		virtual ~IVideoRoomApi() = default;

		virtual void create(std::shared_ptr<vr::CreateRoomRequest> request, std::function<void(std::shared_ptr<vr::RoomCurdResponse>)> callback) = 0;

		virtual void destroy(std::shared_ptr<vr::DestroyRoomRequest> request, std::function<void(std::shared_ptr<vr::RoomCurdResponse>)> callback) = 0;

		virtual void edit(std::shared_ptr<vr::EditRoomRequest> request, std::function<void(std::shared_ptr<vr::RoomCurdResponse>)> callback) = 0;

		virtual void exists(std::shared_ptr<vr::ExistsRequest> request, std::function<void(std::shared_ptr<vr::RoomCurdResponse>)> callback) = 0;

		virtual void join(std::shared_ptr<vr::PublisherJoinRequest> request, std::function<void(std::shared_ptr<JanusResponse>)> callback) = 0;
		
		virtual void join(std::shared_ptr<vr::SubscriberJoinRequest> request, std::function<void(std::shared_ptr<JanusResponse>)> callback) = 0;

		virtual void publisherConfigure(std::shared_ptr<vr::PublisherConfigureRequest> request, std::function<void(std::shared_ptr<JanusResponse>)> callback) = 0;

		virtual void subscriberConfigure(std::shared_ptr<vr::SubscriberConfigureRequest> request, std::function<void(std::shared_ptr<JanusResponse>)> callback) = 0;

		virtual void publish(std::shared_ptr<vr::PublishRequest> request, std::function<void(std::shared_ptr<JanusResponse>)> callback) = 0;

		virtual void unpublish(std::shared_ptr<vr::UnpublishRequest> request, std::function<void(std::shared_ptr<JanusResponse>)> callback) = 0;

		virtual void subscribe(std::shared_ptr<vr::SubscribeRequest> request, std::function<void(std::shared_ptr<JanusResponse>)> callback) = 0;

		virtual void unsubscribe(std::shared_ptr<vr::UnsubscribeRequest> request, std::function<void(std::shared_ptr<JanusResponse>)> callback) = 0;

		virtual void startPeerConnection(std::shared_ptr<vr::StartPeerConnectionRequest> request, std::function<void(std::shared_ptr<JanusResponse>)> callback) = 0;

		virtual void pausePeerConnection(std::shared_ptr<vr::PausePeerConnectionRequest> request, std::function<void(std::shared_ptr<JanusResponse>)> callback) = 0;

		// TODO: testing
		virtual void switchPublisher(std::shared_ptr<vr::SwitchPublisherRequest> request, std::function<void(std::shared_ptr<JanusResponse>)> callback) = 0;

		virtual void leave(std::shared_ptr<vr::LeaveRequest> request, std::function<void(std::shared_ptr<JanusResponse>)> callback) = 0;

		virtual void allowed(std::shared_ptr<vr::AllowedRequest> request, std::function<void(std::shared_ptr<vr::AllowedResponse>)> callback) = 0; 

		virtual void kick(std::shared_ptr<vr::KickRequest> request, std::function<void(std::shared_ptr<vr::KickResponse>)> callback) = 0; 

		virtual void moderate(std::shared_ptr<vr::ModerateRequest> request, std::function<void(std::shared_ptr<vr::ModerateResponse>)> callback) = 0;

		virtual void fetchRoomsList(std::shared_ptr<vr::FetchRoomsListRequest> request, std::function<void(std::shared_ptr<vr::FetchRoomsListResponse>)> callback) = 0; 

		virtual void fetchParticipants(std::shared_ptr<vr::FetchParticipantsRequest> request, std::function<void(std::shared_ptr<vr::FetchParticipantsResponse>)> callback) = 0;

	};

}