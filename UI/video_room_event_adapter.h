/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-18
 **/

#pragma once

#include <QObject>
#include <memory>
#include "participant.h"
#include "i_video_room_event_handler.h"


class VideoRoomEventAdapter 
	: public QObject
	, public vi::IVideoRoomEventHandler
	, public std::enable_shared_from_this<VideoRoomEventAdapter>
{
	Q_OBJECT

public:
	VideoRoomEventAdapter(QObject *parent);

	~VideoRoomEventAdapter();

private:
	// IVideoRoomEventHandler

	void onCreateRoom(int32_t errorCode) override;

	void onJoinRoom(int32_t errorCode) override;

	void onLeaveRoom(int32_t errorCode) override;

signals:
	void createRoom(int32_t errorCode);

	void joinRoom(int32_t errorCode);

	void leaveRoom(int32_t errorCode);
};
