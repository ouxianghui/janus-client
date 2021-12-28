#include "media_controller.h"
#include "api/media_stream_interface.h"
#include "pc/media_stream.h"
#include "pc/media_stream_proxy.h"

namespace vi {
	MediaController::MediaController()
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

	int32_t MediaController::remoteVolume(const std::string& pid) 
	{
		return 0;
	}

	int32_t MediaController::localVolume(const std::string& pid) 
	{
		return 0;
	}

	void MediaController::muteAudio(const std::string& pid) 
	{

	}

	void MediaController::unmuteAudio(const std::string& pid) 
	{

	}

	bool MediaController::isAudioMuted(const std::string& pid) 
	{
		return false;
	}

	void MediaController::muteVideo(const std::string& pid) 
	{

	}

	void MediaController::unmuteVideo(const std::string& pid) 
	{

	}

	bool MediaController::isVideoMuted(const std::string& pid)
	{
		return false;
	}

	std::map<std::string, rtc::scoped_refptr<webrtc::MediaStreamInterface>>& MediaController::localStream()
	{
		return _localStreams;
	}

	std::map<std::string, rtc::scoped_refptr<webrtc::MediaStreamInterface>>& MediaController::remoteStreams()
	{
		return _remoteStreams;
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
					//self->_localStreams[track->id()] = webrtc::MediaStream::Create(track->id());// webrtc::MediaStreamProxy::Create(TMgr->thread("plugin-client"), webrtc::MediaStream::Create(track->id()));
					auto vt = dynamic_cast<webrtc::VideoTrackInterface*>(track.get());
					//self->_localStreams[track->id()]->AddTrack(vt);
					//auto t = self->_localStreams[track->id()]->GetVideoTracks()[0];
					observer->onCreateVideoTrack(mid, vt);
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
					//if (self->_localStreams.find(track->id()) != self->_localStreams.end()) {
					//	auto vt = self->_localStreams[track->id()]->GetVideoTracks()[0];
					auto vt = dynamic_cast<webrtc::VideoTrackInterface*>(track.get());
					observer->onRemoveVideoTrack(mid, vt);

						//self->_localStreams[track->id()]->RemoveTrack(vt.get());
						//auto it = self->_localStreams.find(track->id());
						//self->_localStreams.erase(it);
					//}
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
					//self->_remoteStreams[track->id()] = webrtc::MediaStream::Create(track->id());
					auto vt = dynamic_cast<webrtc::VideoTrackInterface*>(track.get());
					//self->_remoteStreams[track->id()]->AddTrack(vt);
					//auto t = self->_remoteStreams[track->id()]->GetVideoTracks()[0];
					observer->onCreateVideoTrack(pid, vt);
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
					//if (self->_remoteStreams.find(track->id()) != self->_remoteStreams.end()) {
						//auto vt = self->_remoteStreams[track->id()]->GetVideoTracks()[0];
						auto vt = dynamic_cast<webrtc::VideoTrackInterface*>(track.get());
						observer->onRemoveVideoTrack(pid, vt);

					//	self->_remoteStreams[track->id()]->RemoveTrack(vt.get());
					//	auto it = self->_remoteStreams.find(track->id());
					//	self->_remoteStreams.erase(it);
					//}
				}
			});
		}
	}
}