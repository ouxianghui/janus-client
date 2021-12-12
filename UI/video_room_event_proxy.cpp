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

void VideoRoomEventProxy::onCreate()
{
	emit create();
}

void VideoRoomEventProxy::onJoin()
{
	emit join();
}

void VideoRoomEventProxy::onLeave()
{
	emit leave();
}