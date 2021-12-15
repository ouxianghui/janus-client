#pragma once

#include <QObject>
#include <memory>
#include "i_media_control_event_handler.h"
#include "api/media_stream_interface.h"

class MediaEventAdapter
	: public QObject
	, public vi::IMediaControlEventHandler
	, public std::enable_shared_from_this<MediaEventAdapter>
{
	Q_OBJECT

public:
	MediaEventAdapter(QObject* parent);

	~MediaEventAdapter();

	void onMediaStatus(bool isActive, const std::string& reason) override;

	void onCreateVideoTrack(uint64_t pid, rtc::scoped_refptr<webrtc::VideoTrackInterface> track) override;

	void onRemoveVideoTrack(uint64_t pid, rtc::scoped_refptr<webrtc::VideoTrackInterface> track) override;

	void onLocalAudioMuted(bool muted) override;

	void onLocalVideoMuted(bool muted) override;

	void onRemoteAudioMuted(const std::string& pid, bool muted) override;

	void onRemoteVideoMuted(const std::string& pid, bool muted) override;

signals:
	void mediaStatus(bool isActive, const std::string& reason);

	void createVideoTrack(uint64_t pid, rtc::scoped_refptr<webrtc::VideoTrackInterface> track);

	void removeVideoTrack(uint64_t pid, rtc::scoped_refptr<webrtc::VideoTrackInterface> track);

	void localAudioMuted(bool muted);

	void localVideoMuted(bool muted);

	void remoteAudioMuted(const std::string& pid, bool muted);

	void remoteVideoMuted(const std::string& pid, bool muted);
};
