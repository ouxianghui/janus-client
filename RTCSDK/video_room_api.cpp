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

	void VideoRoomApi::action(const std::string& request, std::function<void(std::shared_ptr<JanusResponse>)> callback)
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
		event->message = request;
		event->callback = cb;
		pluginClient->sendMessage(event);
	}

	void VideoRoomApi::join(const vr::PublisherJoinRequest& request, std::function<void(std::shared_ptr<JanusResponse>)> callback)
	{
		std::string json = x2struct::X::tojson(request);
		if (json.empty()) {
			DLOG("empty json string");
			return;
		}
		action(json, callback);
	}

	void VideoRoomApi::join(const vr::SubscriberJoinRequest& request, std::function<void(std::shared_ptr<JanusResponse>)> callback)
	{
		std::string json = x2struct::X::tojson(request);
		if (json.empty()) {
			DLOG("empty json string");
			return;
		}
		action(json, callback);
	}

	void VideoRoomApi::publisherConfigure(const vr::PublisherConfigureRequest& request, std::function<void(std::shared_ptr<JanusResponse>)> callback)
	{
		std::string json = x2struct::X::tojson(request);
		if (json.empty()) {
			DLOG("empty json string");
			return;
		}
		action(json, callback);
	}

	void VideoRoomApi::subscriberConfigure(const vr::SubscriberConfigureRequest& request, std::function<void(std::shared_ptr<JanusResponse>)> callback)
	{
		std::string json = x2struct::X::tojson(request);
		if (json.empty()) {
			DLOG("empty json string");
			return;
		}
		action(json, callback);
	}

	void VideoRoomApi::publish(const vr::PublishRequest& request, std::function<void(std::shared_ptr<JanusResponse>)> callback)
	{
		std::string json = x2struct::X::tojson(request);
		if (json.empty()) {
			DLOG("empty json string");
			return;
		}
		action(json, callback);
	}

	void VideoRoomApi::unpublish(const vr::UnpublishRequest& request, std::function<void(std::shared_ptr<JanusResponse>)> callback)
	{
		std::string json = x2struct::X::tojson(request);
		if (json.empty()) {
			DLOG("empty json string");
			return;
		}
		action(json, callback);
	}

	void VideoRoomApi::startPeerConnection(const vr::StartPeerConnectionRequest& request, std::function<void(std::shared_ptr<JanusResponse>)> callback)
	{
		std::string json = x2struct::X::tojson(request);
		if (json.empty()) {
			DLOG("empty json string");
			return;
		}
		action(json, callback);
	}

	void VideoRoomApi::pausePeerConnection(const vr::PausePeerConnectionRequest& request, std::function<void(std::shared_ptr<JanusResponse>)> callback)
	{
		std::string json = x2struct::X::tojson(request);
		if (json.empty()) {
			DLOG("empty json string");
			return;
		}
		action(json, callback);
	}

	void VideoRoomApi::switchPublisher(const vr::SwitchPublisherRequest& request, std::function<void(std::shared_ptr<JanusResponse>)> callback)
	{
		std::string json = x2struct::X::tojson(request);
		if (json.empty()) {
			DLOG("empty json string");
			return;
		}
		action(json, callback);
	}

	void VideoRoomApi::leave(const vr::LeaveRequest& request, std::function<void(std::shared_ptr<JanusResponse>)> callback)
	{
		std::string json = x2struct::X::tojson(request);
		if (json.empty()) {
			DLOG("empty json string");
			return;
		}
		action(json, callback);
	}

	void VideoRoomApi::allowed(const vr::AllowedRequest& request, std::function<void(std::shared_ptr<vr::AllowedResponse>)> callback)
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
			std::shared_ptr<vr::AllowedResponse> rar = std::make_shared<vr::AllowedResponse>();
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

	void VideoRoomApi::kick(const vr::KickRequest& request, std::function<void(std::shared_ptr<vr::KickResponse>)> callback)
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
			std::shared_ptr<vr::KickResponse> rar = std::make_shared<vr::KickResponse>();
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

	void VideoRoomApi::fetchRoomsList(const vr::FetchRoomsListRequest& request, std::function<void(std::shared_ptr<vr::FetchRoomsListResponse>)> callback)
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
			std::shared_ptr<vr::FetchRoomsListResponse> rar = std::make_shared<vr::FetchRoomsListResponse>();
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

	void VideoRoomApi::fetchParticipants(const vr::FetchParticipantsRequest& request, std::function<void(std::shared_ptr<vr::FetchParticipantsResponse>)> callback)
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
			std::shared_ptr<vr::FetchParticipantsResponse> rar = std::make_shared<vr::FetchParticipantsResponse>();
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