/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#include "participant.h"
#include "logger/logger.h"

namespace vi {
	Participant::Participant(const std::string& plugin, 
		const std::string& opaqueId,
		int64_t id,
		int64_t privateId,
		const std::string& displayName,
		std::shared_ptr<WebRTCServiceInterface> wrs,
		std::shared_ptr<std::vector<std::weak_ptr<IVideoRoomListener>>> listeners)
		: PluginClient(wrs)
		, _displayName(displayName)
		, _listeners(listeners)
	{
		_pluginContext->plugin = plugin;
		_pluginContext->opaqueId = opaqueId;
		_id = id;
		_privateId = privateId;
	}

	Participant::~Participant()
	{
	}

	void Participant::onAttached(bool success)
	{
		if (success) {
			vr::SubscriberJoinRequest request;
			request.request = "join";
			request.room = 1234;
			request.ptype = "subscriber";
			request.feed = _id;
			request.private_id = _privateId;
			
			// In case you don't want to receive audio, video or data, even if the
			// publisher is sending them, set the 'offer_audio', 'offer_video' or
			// 'offer_data' properties to false (they're true by default), e.g.:
			// 		subscribe["offer_video"] = false;
			if (auto webrtcService = _pluginContext->webrtcService.lock()) {
				std::shared_ptr<SendMessageEvent> event = std::make_shared<vi::SendMessageEvent>();
				auto lambda = [](bool success, const std::string& response) {
					DLOG("response: {}", response.c_str());
				};
				std::shared_ptr<vi::EventCallback> callback = std::make_shared<vi::EventCallback>(lambda);
				event->message = x2struct::X::tojson(request);
				DLOG("event->message: {}", event->message.c_str());
				event->callback = callback;
				sendMessage(event);
			}
		}
		else {
			DLOG("  -- Error attaching plugin...");
		}
	}

	void Participant::onHangup() {}

	void Participant::onIceState(webrtc::PeerConnectionInterface::IceConnectionState iceState) {}

	void Participant::onMediaState(const std::string& media, bool on) {}

	void Participant::onWebrtcState(bool isActive, const std::string& reason)
	{
		DLOG("Janus says this WebRTC PeerConnection (feed #{}) is {} now", _id, (isActive ? "up" : "down"));
	}

	void Participant::onSlowLink(bool uplink, bool lost) {}

	void Participant::onMessage(const EventData& data, const Jsep& jsep)
	{
		DLOG(" ::: Got a message (subscriber) :::");

		if (!data.xhas("videoroom")) {
			return;
		}
		const auto& event = data.videoroom;
		if (event == "attached") {
			int64_t remoteId = data.id;
			std::string remoteName = data.display;
			DLOG("Successfully attached to feed {} ({}) in room {}", remoteId, remoteName.c_str(), data.room);
		}
		else if (event == "event") {
			// Check if we got an event on a simulcast-related event from this publisher
			//const auto& substream = data.substream;
			//const auto& temporal = data.temporal;
		}
		else if (event == "error") {
			DLOG("  -- Error attaching plugin...");
		}

		if (!jsep.type.empty() && !jsep.sdp.empty()) {
			DLOG("Handling SDP as well...");
			//// Answer and attach
			auto wself = weak_from_this();
			std::shared_ptr<PrepareWebRTCEvent> event = std::make_shared<PrepareWebRTCEvent>();
			auto callback = std::make_shared<CreateAnswerOfferCallback>([wself](bool success, const std::string& reason, const JsepConfig& jsep) {
				DLOG("Got a sdp, type: {}, sdp = {}", jsep.type.c_str(), jsep.sdp.c_str());
				auto self = wself.lock();
				if (!self) {
					return;
				}
				if (success) {
					vr::StartRequest request;
					request.room = 1234;
					if (auto webrtcService = self->pluginContext()->webrtcService.lock()) {
						std::shared_ptr<SendMessageEvent> event = std::make_shared<vi::SendMessageEvent>();
						auto lambda = [](bool success, const std::string& response) {
							DLOG("response: {}", response.c_str());
						};
						std::shared_ptr<vi::EventCallback> callback = std::make_shared<vi::EventCallback>(lambda);
						event->message = x2struct::X::tojson(request);
						Jsep jp;
						jp.type = jsep.type;
						jp.sdp = jsep.sdp;
						event->jsep = x2struct::X::tojson(jp);
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

	void Participant::onCreateLocalStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) {}

	void Participant::onDeleteLocalStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) {}

	void Participant::onCreateRemoteStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) 
	{
		//// TODO: render
		//if (auto renderer = _renderer.lock()) {
		//	rtc::VideoSinkWants wants;
		//	stream->GetVideoTracks()[0]->AddOrUpdateSink(renderer.get(), wants);
		//}

		if (auto listeners = _listeners.lock()) {
			notifyObserver4Change<IVideoRoomListener>(*listeners, [pid = _id, stream](const std::shared_ptr<IVideoRoomListener>& listener) {
				listener->onCreateStream(pid, stream);
			});
		}

	}

	void Participant::onDeleteRemoteStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream)
	{
		if (auto listeners = _listeners.lock()) {
			notifyObserver4Change<IVideoRoomListener>(*listeners, [pid = _id, stream](const std::shared_ptr<IVideoRoomListener>& listener) {
				listener->onDeleteStream(pid, stream);
			});
		}
	}

	void Participant::onData(const std::string& data, const std::string& label) {}

	void Participant::onDataOpen(const std::string& label) {}

	void Participant::onCleanup() {}

	void Participant::onDetached() {}
}