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

void VideoRoomEventAdapter::onCreate()
{
	emit create();
}

void VideoRoomEventAdapter::onJoin()
{
	emit join();
}

void VideoRoomEventAdapter::onLeave()
{
	emit leave();
}