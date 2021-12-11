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


class VideoRoomEventProxy 
	: public QObject
	, public vi::IVideoRoomEventHandler
	, public std::enable_shared_from_this<VideoRoomEventProxy>
{
	Q_OBJECT

public:
	VideoRoomEventProxy(QObject *parent);
	~VideoRoomEventProxy();

private:
	// IVideoRoomEventHandler
	void onMediaStatus(bool isActive, const std::string& reason) override;

	void onCreateParticipant(std::shared_ptr<vi::Participant> participant) override;

	void onUpdateParticipant(std::shared_ptr<vi::Participant> participant) override;

	void onRemoveParticipant(std::shared_ptr<vi::Participant> participant) override;

	void onCreateVideoTrack(uint64_t pid, rtc::scoped_refptr<webrtc::VideoTrackInterface> track) override;

	void onRemoveVideoTrack(uint64_t pid, rtc::scoped_refptr<webrtc::VideoTrackInterface> track) override;

signals:
	void mediaState(bool isActive, const std::string& reason);

	void createParticipant(std::shared_ptr<vi::Participant> participant);

	void updateParticipant(std::shared_ptr<vi::Participant> participant);

	void removeParticipant(std::shared_ptr<vi::Participant> participant);

	void createVideoTrack(uint64_t pid, rtc::scoped_refptr<webrtc::VideoTrackInterface> track);

	void removeVideoTrack(uint64_t pid, rtc::scoped_refptr<webrtc::VideoTrackInterface> track);
};
