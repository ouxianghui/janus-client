/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#include "video_room_client.h"
#include "string_utils.h"
#include "logger/logger.h"
#include "participant.h"
#include "Service/app_instance.h"
#include "video_room_api.h"
#include "video_room_subscriber.h"
#include "pc/media_stream.h"
#include "pc/media_stream_proxy.h"
#include "pc/media_stream_track_proxy.h"
#include "media_controller.h"
#include "media_controller_proxy.h"
#include "participants_controller.h"
#include "participants_controller_proxy.h"

namespace vi {
	VideoRoomClient::VideoRoomClient(std::shared_ptr<SignalingServiceInterface> ss)
		: PluginClient(ss)
	{
		_pluginContext->plugin = "janus.plugin.videoroom";
		_pluginContext->opaqueId = "videoroom-" + StringUtils::randomString(12);
	}

	VideoRoomClient::~VideoRoomClient()
	{
		DLOG("~VideoRoomClient()");
	}

	void VideoRoomClient::init()
	{
		PluginClient::init();

		_mediaController = std::make_shared<MediaController>();
		_mediaControllerProxy = MediaControllerProxy::Create(TMgr->thread("plugin-client"), _mediaController);

		_participantsController = std::make_shared<ParticipantsContrller>();
		_participantsControllerProxy = ParticipantsContrllerProxy::Create(TMgr->thread("plugin-client"), _participantsController);

		_videoRoomApi = std::make_shared<VideoRoomApi>(shared_from_this());

		_subscriber = std::make_shared<VideoRoomSubscriber>(_pluginContext->signalingService.lock(), _pluginContext->plugin, _pluginContext->opaqueId, _mediaController);
		_subscriber->init();
		_subscriber->setRoomApi(_videoRoomApi);
	}

	void VideoRoomClient::destroy()
	{

	}

	void VideoRoomClient::registerEventHandler(std::shared_ptr<IVideoRoomEventHandler> handler)
	{
		UniversalObservable<IVideoRoomEventHandler>::addWeakObserver(handler, std::string("main"));

		_subscriber->registerEventHandler(handler);
	}

	void VideoRoomClient::unregisterEventHandler(std::shared_ptr<IVideoRoomEventHandler> handler)
	{
		UniversalObservable<IVideoRoomEventHandler>::removeObserver(handler);

		_subscriber->unregisterEventHandler(handler);
	}

	void VideoRoomClient::attach()
	{
		PluginClient::attach();
	}

	void VideoRoomClient::detach()
	{
		auto event = std::make_shared<DetachEvent>();
		PluginClient::detach(event);
		_subscriber->detach(event);
	}
	
	//std::shared_ptr<Participant> VideoRoomClient::getParticipant(int64_t pid)
	//{
	//	return _participantsMap.find(pid) == _participantsMap.end() ? nullptr : _participantsMap[pid];
	//}

	//void VideoRoomClient::setRoomId(int64_t roomId)
	//{
	//	_roomId = roomId;
	//	_subscriber->setRoomId(_roomId);
	//}

	//int64_t VideoRoomClient::getRoomId() const
	//{
	//	return _roomId;
	//}

	//std::shared_ptr<IVideoRoomApi> VideoRoomClient::getVideoRoomApi()
	//{
	//	return _videoRoomApi;
	//}


	void VideoRoomClient::create(std::shared_ptr<vr::CreateRoomRequest> request)
	{

	}

	void VideoRoomClient::join(std::shared_ptr<vr::PublisherJoinRequest> request)
	{
		_roomId = request->room.value();
		_subscriber->setRoomId(_roomId);
		if (_videoRoomApi) {
			_videoRoomApi->join(request, nullptr);
		}
	}

	void VideoRoomClient::leave(std::shared_ptr<vr::LeaveRequest> request)
	{

	}

	std::shared_ptr<ParticipantsContrllerInterface> VideoRoomClient::participantsController()
	{
		return _participantsControllerProxy;
	}

	std::shared_ptr<MediaControllerInterface> VideoRoomClient::mediaContrller()
	{
		return _mediaControllerProxy;
	}

