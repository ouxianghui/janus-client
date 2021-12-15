#include "media_event_adapter.h"

MediaEventAdapter::MediaEventAdapter(QObject* parent) : QObject(parent)
{

}

MediaEventAdapter::~MediaEventAdapter()
{

}

void MediaEventAdapter::onMediaStatus(bool isActive, const std::string& reason)
{
	emit mediaStatus(isActive, reason);
}

void MediaEventAdapter::onCreateVideoTrack(uint64_t pid, rtc::scoped_refptr<webrtc::VideoTrackInterface> track)
{
	emit createVideoTrack(pid, track);
}

void MediaEventAdapter::onRemoveVideoTrack(uint64_t pid, rtc::scoped_refptr<webrtc::VideoTrackInterface> track)
{
	emit removeVideoTrack(pid, track);
}

void MediaEventAdapter::onLocalAudioMuted(bool muted)
{
	emit localAudioMuted(muted);
}

void MediaEventAdapter::onLocalVideoMuted(bool muted)
{
	emit localVideoMuted(muted);
}

void MediaEventAdapter::onRemoteAudioMuted(const std::string& pid, bool muted)
{
	emit remoteAudioMuted(pid, muted);
}

void MediaEventAdapter::onRemoteVideoMuted(const std::string& pid, bool muted)
{
	emit remoteVideoMuted(pid, muted);
}