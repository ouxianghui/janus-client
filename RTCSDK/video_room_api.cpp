#include "video_room_api.h"
#include "webrtc_service_events.h"
#include "logger/logger.h"
#include "x2struct.hpp"

namespace vi {

	VideoRoomApi::VideoRoomApi(std::shared_ptr<IPluginClient> pluginClient)
		: _pluginClient(pluginClient)
	{

	}

	VideoRoomApi::~VideoRoomApi()
	{

	}

	void VideoRoomApi::create(const vr::CreateRoomRequest& request, std::function<void(std::shared_ptr<vr::RoomActionResponse>)> callback)
	{
		auto pluginClient = _pluginClient.lock();
		if (!pluginClient) {
			DLOG("invalid plugin client");
			return;
		}
		std::shared_ptr<SendMessageEvent> event = std::make_shared<vi::SendMessageEvent>();
		auto lambda = [callback](bool success, const std::string& response) {
			DLOG("response: {}", response.c_str());
			if (response.empty()) {
				return;
			}
			std::shared_ptr<vr::RoomActionResponse> rar = std::make_shared<vr::RoomActionResponse>();
			x2struct::X::loadjson(response, *rar, false, true);
			if (callback) {
				callback(rar);
			}
		};
		std::shared_ptr<vi::EventCallback> cb = std::make_shared<vi::EventCallback>(lambda);
		event->message = x2struct::X::tojson(request);
		event->callback = cb;
		pluginClient->sendMessage(event);
	}

	void VideoRoomApi::destroy(const vr::DestroyRoomRequest& request, std::function<void(std::shared_ptr<vr::RoomActionResponse>)> callback)
	{
		auto pluginClient = _pluginClient.lock();
		if (!pluginClient) {
			DLOG("invalid plugin client");
			return;
		}
		std::shared_ptr<SendMessageEvent> event = std::make_shared<vi::SendMessageEvent>();
		auto lambda = [callback](bool success, const std::string& response) {
			DLOG("response: {}", response.c_str());
			if (response.empty()) {
				return;
			}
			std::shared_ptr<vr::RoomActionResponse> rar = std::make_shared<vr::RoomActionResponse>();
			x2struct::X::loadjson(response, *rar, false, true);
			if (callback) {
				callback(rar);
			}
		};
		std::shared_ptr<vi::EventCallback> cb = std::make_shared<vi::EventCallback>(lambda);
		event->message = x2struct::X::tojson(request);
		event->callback = cb;
		pluginClient->sendMessage(event);
	}

	void VideoRoomApi::edit(const vr::EditRoomRequest& request, std::function<void(std::shared_ptr<vr::RoomActionResponse>)> callback)
	{
		auto pluginClient = _pluginClient.lock();
		if (!pluginClient) {
			DLOG("invalid plugin client");
			return;
		}
		std::shared_ptr<SendMessageEvent> event = std::make_shared<vi::SendMessageEvent>();
		auto lambda = [callback](bool success, const std::string& response) {
			DLOG("response: {}", response.c_str());
			if (response.empty()) {
				return;
			}
			std::shared_ptr<vr::RoomActionResponse> rar = std::make_shared<vr::RoomActionResponse>();
			x2struct::X::loadjson(response, *rar, false, true);
			if (callback) {
				callback(rar);
			}
		};
		std::shared_ptr<vi::EventCallback> cb = std::make_shared<vi::EventCallback>(lambda);
		event->message = x2struct::X::tojson(request);
		event->callback = cb;
		pluginClient->sendMessage(event);
	}
}