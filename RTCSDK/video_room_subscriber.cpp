#include "video_room_subscriber.h"
#include "string_utils.h"
#include "logger/logger.h"
#include "participant.h"
#include "x2struct.hpp"
#include "thread_manager.h"
#include "Service/app_instance.h"
#include "video_room_api.h"
#include "pc/media_stream.h"
#include "api/media_stream_proxy.h"
#include "api/media_stream_track_proxy.h"

namespace vi {

	VideoRoomSubscriber::VideoRoomSubscriber(std::shared_ptr<WebRTCServiceInterface> wrs, const std::string& plugin, const std::string& opaqueId)
		: PluginClient(wrs)
	{
		_pluginContext->plugin = plugin;
		_pluginContext->opaqueId = opaqueId;
		_listeners = std::make_shared<std::vector<std::weak_ptr<IVideoRoomListener>>>();
		_attached = false;
	}

	VideoRoomSubscriber::~VideoRoomSubscriber()
	{
		if (_pluginContext->webrtcContext->pc) {
			_pluginContext->webrtcContext->pc->Close();
		}
	}

	void VideoRoomSubscriber::addListener(std::shared_ptr<IVideoRoomListener> listener)
	{
		addBizObserver<IVideoRoomListener>(*_listeners, listener);
	}

	void VideoRoomSubscriber::removeListener(std::shared_ptr<IVideoRoomListener> listener)
	{
		removeBizObserver<IVideoRoomListener>(*_listeners, listener);
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
			for (const auto& str : pub.streams) {
				vr::SubscriberJoinRequest::Stream stream;
				stream.feed = pub.id;
				stream.mid = str.mid;
				request.streams.emplace_back(stream);
			}
		}

