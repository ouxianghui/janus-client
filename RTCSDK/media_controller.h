#pragma once

#include <string>
#include "media_controller_interface.h"
#include "utils/universal_observable.hpp"
#include "i_media_control_event_handler.h"

namespace webrtc {
    class MediaStreamInterface;
    class MediaStreamTrackInterface;
}

namespace vi {
    class VideoRoomClient;
    class IVideoRoomApi;

    class MediaController
        : public MediaControllerInterface
        , public UniversalObservable<IMediaControlEventHandler>
        , public std::enable_shared_from_this<MediaController>
    {
    public:
        MediaController(std::shared_ptr<VideoRoomClient> vrc);

        ~MediaController();

        void init() override;

        void destroy() override;

		void registerEventHandler(std::shared_ptr<IMediaControlEventHandler> handler) override;

		void unregisterEventHandler(std::shared_ptr<IMediaControlEventHandler> handler) override;

        void muteLocalAudio(bool mute) override;

        bool isLocalAudioMuted() override;

        void muteLocalVideo(bool mute) override;

        bool isLocalVideoMuted() override;

        void muteAudio(int64_t pid, const std::string& mid, bool mute) override;

        bool isAudioMuted(int64_t pid) override;

        void muteVideo(int64_t pid, const std::string& mid, bool mute) override;

        bool isVideoMuted(int64_t pid) override;

        void onWebrtcStatus(bool isActive, const std::string& reason);

        void onLocalTrack(rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> track, int64_t mid, bool on);

        void onRemoteTrack(rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> track, const std::string& mid, bool on);

    private:
        bool isLocalMuted(bool isVideo);

        bool muteLocal(bool isVideo, bool mute);

    private:

        std::weak_ptr<VideoRoomClient> _vrc;
    };

}

