#include "media_controller.h"
#include "api/media_stream_interface.h"
#include "pc/media_stream.h"
#include "pc/media_stream_proxy.h"
#include "video_room_client.h"
#include "video_room_api.h"
#include "logger/logger.h"

namespace vi {
	MediaController::MediaController(std::shared_ptr<VideoRoomClient> vrc)
		: _vrc(vrc)
	{

	}

	MediaController::~MediaController()
	{

	}

	void MediaController::init() 
	{

	}

	void MediaController::destroy() 
	{

	}

	void MediaController::registerEventHandler(std::shared_ptr<IMediaControlEventHandler> handler) 
	{
		UniversalObservable<IMediaControlEventHandler>::addWeakObserver(handler, std::string("main"));
	}

	void MediaController::unregisterEventHandler(std::shared_ptr<IMediaControlEventHandler> handler) 
	{
		UniversalObservable<IMediaControlEventHandler>::removeObserver(handler);
	}

	void MediaController::muteLocalAudio(bool mute)
	{
		muteLocal(false, mute);
	}

	bool MediaController::isLocalAudioMuted()
	{
		return isLocalMuted(false);
	}

	void MediaController::muteLocalVideo(bool mute)
	{
		muteLocal(true, mute);
	}

	bool MediaController::isLocalVideoMuted()
	{
		return isLocalMuted(true);
	}

	void MediaController::muteAudio(int64_t pid, const std::string& mid, bool mute)
	{
		auto vrc = _vrc.lock();
		if (!vrc) {
			DLOG("Invalid video room client instance");
			return;
		}

		auto req = std::make_shared<vr::ModerateRequest>();
		req->room = vrc->roomId();
		req->secret = "adminpwd";
		req->id = pid;
		req->mid = mid;
		req->mute = mute;
		DLOG("mute audio, pid: {}, mid: {}, mute: {}", pid, mid, mute ? "yes" : "no");
		//vrc->videoRoomApi()->moderate(req, [](std::shared_ptr<vr::ModerateResponse> resp) {
		//	DLOG("mute audio, resq: {}", resp->toJsonStr());
		//});
	}

	bool MediaController::isAudioMuted(int64_t pid)
	{
		return false;
	}

	void MediaController::muteVideo(int64_t pid, const std::string& mid, bool mute)
	{
		auto vrc = _vrc.lock();
		if (!vrc) {
			DLOG("Invalid video room client instance");
			return;
		}

		auto req = std::make_shared<vr::ModerateRequest>();
		req->room = vrc->roomId();
		req->secret = "adminpwd";
		req->id = pid;
		req->mid = mid;
		req->mute = mute;

		DLOG("mute video, pid: {}, mid: {}, mute: {}", pid, mid, mute ? "yes" : "no");
		//vrc->videoRoomApi()->moderate(req, [](std::shared_ptr<vr::ModerateResponse> resp) {
		//	DLOG("mute video, resq: {}", resp->toJsonStr());
		//});
	}

	bool MediaController::isVideoMuted(int64_t pid)
	{
		return false;
	}

	void MediaController::onWebrtcStatus(bool isActive, const std::string& reason)
	{
		UniversalObservable<IMediaControlEventHandler>::notifyObservers([isActive, reason](const auto& observer) {
			observer->onMediaStatus(isActive, reason);
		});
	}

	void MediaController::onLocalTrack(rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> track, int64_t mid, bool on)
	{
		if (on) {
			UniversalObservable<IMediaControlEventHandler>::notifyObservers([wself = weak_from_this(), mid, track](const auto& observer) {
				auto self = wself.lock();
				if (!self) {
					return;
				}
				if (!track) {
					return;
				}

				if (track->kind() == webrtc::MediaStreamTrackInterface::kVideoKind) {
					observer->onCreateVideoTrack(mid, dynamic_cast<webrtc::VideoTrackInterface*>(track.get()));
				}
			});

		}
		else {
			UniversalObservable<IMediaControlEventHandler>::notifyObservers([wself = weak_from_this(), mid, track](const auto& observer) {
				auto self = wself.lock();
				if (!self) {
					return;
				}
				if (!track) {
					return;
				}

				if (track->kind() == webrtc::MediaStreamTrackInterface::kVideoKind) {
					observer->onRemoveVideoTrack(mid, dynamic_cast<webrtc::VideoTrackInterface*>(track.get()));
				}
			});
		}
	}

	void MediaController::onRemoteTrack(rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> track, const std::string& mid, bool on)
	{
		if (on) {
			UniversalObservable<IMediaControlEventHandler>::notifyObservers([wself = weak_from_this(), pid = std::stoul(mid.c_str()), track](const auto& observer) {
				auto self = wself.lock();
				if (!self) {
					return;
				}
				if (!track) {
					return;
				}

				if (track->kind() == webrtc::MediaStreamTrackInterface::kVideoKind) {
					observer->onCreateVideoTrack(pid, dynamic_cast<webrtc::VideoTrackInterface*>(track.get()));
				}
			});
		}
		else {
			UniversalObservable<IMediaControlEventHandler>::notifyObservers([wself = weak_from_this(), pid = std::stoul(mid.c_str()), track](const auto& observer) {
				auto self = wself.lock();
				if (!self) {
					return;
				}
				if (!track) {
					return;
				}

				if (track->kind() == webrtc::MediaStreamTrackInterface::kVideoKind) {
					observer->onRemoveVideoTrack(pid, dynamic_cast<webrtc::VideoTrackInterface*>(track.get()));
				}
			});
		}
	}

