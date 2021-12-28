#include "video_room_subscriber.h"
#include "utils/string_utils.h"
#include "logger/logger.h"
#include "participant.h"
#include "utils/thread_provider.h"
#include "Service/rtc_engine.h"
#include "video_room_api.h"
#include "pc/media_stream.h"
#include "pc/media_stream_proxy.h"
#include "pc/media_stream_track_proxy.h"
#include "media_controller.h"

namespace vi {

	VideoRoomSubscriber::VideoRoomSubscriber(std::shared_ptr<SignalingClientInterface> ss, const std::string& plugin, const std::string& opaqueId, std::shared_ptr<MediaController> mediaController)
		: PluginClient(ss)
		, _mediaController(mediaController)
	{
		_pluginContext->plugin = plugin;
		_pluginContext->opaqueId = opaqueId;
		_attached = false;
	}

	VideoRoomSubscriber::~VideoRoomSubscriber()
	{
		DLOG("~VideoRoomSubscriber()");
	}

	void VideoRoomSubscriber::init()
	{
		PluginClient::init();
	}

	void VideoRoomSubscriber::registerEventHandler(std::shared_ptr<IVideoRoomEventHandler> handler)
	{
		UniversalObservable<IVideoRoomEventHandler>::addWeakObserver(handler, std::string("main"));
	}

	void VideoRoomSubscriber::unregisterEventHandler(std::shared_ptr<IVideoRoomEventHandler> handler)
	{
		UniversalObservable<IVideoRoomEventHandler>::removeObserver(handler);
	}

	void VideoRoomSubscriber::setRoomApi(std::shared_ptr<IVideoRoomApi> videoRoomApi)
	{
		_videoRoomApi = videoRoomApi;
	}

	void VideoRoomSubscriber::setRoomId(int64_t roomId)
	{
		_roomId = roomId;
	}

	int64_t VideoRoomSubscriber::getRoomId() const
	{
		return _roomId;
	}

	void VideoRoomSubscriber::setPrivateId(int64_t id)
	{
		_privateId = id;
	}

	void VideoRoomSubscriber::subscribeTo(const std::vector<vr::Publisher>& publishers)
	{
		if (_attached) {
			subscribe(publishers);
		}
		else {
			this->attach();
			_joinTask = [wself = weak_from_this(), publishers]() {
				auto self = wself.lock();
				if (!self) {
					return;
				}
				auto vrs = std::dynamic_pointer_cast<VideoRoomSubscriber>(self);
				vrs->join(publishers);
			};
		}
	}

	void VideoRoomSubscriber::join(const std::vector<vr::Publisher>& publishers)
	{
		vr::SubscriberJoinRequest request;

		request.request = "join";
		request.room = _roomId;
		request.ptype = "subscriber";
		request.private_id = _privateId;

		for (const auto& pub : publishers) {
			if (pub.streams) {
				auto ss = std::vector<vr::SubscriberJoinRequest::Stream>();
				for (const auto& str : pub.streams.value()) {
					vr::SubscriberJoinRequest::Stream stream;
					stream.feed = pub.id;
					stream.mid = str.mid;
					ss.emplace_back(stream);
				}
				if (!ss.empty()) {
					request.streams = ss;
				}
			}
		}

		std::shared_ptr<MessageEvent> event = std::make_shared<vi::MessageEvent>();
		auto lambda = [](bool success, const std::string& response) {
			DLOG("response: {}", response.c_str());
			if (response.empty()) {
				return;
			}

			std::string err;
			std::shared_ptr<JanusResponse> rar = fromJsonString<JanusResponse>(response, err);
			if (!err.empty()) {
				DLOG("parse JanusResponse failed");
				return;
			}
		};
		std::shared_ptr<vi::EventCallback> cb = std::make_shared<vi::EventCallback>(lambda);
		event->message = request.toJsonStr();
		DLOG("request.toJsonStr(): {}", request.toJsonStr());
		event->callback = cb;
		sendMessage(event);
	}

	void VideoRoomSubscriber::subscribe(const std::vector<vr::Publisher>& publishers)
	{
		vr::SubscribeRequest request;

		request.request = "subscribe";

		for (const auto& pub : publishers) {
			if (pub.streams) {
				auto ss = std::vector<vr::SubscribeRequest::Stream>();
				for (const auto& str : pub.streams.value()) {
					vr::SubscribeRequest::Stream stream;
					stream.feed = pub.id;
					stream.mid = str.mid;
					ss.emplace_back(stream);
				}
				if (!ss.empty()) {
					request.streams = ss;
				}
			}
		}

		std::shared_ptr<MessageEvent> event = std::make_shared<vi::MessageEvent>();
		auto lambda = [](bool success, const std::string& response) {
			DLOG("response: {}", response.c_str());
			if (response.empty()) {
				return;
			}

			std::string err;
			std::shared_ptr<JanusResponse> rar = fromJsonString<JanusResponse>(response, err);
			if (!err.empty()) {
				DLOG("parse JanusResponse failed");
				return;
			}
		};
		std::shared_ptr<vi::EventCallback> cb = std::make_shared<vi::EventCallback>(lambda);
		event->message = request.toJsonStr();
		event->callback = cb;
		sendMessage(event);
	}

