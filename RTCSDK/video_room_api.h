/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-12-9
 **/

#pragma once

#include "i_video_room_api.h"
#include <memory>
#include "i_plugin_client.h"

namespace vi {

	class VideoRoomApi : public IVideoRoomApi
	{
	public:
		VideoRoomApi(std::shared_ptr<IPluginClient> pluginClient);
		~VideoRoomApi();

		void create(const vr::CreateRoomRequest& request, std::function<void(std::shared_ptr<vr::RoomCurdResponse>)> callback) override;

		void destroy(const vr::DestroyRoomRequest& request, std::function<void(std::shared_ptr<vr::RoomCurdResponse>)> callback) override;

		void edit(const vr::EditRoomRequest& request, std::function<void(std::shared_ptr<vr::RoomCurdResponse>)> callback) override;

		void exists(const vr::ExistsRequest& request, std::function<void(std::shared_ptr<vr::RoomCurdResponse>)> callback) override;
		
		void join(const vr::PublisherJoinRequest& request, std::function<void(std::shared_ptr<JanusResponse>)> callback) override;

		void join(const vr::SubscriberJoinRequest& request, std::function<void(std::shared_ptr<JanusResponse>)> callback) override;

		void publisherConfigure(const vr::PublisherConfigureRequest& request, std::function<void(std::shared_ptr<JanusResponse>)> callback) override;

		void subscriberConfigure(const vr::SubscriberConfigureRequest& request, std::function<void(std::shared_ptr<JanusResponse>)> callback) override;

		void publish(const vr::PublishRequest& request, std::function<void(std::shared_ptr<JanusResponse>)> callback) override;

		void unpublish(const vr::UnpublishRequest& request, std::function<void(std::shared_ptr<JanusResponse>)> callback) override;

		void startPeerConnection(const vr::StartPeerConnectionRequest& request, std::function<void(std::shared_ptr<JanusResponse>)> callback) override;

		void pausePeerConnection(const vr::PausePeerConnectionRequest& request, std::function<void(std::shared_ptr<JanusResponse>)> callback) override;

		void switchPublisher(const vr::SwitchPublisherRequest& request, std::function<void(std::shared_ptr<JanusResponse>)> callback) override;

		void leave(const vr::LeaveRequest& request, std::function<void(std::shared_ptr<JanusResponse>)> callback) override;

		void allowed(const vr::AllowedRequest& request, std::function<void(std::shared_ptr<vr::AllowedResponse>)> callback) override;

		void kick(const vr::KickRequest& request, std::function<void(std::shared_ptr<vr::KickResponse>)> callback) override;

		void fetchRoomsList(const vr::FetchRoomsListRequest& request, std::function<void(std::shared_ptr<vr::FetchRoomsListResponse>)> callback) override;

		void fetchParticipants(const vr::FetchParticipantsRequest& request, std::function<void(std::shared_ptr<vr::FetchParticipantsResponse>)> callback) override;

	private:
		void curd(const std::string& request, std::function<void(std::shared_ptr<vr::RoomCurdResponse>)> callback);

		void action(const std::string& request, std::function<void(std::shared_ptr<JanusResponse>)> callback);

	private:
		std::weak_ptr<IPluginClient> _pluginClient;
	};

}