	void VideoRoomClient::onAttached(bool success)
	{
		if (success) {
			DLOG("Plugin attached! ({}, id = {})", _pluginContext->plugin.c_str(), _id);
			DLOG("  -- This is a publisher/manager");
		}
		else {
			ELOG("  -- Error attaching plugin...");
		}
	}

	void VideoRoomClient::onHangup() {}

	void VideoRoomClient::onMediaStatus(const std::string& media, bool on, const std::string& mid)
	{
		DLOG("Janus {} receiving our {}", (on ? "started" : "stopped"), media.c_str());
	}

	void VideoRoomClient::onWebrtcStatus(bool isActive, const std::string& reason)
	{
		DLOG("Janus says our WebRTC PeerConnection is {} now", (isActive ? "up" : "down"));
		if (isActive) {
			if (auto webrtcService = _pluginContext->signalingService.lock()) {
				auto request = std::make_shared<vr::PublishRequest>();
				request->request = "configure";
				request->bitrate = 256000;

				/*
				 * After debugging, the SFU does receive the display name set when we join,
				 * but the subscriber is not displayed. It should be a SFU bug.
				 * After setting once here, the subscriber of the later join will have the opportunity to display your display name.
				 */
				 //request.display = "input your display name here";
				_videoRoomApi->publish(request, [](std::shared_ptr<JanusResponse> response) {
					DLOG("response: {}", response->janus.value_or(""));
				});
			}

			if (_mediaController) {
				_mediaController->onWebrtcStatus(isActive, reason);
			}
		}
		// TODO:-----------------------
		//unmuteVideo("");
		startStatsMonitor();
	}

	void VideoRoomClient::onSlowLink(bool uplink, bool lost, const std::string& mid) {}

