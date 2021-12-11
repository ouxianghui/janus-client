/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-18
 **/

#include "video_room_event_proxy.h"

VideoRoomEventProxy::VideoRoomEventProxy(QObject *parent)
	: QObject(parent)
{
}

VideoRoomEventProxy::~VideoRoomEventProxy()
{
}

// IVideoRoomEventHandler
void VideoRoomEventProxy::onMediaStatus(bool isActive, const std::string& reason)
{
	emit mediaState(isActive, reason);
}

void VideoRoomEventProxy::onCreateParticipant(std::shared_ptr<vi::Participant> participant)
{
	emit createParticipant(participant);
}

void VideoRoomEventProxy::onUpdateParticipant(std::shared_ptr<vi::Participant> participant)
{
	emit onUpdateParticipant(participant);
}

void VideoRoomEventProxy::onRemoveParticipant(std::shared_ptr<vi::Participant> participant)
{
	emit removeParticipant(participant);
}

void VideoRoomEventProxy::onCreateVideoTrack(uint64_t pid, rtc::scoped_refptr<webrtc::VideoTrackInterface> track)
{
	emit createVideoTrack(pid, track);
}

void VideoRoomEventProxy::onRemoveVideoTrack(uint64_t pid, rtc::scoped_refptr<webrtc::VideoTrackInterface> track)
{
	emit removeVideoTrack(pid, track);
}