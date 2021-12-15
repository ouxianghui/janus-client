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

	void onCreate() override;

	void onJoin() override;

	void onLeave() override;

signals:
	void create();

	void join();

	void leave();
};