		if (auto webrtcService = _pluginContext->webrtcService.lock()) {
			std::shared_ptr<SendMessageEvent> event = std::make_shared<vi::SendMessageEvent>();
			auto lambda = [](bool success, const std::string& response) {
				DLOG("response: {}", response.c_str());
				if (response.empty()) {
					return;
				}
				std::shared_ptr<JanusResponse> rar = std::make_shared<JanusResponse>();
				x2struct::X::loadjson(response, *rar, false, true);
			};
			std::shared_ptr<vi::EventCallback> cb = std::make_shared<vi::EventCallback>(lambda);
			event->message = x2struct::X::tojson(request);
			event->callback = cb;
			sendMessage(event);
		}
	}

	void VideoRoomSubscriber::subscribe(const std::vector<vr::Publisher>& publishers)
	{
		vr::SubscribeRequest request;

		request.request = "subscribe";

		for (const auto& pub : publishers) {
			for (const auto& str : pub.streams) {
				vr::SubscribeRequest::Stream stream;
				stream.feed = pub.id;
				stream.mid = str.mid;
				request.streams.emplace_back(stream);
			}
		}

		if (auto webrtcService = _pluginContext->webrtcService.lock()) {
			std::shared_ptr<SendMessageEvent> event = std::make_shared<vi::SendMessageEvent>();
			auto lambda = [](bool success, const std::string& response) {
				DLOG("response: {}", response.c_str());
				if (response.empty()) {
					return;
				}
				std::shared_ptr<JanusResponse> rar = std::make_shared<JanusResponse>();
				x2struct::X::loadjson(response, *rar, false, true);
			};
			std::shared_ptr<vi::EventCallback> cb = std::make_shared<vi::EventCallback>(lambda);
			event->message = x2struct::X::tojson(request);
			event->callback = cb;
			sendMessage(event);
		}
	}

	void VideoRoomSubscriber::unsubscribeFrom(int64_t id)
	{

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

	void VideoRoomSubscriber::onIceState(webrtc::PeerConnectionInterface::IceConnectionState iceState) {}

	void VideoRoomSubscriber::onMediaState(const std::string& media, bool on, const std::string& mid) {}

	void VideoRoomSubscriber::onWebrtcState(bool isActive, const std::string& reason)
	{
		DLOG("Janus says this WebRTC PeerConnection (feed #{}) is {} now", _id, (isActive ? "up" : "down"));
		if (isActive) {
			// TODO: use IVideoRoomApi
			vr::SubscriberConfigureRequest request;
			std::shared_ptr<SendMessageEvent> event = std::make_shared<vi::SendMessageEvent>();
			auto lambda = [](bool success, const std::string& response) {
				DLOG("response: {}", response.c_str());
				if (response.empty()) {
					return;
				}
				std::shared_ptr<JanusResponse> rar = std::make_shared<JanusResponse>();
				x2struct::X::loadjson(response, *rar, false, true);
			};
			std::shared_ptr<vi::EventCallback> cb = std::make_shared<vi::EventCallback>(lambda);
			event->message = x2struct::X::tojson(request);
			event->callback = cb;
			sendMessage(event);
		}
	}

	void VideoRoomSubscriber::onSlowLink(bool uplink, bool lost, const std::string& mid) {}

	void VideoRoomSubscriber::onMessage(const std::string& data, const std::string& jsepString)
	{
		DLOG(" ::: Got a message (subscriber) :::");

		vr::VideoRoomEvent vrEvent;
		x2struct::X::loadjson(data, vrEvent, false, true);

		const auto& pluginData = vrEvent.plugindata;

		if (!pluginData.xhas("plugin")) {
			return;
		}

		if (pluginData.plugin != "janus.plugin.videoroom") {
			return;
		}

		if (!pluginData.data.xhas("videoroom")) {
			return;
		}

		const auto& event = pluginData.data.videoroom;

		if (event == "attached") {
			_attached = true;
			vr::AttachedEvent aEvent;
			x2struct::X::loadjson(data, aEvent, false, true);
			DLOG("Successfully attached to feed in room {}", aEvent.plugindata.data.room);
		}
		else if (event == "event") {
			// Check if we got an event on a simulcast-related event from this publisher
			//const auto& substream = data.substream;
			//const auto& temporal = data.temporal;

			// TODO:
			if (pluginData.data.xhas("updated")) {
				DLOG("updated event.");
			}
			else if (pluginData.data.xhas("error")) {
				DLOG("error event: {}", pluginData.data.error);
			} 
		}

		Jsep jsep;
		x2struct::X::loadjson(jsepString, jsep, false, true);
		if (!jsep.type.empty() && !jsep.sdp.empty()) {
			DLOG("Handling SDP as well...");
			//// Answer and attach
			auto wself = weak_from_this();
			std::shared_ptr<PrepareWebRTCEvent> event = std::make_shared<PrepareWebRTCEvent>();
			auto callback = std::make_shared<CreateAnswerOfferCallback>([wself, roomId = this->_roomId](bool success, const std::string& reason, const JsepConfig& jsepConfig) {
				DLOG("Got a sdp, type: {}, sdp = {}", jsepConfig.type.c_str(), jsepConfig.sdp.c_str());
				auto self = wself.lock();
				if (!self) {
					return;
				}
				if (success) {
					if (auto webrtcService = self->pluginContext()->webrtcService.lock()) {
						// TODO: use IVideoRoomApi
						vr::StartPeerConnectionRequest request;
						request.room = roomId;

						std::shared_ptr<SendMessageEvent> event = std::make_shared<vi::SendMessageEvent>();
						auto lambda = [](bool success, const std::string& response) {
							DLOG("response: {}", response.c_str());
						};

						std::shared_ptr<vi::EventCallback> callback = std::make_shared<vi::EventCallback>(lambda);
						event->message = x2struct::X::tojson(request);
						Jsep jsep;
						jsep.type = jsepConfig.type;
						jsep.sdp = jsepConfig.sdp;
						event->jsep = x2struct::X::tojson(jsep);
						event->callback = callback;
						self->sendMessage(event);
					}
				}
				else {
					DLOG("WebRTC error: {}", reason.c_str());
				}
			});
			event->answerOfferCallback = callback;
			MediaConfig media;
			media.audioSend = false;
			media.videoSend = false;
			event->media = media;
			JsepConfig st;
			st.type = jsep.type;
			st.sdp = jsep.sdp;
			event->jsep = st;
			createAnswer(event);
		}
	}

	void VideoRoomSubscriber::onLocalTrack(rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> track, bool on) {}

	void VideoRoomSubscriber::onRemoteTrack(rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> track, const std::string& mid, bool on)
	{
		if (on) {
			notifyObserver4Change<IVideoRoomListener>(*_listeners, [wself = weak_from_this(), pid = stoul(mid.c_str()), track](const std::shared_ptr<IVideoRoomListener>& listener) {
				auto self = wself.lock();
				if (!self) {
					return;
				}
				if (!track) {
					return;
				}

				auto vrs = std::dynamic_pointer_cast<VideoRoomSubscriber>(self);

				if (track->kind() == webrtc::MediaStreamTrackInterface::kVideoKind) {
					vrs->_remoteStreams[track->id()] = webrtc::MediaStreamProxy::Create(TMgr->thread(ThreadName::MEDIA_STREAM), webrtc::MediaStream::Create(track->id()));
					auto vt = dynamic_cast<webrtc::VideoTrackInterface*>(track.get());
					vrs->_remoteStreams[track->id()]->AddTrack(vt);
					auto t = vrs->_remoteStreams[track->id()]->GetVideoTracks()[0];
					listener->onCreateVideoTrack(pid, t);
				}
			});
		}
		else {
			notifyObserver4Change<IVideoRoomListener>(*_listeners, [wself = weak_from_this(), pid = stoul(mid.c_str()), track](const std::shared_ptr<IVideoRoomListener>& listener) {
				auto self = wself.lock();
				if (!self) {
					return;
				}
				if (!track) {
					return;
				}

				auto vrs = std::dynamic_pointer_cast<VideoRoomSubscriber>(self);

				if (track->kind() == webrtc::MediaStreamTrackInterface::kVideoKind) {
					if (vrs->_remoteStreams.find(track->id()) != vrs->_remoteStreams.end()) {
						auto vt = vrs->_remoteStreams[track->id()]->GetVideoTracks()[0];
						listener->onRemoveVideoTrack(pid, vt);
						vrs->_remoteStreams[track->id()]->RemoveTrack(vt.get());
						auto it = vrs->_remoteStreams.find(track->id());
						vrs->_remoteStreams.erase(it);
					}
				}
			});
		}
	}

	void VideoRoomSubscriber::onData(const std::string& data, const std::string& label) {}

	void VideoRoomSubscriber::onDataOpen(const std::string& label) {}

	void VideoRoomSubscriber::onCleanup() {}

	void VideoRoomSubscriber::onDetached() {}

	void VideoRoomSubscriber::onStatsReport(const rtc::scoped_refptr<const webrtc::RTCStatsReport>& report) {}
}