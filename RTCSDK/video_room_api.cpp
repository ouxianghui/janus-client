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

	void VideoRoomApi::curd(const std::string& request, std::function<void(std::shared_ptr<vr::RoomCurdResponse>)> callback)
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
			std::shared_ptr<vr::RoomCurdResponse> rar = std::make_shared<vr::RoomCurdResponse>();
			x2struct::X::loadjson(response, *rar, false, true);
			if (callback) {
				callback(rar);
			}
		};
		std::shared_ptr<vi::EventCallback> cb = std::make_shared<vi::EventCallback>(lambda);
		event->message = request;
		event->callback = cb;
		pluginClient->sendMessage(event);
	}

	void VideoRoomApi::create(const vr::CreateRoomRequest& request, std::function<void(std::shared_ptr<vr::RoomCurdResponse>)> callback)
	{
		std::string json = x2struct::X::tojson(request);
		if (json.empty()) {
			DLOG("empty json string");
			return;
		}
		curd(json, callback);
	}

	void VideoRoomApi::destroy(const vr::DestroyRoomRequest& request, std::function<void(std::shared_ptr<vr::RoomCurdResponse>)> callback)
	{
		std::string json = x2struct::X::tojson(request);
		if (json.empty()) {
			DLOG("empty json string");
			return;
		}
		curd(json, callback);
	}

	void VideoRoomApi::edit(const vr::EditRoomRequest& request, std::function<void(std::shared_ptr<vr::RoomCurdResponse>)> callback)
	{
		std::string json = x2struct::X::tojson(request);
		if (json.empty()) {
			DLOG("empty json string");
			return;
		}
		curd(json, callback);
	}

	void VideoRoomApi::exists(const vr::ExistsRequest& request, std::function<void(std::shared_ptr<vr::RoomCurdResponse>)> callback)
	{
		std::string json = x2struct::X::tojson(request);
		if (json.empty()) {
			DLOG("empty json string");
			return;
		}
		curd(json, callback);
	}

	void VideoRoomApi::join(const vr::PublisherJoinRequest& request, std::function<void(std::shared_ptr<JanusResponse>)> callback)
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
			std::shared_ptr<JanusResponse> rar = std::make_shared<JanusResponse>();
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

	void VideoRoomApi::join(const vr::SubscriberJoinRequest& request, std::function<void(std::shared_ptr<JanusResponse>)> callback)
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
			std::shared_ptr<JanusResponse> rar = std::make_shared<JanusResponse>();
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