	void VideoRoomClient::onMessage(const std::string& data, const std::string& jsepString)
	{
		DLOG(" ::: Got a message (publisher).");

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

		if (event.value_or("") == "joined") {
			std::string err;
			std::shared_ptr<vr::PublisherJoinEvent> pjEvent = fromJsonString<vr::PublisherJoinEvent>(data, err);
			if (!err.empty()) {
				DLOG("parse JanusResponse failed");
				return;
			}

			const auto& pluginData = pjEvent->plugindata;
			// Publisher/manager created, negotiate WebRTC and attach to existing feeds, if any
			_id = pluginData->data->id.value();
			_privateId = pluginData->data->private_id.value();
			_subscriber->setPrivateId(_privateId);
			DLOG("Successfully joined room {} with ID {}", pluginData->data->room.value_or(0), _id);

			// TODO:
			publishStream(true);

			// Any new feed to attach to
			if (pluginData->data->publishers && !pluginData->data->publishers->empty()) {
				const auto& publishers = pluginData->data->publishers.value();
				DLOG("Got a list of available publishers/feeds:");
        				for (const auto& pub : publishers) {
					DLOG("  >> [{}] {}", pub.id.value(), pub.display.value_or(""));

					auto participant = std::make_shared<Participant>(pub.id.value(), pub.display.value_or(""));
					createParticipant(participant);
				}
				_subscriber->subscribeTo(publishers);
			}
		}
		else if (event.value_or("") == "destroyed") {
			ELOG("The room has been destroyed!");
		}
		else if (event.value_or("") == "event") {
			// Any new feed to attach to
			if (pluginData->data->publishers && !pluginData->data->publishers->empty()) {
				const auto& publishers = pluginData->data->publishers.value();
				DLOG("Got a list of available publishers/feeds:");
				for (const auto& pub : publishers) {
					DLOG("  >> [{}] {})", pub.id.value(), pub.display.value_or(""));
					auto participant = std::make_shared<Participant>(pub.id.value(), pub.display.value_or(""));
					createParticipant(participant);
				}
				_subscriber->subscribeTo(publishers);
			}

			if (pluginData->data->leaving) {
				const auto& leaving = pluginData->data->leaving.value();

				// Figure out the participant and detach it
				removeParticipant(leaving);

				//_subscriber->unsubscribeFrom(leaving);
			}
			else if (pluginData->data->unpublished) {
				const auto& unpublished = pluginData->data->unpublished.value();
				DLOG("Publisher left: {}", unpublished);

				// TODO: |unpublished| can be int or string
				if (unpublished == 0) {
					// That's us
					this->hangup(true);
					return;
				}

				// Figure out the participant and detach it
				removeParticipant(unpublished);

				//_subscriber->unsubscribeFrom(unpublished);
			}
			else if (pluginData->data->error) {
				if (pluginData->data->error_code.value_or(0) == 426) {
					DLOG("No such room");
				}
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
			// TODO:
			//sfutest.handleRemoteJsep({ jsep: jsep });
			std::shared_ptr<PrepareWebrtcPeerEvent> event = std::make_shared<PrepareWebrtcPeerEvent>();
			auto lambda = [](bool success, const std::string& response) {
				DLOG("response: {}", response.c_str());
			};
			std::shared_ptr<vi::EventCallback> callback = std::make_shared<vi::EventCallback>(lambda);
			JsepConfig jst;
			jst.type = jsep->type.value_or("");
			jst.sdp = jsep->sdp.value_or("");
			event->jsep = jst;
			event->callback = callback;

			handleRemoteJsep(event);

			if (!_pluginContext) {
				return;
			}

			//const auto& audio = pluginData->data->audio_codec.value_or("");
			//if (_pluginContext->localStream && _pluginContext->localStream->GetAudioTracks().size() > 0 && audio.empty()) {
			//	WLOG("Our audio stream has been rejected, viewers won't hear us");
			//}

			//const auto& video = pluginData->data->video_codec.value_or("");
			//if (_pluginContext->localStream && _pluginContext->localStream->GetVideoTracks().size() > 0 && video.empty()) {
			//	WLOG("Our video stream has been rejected, viewers won't see us");
			//}
		}
	}

	void VideoRoomClient::onTimeout()
	{

	}

	void VideoRoomClient::onError(const std::string& desc)
	{

	}

	void VideoRoomClient::onLocalTrack(rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> track, bool on)
	{
		_mediaController->onLocalTrack(track, _id, on);
	}

	void VideoRoomClient::onCleanup()
	{
		PluginClient::onCleanup();
	}

	void VideoRoomClient::onDetached() {}

	void VideoRoomClient::publishStream(bool audioOn)
	{
		auto event = std::make_shared<PrepareWebrtcEvent>();
		_pluginContext->offerAnswerCallback = std::make_shared<CreateOfferAnswerCallback>([wself = weak_from_this(), audioOn](bool success, const std::string& reason, const JsepConfig& jsep) {
			auto self = wself.lock();
			if (!self) {
				return;
			}
			if (success) {
				vr::PublisherConfigureRequest request;
				request.audio = audioOn;
				request.video = true;
				auto event = std::make_shared<vi::MessageEvent>();
				auto lambda = [](bool success, const std::string& response) {
					DLOG("publishStream: {}", response.c_str());
				};
				auto callback = std::make_shared<vi::EventCallback>(lambda);
				event->message = request.toJsonStr();
				Jsep jp; 
				jp.type = jsep.type;
				jp.sdp = jsep.sdp;
				event->jsep = jp.toJsonStr();
				event->callback = callback;
				self->sendMessage(event);
			}
			else {
				DLOG("WebRTC error: {}", reason.c_str());
			}
		});
		MediaConfig media;
		media.audioRecv = true;
		media.videoRecv = true;
		media.audioSend = audioOn;
		media.videoSend = true;
		event->media = media;
		event->simulcast = true;
		event->simulcast2 = false;
		createOffer(event);
	}

	void VideoRoomClient::unpublishStream()
	{
		vr::UnpublishRequest request;
		if (auto webrtcService = _pluginContext->signalingService.lock()) {
			auto event = std::make_shared<vi::MessageEvent>();
			auto lambda = [](bool success, const std::string& response) {
				DLOG("response: {}", response.c_str());
			};
			auto callback = std::make_shared<vi::EventCallback>(lambda);
			event->message = request.toJsonStr();
			event->callback = callback;
			sendMessage(event);
		}
	}

	void VideoRoomClient::createParticipant(std::shared_ptr<Participant> participant)
	{
		if (_participantsController) {
			_participantsController->createParticipant(participant);
		}
	}

	void VideoRoomClient::removeParticipant(int64_t id)
	{
		if (_participantsController) {
			_participantsController->removeParticipant(id);
		}
	}

	//int32_t SignalingService::getVolume(int64_t handleId, bool isRemote, const std::string& mid)
//{
//	const auto& pluginClient = getHandler(handleId);
//	if (!pluginClient) {
//		DLOG("Invalid handle");
//	}

//	const auto& context = pluginClient->pluginContext()->webrtcContext;

//	return 0;
//}

//int32_t SignalingService::remoteVolume(int64_t handleId, const std::string& mid)
//{
//	return getVolume(handleId, true, mid);
//}

//int32_t SignalingService::localVolume(int64_t handleId, const std::string& mid)
//{
//	return getVolume(handleId, false, mid);
//}

//bool SignalingService::isAudioMuted(int64_t handleId, const std::string& mid)
//{
//	return isMuted(handleId, true, mid);
//}

//bool SignalingService::isVideoMuted(int64_t handleId, const std::string& mid)
//{
//	return isMuted(handleId, false, mid);
//}

//bool SignalingService::isMuted(int64_t handleId, bool isVideo, const std::string& mid)
//{
//	const auto& pluginClient = getHandler(handleId);
//	if (!pluginClient) {
//		DLOG("Invalid handle");
//		return true;
//	}

//	const auto& context = pluginClient->pluginContext()->webrtcContext;
//	if (!context) {
//		return true;
//	}
//	if (!context->pc) {
//		DLOG("Invalid PeerConnection");
//		return true;
//	}
//	if (!context->localStream) {
//		DLOG("Invalid local MediaStream");
//		return true;
//	}
//	if (isVideo) {
//		// Check video track
//		if (context->localStream->GetVideoTracks().size() == 0) {
//			DLOG("No video track");
//			return true;
//		}
//		if (!mid.empty() && _pluginContext->unifiedPlan) {
//			std::vector<rtc::scoped_refptr<webrtc::RtpTransceiverInterface>> transceivers = context->pc->GetTransceivers();
//			std::vector<rtc::scoped_refptr<webrtc::RtpTransceiverInterface>>::iterator it = std::find_if(transceivers.begin(), transceivers.end(),
//				[mid](const rtc::scoped_refptr<webrtc::RtpTransceiverInterface>& transceiver) {
//				return transceiver->mid().value_or("") == mid && transceiver->media_type() == cricket::MediaType::MEDIA_TYPE_VIDEO;
//			});
//			if (it == transceivers.end()) {
//				DLOG("No video transceiver with mid: {}", mid);
//				return true;
//			}
//			if (!(*it)->sender() || !(*it)->sender()->track()) {
//				DLOG("No video sender with mid: {}", mid);
//				return true;
//			}

//			return (*it)->sender()->track()->enabled();
//		}
//		else {
//			return !context->localStream->GetVideoTracks()[0]->enabled();
//		}
//	}
//	else {
//		// Check audio track
//		if (context->localStream->GetAudioTracks().size() == 0) {
//			DLOG("No audio track");
//			return true;
//		}
//		if (!mid.empty() && _pluginContext->unifiedPlan) {
//			std::vector<rtc::scoped_refptr<webrtc::RtpTransceiverInterface>> transceivers = context->pc->GetTransceivers();
//			std::vector<rtc::scoped_refptr<webrtc::RtpTransceiverInterface>>::iterator it = std::find_if(transceivers.begin(), transceivers.end(),
//				[mid](const rtc::scoped_refptr<webrtc::RtpTransceiverInterface>& transceiver) {
//				return transceiver->mid().value_or("") == mid && transceiver->media_type() == cricket::MediaType::MEDIA_TYPE_AUDIO;
//			});
//			if (it == transceivers.end()) {
//				DLOG("No audio transceiver with mid: {}", mid);
//				return true;
//			}
//			if (!(*it)->sender() || !(*it)->sender()->track()) {
//				DLOG("No audio sender with mid: {}", mid);
//				return true;
//			}

//			return (*it)->sender()->track()->enabled();
//		}
//		else {
//			return !context->localStream->GetAudioTracks()[0]->enabled();
//		}
//	}
//	return true;
//}

//bool SignalingService::muteAudio(int64_t handleId, const std::string& mid)
//{
//	return mute(handleId, false, true, mid);
//}

//bool SignalingService::muteVideo(int64_t handleId, const std::string& mid)
//{
//	return mute(handleId, true, true, mid);
//}

//bool SignalingService::unmuteAudio(int64_t handleId, const std::string& mid)
//{
//	return mute(handleId, false, false, mid);
//}

//bool SignalingService::unmuteVideo(int64_t handleId, const std::string& mid)
//{
//	return mute(handleId, true, false, mid);
//}

//bool SignalingService::mute(int64_t handleId, bool isVideo, bool mute, const std::string& mid)
//{
//	const auto& pluginClient = getHandler(handleId);
//	if (!pluginClient) {
//		DLOG("Invalid handle");
//		return false;
//	}

//	const auto& context = pluginClient->pluginContext()->webrtcContext;
//	if (!context) {
//		return false;
//	}
//	if (!context->pc) {
//		DLOG("Invalid PeerConnection");
//		return false;
//	}
//	if (!context->localStream) {
//		DLOG("Invalid local MediaStream");
//		return false;
//	}

//	bool enabled = mute ? false : true;

//	if (isVideo) {
//		// Mute/unmute video track
//		if (context->localStream->GetVideoTracks().size() == 0) {
//			DLOG("No video track");
//			return false;
//		}
//		if (!mid.empty() && _pluginContext->unifiedPlan) {
//			std::vector<rtc::scoped_refptr<webrtc::RtpTransceiverInterface>> transceivers = context->pc->GetTransceivers();
//			std::vector<rtc::scoped_refptr<webrtc::RtpTransceiverInterface>>::iterator it = std::find_if(transceivers.begin(), transceivers.end(),
//				[mid](const rtc::scoped_refptr<webrtc::RtpTransceiverInterface>& transceiver) {
//				return transceiver->mid().value_or("") == mid && transceiver->media_type() == cricket::MediaType::MEDIA_TYPE_VIDEO;
//			});
//			if (it == transceivers.end()) {
//				DLOG("No video transceiver with mid: {}", mid);
//				return true;
//			}
//			if (!(*it)->sender() || !(*it)->sender()->track()) {
//				DLOG("No video sender with mid: {}", mid);
//				return true;
//			}
//			return (*it)->sender()->track()->set_enabled(enabled);
//		}
//		else {
//			return context->localStream->GetVideoTracks()[0]->set_enabled(enabled);
//		}
//	}
//	else {
//		// Mute/unmute audio track
//		if (context->localStream->GetAudioTracks().size() == 0) {
//			DLOG("No audio track");
//			return false;
//		}
//		if (!mid.empty() && _pluginContext->unifiedPlan) {
//			std::vector<rtc::scoped_refptr<webrtc::RtpTransceiverInterface>> transceivers = context->pc->GetTransceivers();
//			std::vector<rtc::scoped_refptr<webrtc::RtpTransceiverInterface>>::iterator it = std::find_if(transceivers.begin(), transceivers.end(),
//				[mid](const rtc::scoped_refptr<webrtc::RtpTransceiverInterface>& transceiver) {
//				return transceiver->mid().value_or("") == mid && transceiver->media_type() == cricket::MediaType::MEDIA_TYPE_AUDIO;
//			});
//			if (it == transceivers.end()) {
//				DLOG("No audio transceiver with mid: {}", mid);
//				return true;
//			}
//			if (!(*it)->sender() || !(*it)->sender()->track()) {
//				DLOG("No audio sender with mid: {}", mid);
//				return true;
//			}
//			return (*it)->sender()->track()->set_enabled(enabled);
//		}
//		else {
//			return context->localStream->GetAudioTracks()[0]->set_enabled(enabled);
//		}
//	}
//	return false;
//}

//std::string SignalingService::getBitrate(int64_t handleId, const std::string& mid)
//{
//	return "";
//}
}
