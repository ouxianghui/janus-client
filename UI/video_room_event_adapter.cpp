/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-18
 **/

#include "video_room_event_adapter.h"

VideoRoomEventAdapter::VideoRoomEventAdapter(QObject *parent)
	: QObject(parent)
{
}

VideoRoomEventAdapter::~VideoRoomEventAdapter()
{
}

// IVideoRoomEventHandler

void VideoRoomEventAdapter::onCreateRoom(std::shared_ptr<vi::CreateRoomResult> result, int32_t errorCode)
{
	emit createRoom(result, errorCode);
}

void VideoRoomEventAdapter::onJoinRoom(int64_t roomId, int32_t errorCode)
{
	emit joinRoom(roomId, errorCode);
}

void VideoRoomEventAdapter::onLeaveRoom(int64_t roomId, int32_t errorCode)
{
	emit leaveRoom(roomId, errorCode);
}