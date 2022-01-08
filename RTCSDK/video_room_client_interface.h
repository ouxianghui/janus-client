#pragma once

#include <memory>
#include <string>
#include <vector>
#include "video_room_models.h"
#include "weak_proxy.h"

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

		virtual void attach() = 0;

		virtual void detach() = 0;

		virtual void create(std::shared_ptr<vr::CreateRoomRequest> request) = 0;

		virtual void join(std::shared_ptr<vr::PublisherJoinRequest> request) = 0;

		virtual void leave(std::shared_ptr<vr::LeaveRequest> request) = 0;

		virtual std::shared_ptr<ParticipantsContrllerInterface> participantsController() = 0;

		virtual std::shared_ptr<MediaControllerInterface> mediaContrller() = 0;
	};

	BEGIN_WEAK_PROXY_MAP(VideoRoomClient)
		WEAK_PROXY_THREAD_DESTRUCTOR()
		WEAK_PROXY_METHOD0(void, init)
		WEAK_PROXY_METHOD0(void, destroy)
		WEAK_PROXY_METHOD1(void, registerEventHandler, std::shared_ptr<IVideoRoomEventHandler>)
		WEAK_PROXY_METHOD1(void, unregisterEventHandler, std::shared_ptr<IVideoRoomEventHandler>)
		WEAK_PROXY_METHOD0(void, attach)
		WEAK_PROXY_METHOD0(void, detach)
		WEAK_PROXY_METHOD1(void, create, std::shared_ptr<vr::CreateRoomRequest>)
		WEAK_PROXY_METHOD1(void, join, std::shared_ptr<vr::PublisherJoinRequest>)
		WEAK_PROXY_METHOD1(void, leave, std::shared_ptr<vr::LeaveRequest>)
		WEAK_PROXY_METHOD0(std::shared_ptr<ParticipantsContrllerInterface>, participantsController)
		WEAK_PROXY_METHOD0(std::shared_ptr<MediaControllerInterface>, mediaContrller)
	END_WEAK_PROXY_MAP()
}