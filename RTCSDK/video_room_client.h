/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#pragma once

#include "plugin_client.h"
#include "utils/universal_observable.hpp"
#include "video_room_client_interface.h"
#include "i_video_room_event_handler.h"

namespace webrtc {
	class MediaStreamInterface;
}

namespace vi {
	class IVideoRoomApi;
	class Participant;
	class VideoRoomPublisher;
	class VideoRoomSubscriber;
	class ParticipantsContrller;
	class ParticipantsContrllerInterface;
	class MediaController;
	class MediaControllerInterface;

	class VideoRoomClient : public PluginClient, public VideoRoomClientInterface, public UniversalObservable<IVideoRoomEventHandler>
	{
	public:
		VideoRoomClient(std::shared_ptr<SignalingServiceInterface> ss);

		~VideoRoomClient();

		//void init();

		//void registerEventHandler(std::shared_ptr<IVideoRoomEventHandler> handler);

		//void unregisterEventHandler(std::shared_ptr<IVideoRoomEventHandler> handler);

		//std::shared_ptr<Participant> getParticipant(int64_t pid);

		//std::shared_ptr<IVideoRoomApi> getVideoRoomApi();

		//uint64_t getId() { return PluginClient::getId(); }

		//void setRoomId(int64_t roomId);

		//int64_t getRoomId() const;


		void init() override;

		void destroy() override;

		void registerEventHandler(std::shared_ptr<IVideoRoomEventHandler> handler) override;

		void unregisterEventHandler(std::shared_ptr<IVideoRoomEventHandler> handler) override;

		void create(std::shared_ptr<vr::CreateRoomRequest> request) override;

		void join(std::shared_ptr<vr::PublisherJoinRequest> request) override;

		void leave(std::shared_ptr<vr::LeaveRequest> request) override;

		std::shared_ptr<ParticipantsContrllerInterface> participantsController() override;

		std::shared_ptr<MediaControllerInterface> mediaContrller() override;

	protected:

		// signaling events

		void onAttached(bool success) override;

		void onMediaStatus(const std::string& media, bool on, const std::string& mid) override;

		void onWebrtcStatus(bool isActive, const std::string& desc) override;

		void onSlowLink(bool uplink, bool lost, const std::string& mid) override;

		void onMessage(const std::string& data, const std::string& jsep) override;

		void onTimeout()override;

		void onError(const std::string& desc) override;

		void onHangup() override;

		void onCleanup() override;

		void onDetached() override;

	protected:

		// webrtc events

		virtual void onLocalTrack(rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> track, bool on);

	protected:
		void publishStream(bool audioOn);

		void unpublishStream();

		void createParticipant(std::shared_ptr<Participant> participant);

		void removeParticipant(int64_t id);

	private:
		int64_t _roomId;

		std::shared_ptr<IVideoRoomApi> _videoRoomApi;

		std::shared_ptr<VideoRoomSubscriber> _subscriber;

		std::shared_ptr<MediaController> _mediaController;

		std::shared_ptr<MediaControllerInterface> _mediaControllerProxy;

		std::shared_ptr<ParticipantsContrller> _participantsController;

		std::shared_ptr<ParticipantsContrllerInterface> _participantsControllerProxy;
	};
}