	void VideoRoomSubscriber::unsubscribeFrom(int64_t id)
	{
		vr::UnsubscribeRequest request;

		request.request = "subscribe";

		vr::UnsubscribeRequest::Stream stream;
		stream.feed = id;
		auto ss = std::vector<vr::UnsubscribeRequest::Stream>();
		ss.emplace_back(stream);
		request.streams = ss;

		std::shared_ptr<MessageEvent> event = std::make_shared<vi::MessageEvent>();
		auto lambda = [](bool success, const std::string& response) {
			DLOG("response: {}", response.c_str());
			if (response.empty()) {
				return;
			}

			std::string err;
			std::shared_ptr<JanusResponse> rar = fromJsonString<JanusResponse>(response, err);
			if (!err.empty()) {
				DLOG("parse JanusResponse failed");
				return;
			}
		};
		std::shared_ptr<vi::EventCallback> cb = std::make_shared<vi::EventCallback>(lambda);
		event->message = request.toJsonStr();
		event->callback = cb;
		sendMessage(event);
	}

	void VideoRoomSubscriber::onAttached(bool success)
	{
		if (success) {
			if (_joinTask) {
				_joinTask();
			}
		}
		else {
			DLOG("  -- Error attaching plugin...");
		}
	}

	void VideoRoomSubscriber::onHangup() {}

	void VideoRoomSubscriber::onMediaStatus(const std::string& media, bool on, const std::string& mid) {}

	void VideoRoomSubscriber::onWebrtcStatus(bool isActive, const std::string& reason)
	{
		DLOG("Janus says this WebRTC PeerConnection (remote feed) is {} now", (isActive ? "up" : "down"));
	}

	void VideoRoomSubscriber::onSlowLink(bool uplink, bool lost, const std::string& mid) 
	{
		DLOG("Janus reports problems {} packets on mid {} ({} lost packets)", (uplink ? "sending" : "receiving"), mid, lost);
	}

	void VideoRoomSubscriber::onMessage(const std::string& data, const std::string& jsepString)
	{
		DLOG(" ::: Got a message (subscriber) :::");

		std::string err;
		std::shared_ptr<vr::VideoRoomEvent> vrEvent = fromJsonString<vr::VideoRoomEvent>(data, err);
		if (!err.empty()) {
			DLOG("parse JanusResponse failed");
			return;
		}

		const auto& pluginData = vrEvent->plugindata;

		if (!pluginData->plugin) {
			return;
		}

		if (pluginData->plugin.value_or("") != "janus.plugin.videoroom") {
			return;
		}

		if (!pluginData->data->videoroom) {
			return;
		}

		const auto& event = pluginData->data->videoroom;

		if (event.value_or("") == "attached") {
			_attached = true;
			std::string err;
			std::shared_ptr<vr::AttachedEvent> aEvent = fromJsonString<vr::AttachedEvent>(data, err);
			if (!err.empty()) {
				DLOG("parse JanusResponse failed");
				return;
			}

			DLOG("Successfully attached to feed in room {}", aEvent->plugindata->data->room.value_or(0));
		}
		else if (event.value_or("") == "event") {
			// Check if we got an event on a simulcast-related event from this publisher
			//const auto& substream = data.substream;
			//const auto& temporal = data.temporal;

			if (pluginData->data->error) {
				DLOG("error event: {}", pluginData->data->error.value_or(""));
			}
		}

		if (jsepString.empty()) {
			return;
		}
		err.clear();
		std::shared_ptr<Jsep> jsep = fromJsonString<Jsep>(jsepString, err);
		if (!err.empty()) {
			DLOG("parse JanusResponse failed");
			return;
		}

		if (jsep->type && jsep->sdp && !jsep->type.value().empty() && !jsep->sdp.value().empty()) {
			DLOG("Handling SDP as well...");
			//// Answer and attach
			std::shared_ptr<PrepareWebrtcEvent> event = std::make_shared<PrepareWebrtcEvent>();
			_pluginContext->offerAnswerCallback = std::make_shared<CreateOfferAnswerCallback>([wself = weak_from_this(), roomId = this->_roomId](bool success, const std::string& reason, const JsepConfig& jsepConfig) {
				DLOG("Got a sdp, type: {}, sdp = {}", jsepConfig.type.c_str(), jsepConfig.sdp.c_str());
				auto self = wself.lock();
				if (!self) {
					return;
				}
				if (success) {
					// TODO: use IVideoRoomApi
					vr::StartPeerConnectionRequest request;
					request.room = roomId;

					std::shared_ptr<MessageEvent> event = std::make_shared<vi::MessageEvent>();
					auto lambda = [](bool success, const std::string& response) {
						DLOG("response: {}", response.c_str());
					};

					std::shared_ptr<vi::EventCallback> callback = std::make_shared<vi::EventCallback>(lambda);
					event->message = request.toJsonStr();
					Jsep jsep;
					jsep.type = jsepConfig.type;
					jsep.sdp = jsepConfig.sdp;
					event->jsep = jsep.toJsonStr();
					event->callback = callback;
					self->sendMessage(event);
				}
				else {
					DLOG("WebRTC error: {}", reason.c_str());
				}
			});
			MediaConfig media;
			media.audioSend = false;
			media.videoSend = false;
			event->media = media;
			JsepConfig st;
			st.type = jsep->type.value_or("");
			st.sdp = jsep->sdp.value_or("");
			event->jsep = st;
			createAnswer(event);
		}
	}

	void VideoRoomSubscriber::onTimeout()
	{

	}

	void VideoRoomSubscriber::onError(const std::string& desc)
	{

	}

	void VideoRoomSubscriber::onCleanup() 
	{
		PluginClient::onCleanup();
	}

	void VideoRoomSubscriber::onDetached() {}

	void VideoRoomSubscriber::onRemoteTrack(rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> track, const std::string& mid, bool on)
	{
		if (auto mc = _mediaController.lock()) {
			mc->onRemoteTrack(track, mid, on);
		}
	}
}