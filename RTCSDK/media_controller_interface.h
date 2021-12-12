#pragma once

#include <memory>
#include <string>

namespace vi {

	class IMediaControlEventHandler;

    class MediaControllerInterface 
    {
    public:
        virtual ~MediaControllerInterface() = default;

		virtual void init() = 0;

		virtual void destroy() = 0;

		virtual void registerEventHandler(std::shared_ptr<IMediaControlEventHandler> handler) = 0;

		virtual void unregisterEventHandler(std::shared_ptr<IMediaControlEventHandler> handler) = 0;

		virtual int32_t remoteVolume(const std::string& pid) = 0;

		virtual int32_t localVolume(const std::string& pid) = 0;

		virtual void muteAudio(const std::string& pid) = 0;

		virtual void unmuteAudio(const std::string& pid) = 0;

		virtual bool isAudioMuted(const std::string& pid) = 0;

		virtual void muteVideo(const std::string& pid) = 0;

		virtual void unmuteVideo(const std::string& pid) = 0;

		virtual bool isVideoMuted(const std::string& pid) = 0;
    };

}