	bool MediaController::isLocalMuted(bool isVideo)
	{
		auto vrc = _vrc.lock();
		if (!vrc) {
			DLOG("Invalid vrc");
			return true;
		}

		const auto& context = vrc->pluginContext();
		if (!context) {
			return true;
		}

		if (!context->pc) {
			DLOG("Invalid PeerConnection");
			return true;
		}

		if (!context->localStream) {
			DLOG("Invalid local MediaStream");
			return true;
		}

		if (isVideo) {
			// Check video track
			if (context->localStream->GetVideoTracks().size() == 0) {
				DLOG("No video track");
				return true;
			}

			return !context->localStream->GetVideoTracks()[0]->enabled();
		}
		else {
			// Check audio track
			if (context->localStream->GetAudioTracks().size() == 0) {
				DLOG("No audio track");
				return true;
			}

			return !context->localStream->GetAudioTracks()[0]->enabled();
		}

		return true;
	}

	bool MediaController::muteLocal(bool isVideo, bool mute)
	{
		auto vrc = _vrc.lock();
		if (!vrc) {
			DLOG("Invalid vrc");
			return true;
		}

		const auto& context = vrc->pluginContext();
		if (!context) {
			return false;
		}

		if (!context->pc) {
			DLOG("Invalid PeerConnection");
			return false;
		}

		if (!context->localStream) {
			DLOG("Invalid local MediaStream");
			return false;
		}

		bool enabled = mute ? false : true;

		if (isVideo) {
			// Mute/unmute video track
			if (context->localStream->GetVideoTracks().size() == 0) {
				DLOG("No video track");
				return false;
			}

			return context->localStream->GetVideoTracks()[0]->set_enabled(enabled);
		}
		else {
			// Mute/unmute audio track
			if (context->localStream->GetAudioTracks().size() == 0) {
				DLOG("No audio track");
				return false;
			}

			return context->localStream->GetAudioTracks()[0]->set_enabled(enabled);
		}

		return false;
	}
}

//if (!mid.empty() && _pluginContext->unifiedPlan) {
//	std::vector<rtc::scoped_refptr<webrtc::RtpTransceiverInterface>> transceivers = context->pc->GetTransceivers();
//	std::vector<rtc::scoped_refptr<webrtc::RtpTransceiverInterface>>::iterator it = std::find_if(transceivers.begin(), transceivers.end(),
//		[mid](const rtc::scoped_refptr<webrtc::RtpTransceiverInterface>& transceiver) {
//			return transceiver->mid().value_or("") == mid && transceiver->media_type() == cricket::MediaType::MEDIA_TYPE_VIDEO;
//		});
//	if (it == transceivers.end()) {
//		DLOG("No video transceiver with mid: {}", mid);
//		return true;
//	}
//	if (!(*it)->sender() || !(*it)->sender()->track()) {
//		DLOG("No video sender with mid: {}", mid);
//		return true;
//	}
//
//	return (*it)->sender()->track()->enabled();
//}

//if (!mid.empty() && _pluginContext->unifiedPlan) {
//	std::vector<rtc::scoped_refptr<webrtc::RtpTransceiverInterface>> transceivers = context->pc->GetTransceivers();
//	std::vector<rtc::scoped_refptr<webrtc::RtpTransceiverInterface>>::iterator it = std::find_if(transceivers.begin(), transceivers.end(),
//		[mid](const rtc::scoped_refptr<webrtc::RtpTransceiverInterface>& transceiver) {
//			return transceiver->mid().value_or("") == mid && transceiver->media_type() == cricket::MediaType::MEDIA_TYPE_VIDEO;
//		});
//	if (it == transceivers.end()) {
//		DLOG("No video transceiver with mid: {}", mid);
//		return true;
//	}
//	if (!(*it)->sender() || !(*it)->sender()->track()) {
//		DLOG("No video sender with mid: {}", mid);
//		return true;
//	}
//
//	return (*it)->sender()->track()->enabled();
//}
//else {
//	return !context->localStream->GetVideoTracks()[0]->enabled();
//}


//if (!mid.empty() && _pluginContext->unifiedPlan) {
//	std::vector<rtc::scoped_refptr<webrtc::RtpTransceiverInterface>> transceivers = context->pc->GetTransceivers();
//	std::vector<rtc::scoped_refptr<webrtc::RtpTransceiverInterface>>::iterator it = std::find_if(transceivers.begin(), transceivers.end(),
//		[mid](const rtc::scoped_refptr<webrtc::RtpTransceiverInterface>& transceiver) {
//			return transceiver->mid().value_or("") == mid && transceiver->media_type() == cricket::MediaType::MEDIA_TYPE_VIDEO;
//		});
//	if (it == transceivers.end()) {
//		DLOG("No video transceiver with mid: {}", mid);
//		return true;
//	}
//	if (!(*it)->sender() || !(*it)->sender()->track()) {
//		DLOG("No video sender with mid: {}", mid);
//		return true;
//	}
//	return (*it)->sender()->track()->set_enabled(enabled);
//}


//if (!mid.empty() && _pluginContext->unifiedPlan) {
//	std::vector<rtc::scoped_refptr<webrtc::RtpTransceiverInterface>> transceivers = context->pc->GetTransceivers();
//	std::vector<rtc::scoped_refptr<webrtc::RtpTransceiverInterface>>::iterator it = std::find_if(transceivers.begin(), transceivers.end(),
//		[mid](const rtc::scoped_refptr<webrtc::RtpTransceiverInterface>& transceiver) {
//			return transceiver->mid().value_or("") == mid && transceiver->media_type() == cricket::MediaType::MEDIA_TYPE_AUDIO;
//		});
//	if (it == transceivers.end()) {
//		DLOG("No audio transceiver with mid: {}", mid);
//		return true;
//	}
//	if (!(*it)->sender() || !(*it)->sender()->track()) {
//		DLOG("No audio sender with mid: {}", mid);
//		return true;
//	}
//	return (*it)->sender()->track()->set_enabled(enabled);
//}