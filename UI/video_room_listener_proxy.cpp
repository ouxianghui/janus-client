/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-18
 **/

#include "video_room_listener_proxy.h"

VideoRoomListenerProxy::VideoRoomListenerProxy(QObject *parent)
	: QObject(parent)
{
}

VideoRoomListenerProxy::~VideoRoomListenerProxy()
{
}

// IVideoRoomListener
void VideoRoomListenerProxy::onMediaState(bool isActive, const std::string& reason)
{
	emit mediaState(isActive, reason);
}

void VideoRoomListenerProxy::onCreateParticipant(std::shared_ptr<vi::Participant> participant)
{
	emit createParticipant(participant);
}

void VideoRoomListenerProxy::onUpdateParticipant(std::shared_ptr<vi::Participant> participant)
{
	emit onUpdateParticipant(participant);
}

void VideoRoomListenerProxy::onRemoveParticipant(std::shared_ptr<vi::Participant> participant)
{
	emit removeParticipant(participant);
}

void VideoRoomListenerProxy::onCreateVideoTrack(uint64_t pid, rtc::scoped_refptr<webrtc::VideoTrackInterface> track)
{
	emit createVideoTrack(pid, track);
}

void VideoRoomListenerProxy::onRemoveVideoTrack(uint64_t pid, rtc::scoped_refptr<webrtc::VideoTrackInterface> track)
{
	emit removeVideoTrack(pid, track);
}