#pragma once

#include <memory>
#include <string>
#include <vector>
#include "video_room_models.h"

namespace vi {

	class Participant;
	class IVideoRoomEventHandler;
	class ParticipantsContrllerInterface;
	class MediaControllerInterface;

	class VideoRoomClientInterface {
	public:
		virtual ~VideoRoomClientInterface() = default;

		virtual void init() = 0;

		virtual void destroy() = 0;

		virtual void registerEventHandler(std::shared_ptr<IVideoRoomEventHandler> handler) = 0;

		virtual void unregisterEventHandler(std::shared_ptr<IVideoRoomEventHandler> handler) = 0;

		virtual void create(std::shared_ptr<vr::CreateRoomRequest> request) = 0;

		virtual void join(std::shared_ptr<vr::PublisherJoinRequest> request) = 0;

		virtual void leave(std::shared_ptr<vr::LeaveRequest> request) = 0;

		virtual std::shared_ptr<ParticipantsContrllerInterface> participantsController() = 0;

		virtual std::shared_ptr<MediaControllerInterface> mediaContrller() = 0;

	};

}