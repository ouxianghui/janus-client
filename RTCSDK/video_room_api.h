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

		void create(const vr::CreateRoomRequest& request, std::function<void(std::shared_ptr<vr::RoomActionResponse>)> callback) override;

		void destroy(const vr::DestroyRoomRequest& request, std::function<void(std::shared_ptr<vr::RoomActionResponse>)> callback) override;

		void edit(const vr::EditRoomRequest& request, std::function<void(std::shared_ptr<vr::RoomActionResponse>)> callback) override;

	private:
		std::weak_ptr<IPluginClient> _pluginClient;
	};

}