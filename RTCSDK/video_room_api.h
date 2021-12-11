/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-12-9
 **/

#pragma once

#include "i_video_room_api.h"
#include <memory>

namespace vi {

	class PluginClient;

	class VideoRoomApi : public IVideoRoomApi
	{
	public:
		VideoRoomApi(std::shared_ptr<PluginClient> pluginClient);

		~VideoRoomApi();

		void create(std::shared_ptr<vr::CreateRoomRequest> request, std::function<void(std::shared_ptr<vr::RoomCurdResponse>)> callback) override;

		void destroy(std::shared_ptr<vr::DestroyRoomRequest> request, std::function<void(std::shared_ptr<vr::RoomCurdResponse>)> callback) override;

		void edit(std::shared_ptr<vr::EditRoomRequest> request, std::function<void(std::shared_ptr<vr::RoomCurdResponse>)> callback) override;

		void exists(std::shared_ptr<vr::ExistsRequest> request, std::function<void(std::shared_ptr<vr::RoomCurdResponse>)> callback) override;
		
		void join(std::shared_ptr<vr::PublisherJoinRequest> request, std::function<void(std::shared_ptr<JanusResponse>)> callback) override;

		void join(std::shared_ptr<vr::SubscriberJoinRequest> request, std::function<void(std::shared_ptr<JanusResponse>)> callback) override;

		void publisherConfigure(std::shared_ptr<vr::PublisherConfigureRequest> request, std::function<void(std::shared_ptr<JanusResponse>)> callback) override;

		void subscriberConfigure(std::shared_ptr<vr::SubscriberConfigureRequest> request, std::function<void(std::shared_ptr<JanusResponse>)> callback) override;

		void publish(std::shared_ptr<vr::PublishRequest> request, std::function<void(std::shared_ptr<JanusResponse>)> callback) override;

		void unpublish(std::shared_ptr<vr::UnpublishRequest> request, std::function<void(std::shared_ptr<JanusResponse>)> callback) override;

		void startPeerConnection(std::shared_ptr<vr::StartPeerConnectionRequest> request, std::function<void(std::shared_ptr<JanusResponse>)> callback) override;

		void pausePeerConnection(std::shared_ptr<vr::PausePeerConnectionRequest> request, std::function<void(std::shared_ptr<JanusResponse>)> callback) override;

		void switchPublisher(std::shared_ptr<vr::SwitchPublisherRequest> request, std::function<void(std::shared_ptr<JanusResponse>)> callback) override;

		void leave(std::shared_ptr<vr::LeaveRequest> request, std::function<void(std::shared_ptr<JanusResponse>)> callback) override;

		void allowed(std::shared_ptr<vr::AllowedRequest> request, std::function<void(std::shared_ptr<vr::AllowedResponse>)> callback) override;

		void kick(std::shared_ptr<vr::KickRequest> request, std::function<void(std::shared_ptr<vr::KickResponse>)> callback) override;

		void fetchRoomsList(std::shared_ptr<vr::FetchRoomsListRequest> request, std::function<void(std::shared_ptr<vr::FetchRoomsListResponse>)> callback) override;

		void fetchParticipants(std::shared_ptr<vr::FetchParticipantsRequest> request, std::function<void(std::shared_ptr<vr::FetchParticipantsResponse>)> callback) override;

	private:
		void curd(const std::string& request, std::function<void(std::shared_ptr<vr::RoomCurdResponse>)> callback);

		void action(const std::string& request, std::function<void(std::shared_ptr<JanusResponse>)> callback);

	private:
		std::weak_ptr<PluginClient> _pluginClient;
	};

}