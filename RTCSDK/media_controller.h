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

    class MediaController
        : public MediaControllerInterface
        , public UniversalObservable<IMediaControlEventHandler>
        , public std::enable_shared_from_this<MediaController>
    {
    public:
        MediaController();

        ~MediaController();

        void init() override;

        void destroy() override;

		void registerEventHandler(std::shared_ptr<IMediaControlEventHandler> handler) override;

		void unregisterEventHandler(std::shared_ptr<IMediaControlEventHandler> handler) override;

		int32_t remoteVolume(const std::string& pid) override;

		int32_t localVolume(const std::string& pid) override;

		void muteAudio(const std::string& pid) override;

		void unmuteAudio(const std::string& pid) override;

		bool isAudioMuted(const std::string& pid) override;

		void muteVideo(const std::string& pid) override;

		void unmuteVideo(const std::string& pid) override;

		bool isVideoMuted(const std::string& pid) override;

        std::map<std::string, rtc::scoped_refptr<webrtc::MediaStreamInterface>>& localStream();

        std::map<std::string, rtc::scoped_refptr<webrtc::MediaStreamInterface>>& remoteStreams();

        void onWebrtcStatus(bool isActive, const std::string& reason);

        void onLocalTrack(rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> track, int64_t mid, bool on);

        void onRemoteTrack(rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> track, const std::string& mid, bool on);

    private:

        // key: trackId
        std::map<std::string, rtc::scoped_refptr<webrtc::MediaStreamInterface>> _localStreams;

        // key: mid
        std::map<std::string, rtc::scoped_refptr<webrtc::MediaStreamInterface>> _remoteStreams;
    };

}

