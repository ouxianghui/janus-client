#include "media_event_proxy.h"

MediaEventProxy::MediaEventProxy(QObject* parent) : QObject(parent)
{

}

MediaEventProxy::~MediaEventProxy()
{

}

void MediaEventProxy::onMediaStatus(bool isActive, const std::string& reason)
{
	emit mediaStatus(isActive, reason);
}

void MediaEventProxy::onCreateVideoTrack(uint64_t pid, rtc::scoped_refptr<webrtc::VideoTrackInterface> track)
{
	emit createVideoTrack(pid, track);
}

void MediaEventProxy::onRemoveVideoTrack(uint64_t pid, rtc::scoped_refptr<webrtc::VideoTrackInterface> track)
{
	emit removeVideoTrack(pid, track);
}

void MediaEventProxy::onLocalAudioMuted(bool muted)
{
	emit localAudioMuted(muted);
}

void MediaEventProxy::onLocalVideoMuted(bool muted)
{
	emit localVideoMuted(muted);
}

void MediaEventProxy::onRemoteAudioMuted(const std::string& pid, bool muted)
{
	emit remoteAudioMuted(pid, muted);
}

void MediaEventProxy::onRemoteVideoMuted(const std::string& pid, bool muted)
{
	emit remoteVideoMuted(pid, muted);
}