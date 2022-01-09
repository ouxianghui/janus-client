/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-12-9
 **/

#include "video_room_api.h"
#include "signaling_events.h"
#include "logger/logger.h"
#include "video_room_models.h"
#include "plugin_client.h"

namespace vi {

	VideoRoomApi::VideoRoomApi(std::shared_ptr<PluginClient> pluginClient)
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
		std::shared_ptr<MessageEvent> event = std::make_shared<vi::MessageEvent>();
		auto lambda = [callback](bool success, const std::string& response) {
			DLOG("response: {}", response.c_str());
			if (response.empty()) {
				return;
			}

			std::string err;

			std::shared_ptr<vr::RoomCurdResponse> jr = fromJsonString<vr::RoomCurdResponse>(response, err);

			if (!err.empty()) {
				DLOG("parse JanusResponse failed");
				return;
			}

			if (callback) {
				callback(jr);
			}
		};
		std::shared_ptr<vi::EventCallback> cb = std::make_shared<vi::EventCallback>(lambda);
		event->message = request;
		event->callback = cb;
		pluginClient->sendMessage(event);
	}

	void VideoRoomApi::create(std::shared_ptr<vr::CreateRoomRequest> request, std::function<void(std::shared_ptr<vr::RoomCurdResponse>)> callback)
	{
		std::string json = request->toJsonStr();
		if (json.empty()) {
			DLOG("empty json string");
			return;
		}
		curd(json, callback);
	}

	void VideoRoomApi::destroy(std::shared_ptr<vr::DestroyRoomRequest> request, std::function<void(std::shared_ptr<vr::RoomCurdResponse>)> callback)
	{
		std::string json = request->toJsonStr();
		if (json.empty()) {
			DLOG("empty json string");
			return;
		}
		curd(json, callback);
	}

	void VideoRoomApi::edit(std::shared_ptr<vr::EditRoomRequest> request, std::function<void(std::shared_ptr<vr::RoomCurdResponse>)> callback)
	{
		std::string json = request->toJsonStr();
		if (json.empty()) {
			DLOG("empty json string");
			return;
		}
		curd(json, callback);
	}

	void VideoRoomApi::exists(std::shared_ptr<vr::ExistsRequest> request, std::function<void(std::shared_ptr<vr::RoomCurdResponse>)> callback)
	{
		std::string json = request->toJsonStr();
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
		std::shared_ptr<MessageEvent> event = std::make_shared<vi::MessageEvent>();
		auto lambda = [callback](bool success, const std::string& response) {
			DLOG("response: {}", response.c_str());
			if (response.empty()) {
				return;
			}

			std::string err;

			std::shared_ptr<JanusResponse> jr = fromJsonString<JanusResponse>(response, err);

			if (!err.empty()) {
				DLOG("parse JanusResponse failed");
				return;
			}

			if (callback) {
				callback(jr);
			}
		};
		std::shared_ptr<vi::EventCallback> cb = std::make_shared<vi::EventCallback>(lambda);
		event->message = request;
		event->callback = cb;
		pluginClient->sendMessage(event);
	}

	void VideoRoomApi::join(std::shared_ptr<vr::PublisherJoinRequest> request, std::function<void(std::shared_ptr<JanusResponse>)> callback)
	{
		std::string json = request->toJsonStr();
		if (json.empty()) {
			DLOG("empty json string");
			return;
		}
		action(json, callback);
	}

	void VideoRoomApi::join(std::shared_ptr<vr::SubscriberJoinRequest> request, std::function<void(std::shared_ptr<JanusResponse>)> callback)
	{
		std::string json = request->toJsonStr();
		if (json.empty()) {
			DLOG("empty json string");
			return;
		}
		action(json, callback);
	}

	void VideoRoomApi::publisherConfigure(std::shared_ptr<vr::PublisherConfigureRequest> request, std::function<void(std::shared_ptr<JanusResponse>)> callback)
	{
		std::string json = request->toJsonStr();
		if (json.empty()) {
			DLOG("empty json string");
			return;
		}
		action(json, callback);
	}

	void VideoRoomApi::subscriberConfigure(std::shared_ptr<vr::SubscriberConfigureRequest> request, std::function<void(std::shared_ptr<JanusResponse>)> callback)
	{
		std::string json = request->toJsonStr();
		if (json.empty()) {
			DLOG("empty json string");
			return;
		}
		action(json, callback);
	}

	void VideoRoomApi::publish(std::shared_ptr<vr::PublishRequest> request, std::function<void(std::shared_ptr<JanusResponse>)> callback)
	{
		std::string json = request->toJsonStr();
		if (json.empty()) {
			DLOG("empty json string");
			return;
		}
		action(json, callback);
	}

	void VideoRoomApi::unpublish(std::shared_ptr<vr::UnpublishRequest> request, std::function<void(std::shared_ptr<JanusResponse>)> callback)
	{
		std::string json = request->toJsonStr();
		if (json.empty()) {
			DLOG("empty json string");
			return;
		}
		action(json, callback);
	}

	void VideoRoomApi::subscribe(std::shared_ptr<vr::SubscribeRequest> request, std::function<void(std::shared_ptr<JanusResponse>)> callback)
	{
		std::string json = request->toJsonStr();
		if (json.empty()) {
			DLOG("empty json string");
			return;
		}
		action(json, callback);
	}

	void VideoRoomApi::unsubscribe(std::shared_ptr<vr::UnsubscribeRequest> request, std::function<void(std::shared_ptr<JanusResponse>)> callback)
	{
		std::string json = request->toJsonStr();
		if (json.empty()) {
			DLOG("empty json string");
			return;
		}
		action(json, callback);
	}

	void VideoRoomApi::startPeerConnection(std::shared_ptr<vr::StartPeerConnectionRequest> request, std::function<void(std::shared_ptr<JanusResponse>)> callback)
	{
		std::string json = request->toJsonStr();
		if (json.empty()) {
			DLOG("empty json string");
			return;
		}
		action(json, callback);
	}

	void VideoRoomApi::pausePeerConnection(std::shared_ptr<vr::PausePeerConnectionRequest> request, std::function<void(std::shared_ptr<JanusResponse>)> callback)
	{
		std::string json = request->toJsonStr();
		if (json.empty()) {
			DLOG("empty json string");
			return;
		}
		action(json, callback);
	}

	void VideoRoomApi::switchPublisher(std::shared_ptr<vr::SwitchPublisherRequest> request, std::function<void(std::shared_ptr<JanusResponse>)> callback)
	{
		std::string json = request->toJsonStr();
		if (json.empty()) {
			DLOG("empty json string");
			return;
		}
		action(json, callback);
	}

	void VideoRoomApi::leave(std::shared_ptr<vr::LeaveRequest> request, std::function<void(std::shared_ptr<JanusResponse>)> callback)
	{
		std::string json = request->toJsonStr();
		if (json.empty()) {
			DLOG("empty json string");
			return;
		}
		action(json, callback);
	}

	void VideoRoomApi::allowed(std::shared_ptr<vr::AllowedRequest> request, std::function<void(std::shared_ptr<vr::AllowedResponse>)> callback)
	{
		auto pluginClient = _pluginClient.lock();
		if (!pluginClient) {
			DLOG("invalid plugin client");
			return;
		}
		std::shared_ptr<MessageEvent> event = std::make_shared<vi::MessageEvent>();
		auto lambda = [callback](bool success, const std::string& response) {
			DLOG("response: {}", response.c_str());
			if (response.empty()) {
				return;
			}

			std::string err;

			std::shared_ptr<vr::AllowedResponse> jr = fromJsonString<vr::AllowedResponse>(response, err);

			if (!err.empty()) {
				DLOG("parse AllowedResponse failed");
				return;
			}

			if (callback) {
				callback(jr);
			}
		};
		std::shared_ptr<vi::EventCallback> cb = std::make_shared<vi::EventCallback>(lambda);
		event->message = request->toJsonStr();
		event->callback = cb;
		pluginClient->sendMessage(event);
	}

	void VideoRoomApi::kick(std::shared_ptr<vr::KickRequest> request, std::function<void(std::shared_ptr<vr::KickResponse>)> callback)
	{
		auto pluginClient = _pluginClient.lock();
		if (!pluginClient) {
			DLOG("invalid plugin client");
			return;
		}
		std::shared_ptr<MessageEvent> event = std::make_shared<vi::MessageEvent>();
		auto lambda = [callback](bool success, const std::string& response) {
			DLOG("response: {}", response.c_str());
			if (response.empty()) {
				return;
			}

			std::string err;

			std::shared_ptr<vr::KickResponse> jr = fromJsonString<vr::KickResponse>(response, err);

			if (!err.empty()) {
				DLOG("parse KickResponse failed");
				return;
			}

			if (callback) {
				callback(jr);
			}
		};
		std::shared_ptr<vi::EventCallback> cb = std::make_shared<vi::EventCallback>(lambda);
		event->message = request->toJsonStr();
		event->callback = cb;
		pluginClient->sendMessage(event);
	}

	void VideoRoomApi::moderate(std::shared_ptr<vr::ModerateRequest> request, std::function<void(std::shared_ptr<vr::ModerateResponse>)> callback)
	{
		auto pluginClient = _pluginClient.lock();
		if (!pluginClient) {
			DLOG("invalid plugin client");
			return;
		}
		std::shared_ptr<MessageEvent> event = std::make_shared<vi::MessageEvent>();
		auto lambda = [callback](bool success, const std::string& response) {
			DLOG("response: {}", response.c_str());
			if (response.empty()) {
				return;
			}

			std::string err;

			std::shared_ptr<vr::ModerateResponse> jr = fromJsonString<vr::ModerateResponse>(response, err);

			if (!err.empty()) {
				DLOG("parse ModerateResponse failed");
				return;
			}

			if (callback) {
				callback(jr);
			}
		};
		std::shared_ptr<vi::EventCallback> cb = std::make_shared<vi::EventCallback>(lambda);
		event->message = request->toJsonStr();
		event->callback = cb;
		pluginClient->sendMessage(event);
	}


	void VideoRoomApi::fetchRoomsList(std::shared_ptr<vr::FetchRoomsListRequest> request, std::function<void(std::shared_ptr<vr::FetchRoomsListResponse>)> callback)
	{
		auto pluginClient = _pluginClient.lock();
		if (!pluginClient) {
			DLOG("invalid plugin client");
			return;
		}
		std::shared_ptr<MessageEvent> event = std::make_shared<vi::MessageEvent>();
		auto lambda = [callback](bool success, const std::string& response) {
			DLOG("response: {}", response.c_str());
			if (response.empty()) {
				return;
			}

			std::string err;

			std::shared_ptr<vr::FetchRoomsListResponse> jr = fromJsonString<vr::FetchRoomsListResponse>(response, err);

			if (!err.empty()) {
				DLOG("parse FetchRoomsListResponse failed");
				return;
			}

			if (callback) {
				callback(jr);
			}
		};
		std::shared_ptr<vi::EventCallback> cb = std::make_shared<vi::EventCallback>(lambda);
		event->message = request->toJsonStr();
		event->callback = cb;
		pluginClient->sendMessage(event);
	}

	void VideoRoomApi::fetchParticipants(std::shared_ptr<vr::FetchParticipantsRequest> request, std::function<void(std::shared_ptr<vr::FetchParticipantsResponse>)> callback)
	{
		auto pluginClient = _pluginClient.lock();
		if (!pluginClient) {
			DLOG("invalid plugin client");
			return;
		}
		std::shared_ptr<MessageEvent> event = std::make_shared<vi::MessageEvent>();
		auto lambda = [callback](bool success, const std::string& response) {
			DLOG("response: {}", response.c_str());
			if (response.empty()) {
				return;
			}

			std::string err;

			std::shared_ptr<vr::FetchParticipantsResponse> jr = fromJsonString<vr::FetchParticipantsResponse>(response, err);

			if (!err.empty()) {
				DLOG("parse FetchParticipantsResponse failed");
				return;
			}

			if (callback) {
				callback(jr);
			}
		};
		std::shared_ptr<vi::EventCallback> cb = std::make_shared<vi::EventCallback>(lambda);
		event->message = request->toJsonStr();
		event->callback = cb;
		pluginClient->sendMessage(event);
	}
}