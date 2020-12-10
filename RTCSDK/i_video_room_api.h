#pragma once

#include <memory>
#include "message_models.h"
#include "webrtc_service_events.h"

namespace vi {

	class IVideoRoomApi {
	public:
		virtual ~IVideoRoomApi() = default;

		virtual void create(const vr::CreateRoomRequest& request, std::function<void(std::shared_ptr<vr::RoomCurdResponse>)> callback) = 0;

		virtual void destroy(const vr::DestroyRoomRequest& request, std::function<void(std::shared_ptr<vr::RoomCurdResponse>)> callback) = 0;

		virtual void edit(const vr::EditRoomRequest& request, std::function<void(std::shared_ptr<vr::RoomCurdResponse>)> callback) = 0;

		virtual void exists(const vr::ExistsRequest& request, std::function<void(std::shared_ptr<vr::RoomCurdResponse>)> callback) = 0;

		virtual void join(const vr::PublisherJoinRequest& request, std::function<void(std::shared_ptr<JanusResponse>)> callback) = 0;
		
		virtual void join(const vr::SubscriberJoinRequest& request, std::function<void(std::shared_ptr<JanusResponse>)> callback) = 0;

		//virtual void publish(const vr::PublishRequest& request, std::shared_ptr<EventCallback> callback) = 0;

		//virtual void unpublish(const vr::UnpublishRequest& request, std::shared_ptr<EventCallback> callback) = 0;

		//virtual void start(const vr::StartRequest& request, std::shared_ptr<EventCallback> callback) = 0;

		//virtual void pause(const vr::PauseRequest& request, std::shared_ptr<EventCallback> callback) = 0;

		//virtual void leave(const vr::LeaveRequest& request, std::shared_ptr<EventCallback> callback) = 0;

		//virtual void allowed(const vr::AllowedRequest& request, std::shared_ptr<EventCallback> callback) = 0;

		//virtual void kick(const vr::KickRequest& request, std::shared_ptr<EventCallback> callback) = 0;

		//virtual void fetchRoomsList(const vr::FetchRoomsListRequest& request, std::shared_ptr<EventCallback> callback) = 0;

		//virtual void fetchParticipants(const vr::FetchParticipantsRequest& request, std::shared_ptr<EventCallback> callback) = 0;

		//virtual void subscriberConfigure(const vr::SubscriberConfigureRequest& request, std::shared_ptr<EventCallback> callback) = 0;

		//virtual void publisherConfigure(const vr::PublisherConfigureRequest& request, std::shared_ptr<EventCallback> callback) = 0;

		//virtual void switchMedia(const vr::SwitchRequest& request, std::shared_ptr<EventCallback> callback) = 0;
	};

}