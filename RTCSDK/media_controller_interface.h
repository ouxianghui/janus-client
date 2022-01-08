#pragma once

#include <memory>
#include <string>
#include "weak_proxy.h"

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

	BEGIN_WEAK_PROXY_MAP(MediaController)
		WEAK_PROXY_THREAD_DESTRUCTOR()
		WEAK_PROXY_METHOD0(void, init)
		WEAK_PROXY_METHOD0(void, destroy)
		WEAK_PROXY_METHOD1(void, registerEventHandler, std::shared_ptr<IMediaControlEventHandler>)
		WEAK_PROXY_METHOD1(void, unregisterEventHandler, std::shared_ptr<IMediaControlEventHandler>)
		WEAK_PROXY_METHOD1(int32_t, remoteVolume, const std::string&)
		WEAK_PROXY_METHOD1(int32_t, localVolume, const std::string&)
		WEAK_PROXY_METHOD1(void, muteAudio, const std::string&)
		WEAK_PROXY_METHOD1(void, unmuteAudio, const std::string&)
		WEAK_PROXY_METHOD1(bool, isAudioMuted, const std::string&)
		WEAK_PROXY_METHOD1(void, muteVideo, const std::string&)
		WEAK_PROXY_METHOD1(void, unmuteVideo, const std::string&)
		WEAK_PROXY_METHOD1(bool, isVideoMuted, const std::string&)
	END_WEAK_PROXY_MAP()
}