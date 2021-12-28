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

void VideoRoomEventAdapter::onCreateRoom(int32_t errorCode)
{
	emit createRoom(errorCode);
}

void VideoRoomEventAdapter::onJoinRoom(int32_t errorCode)
{
	emit joinRoom(errorCode);
}

void VideoRoomEventAdapter::onLeaveRoom(int32_t errorCode)
{
	emit leaveRoom(errorCode);
}