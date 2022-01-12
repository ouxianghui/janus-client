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

		virtual void muteLocalAudio(bool mute) = 0;

		virtual bool isLocalAudioMuted() = 0;

		virtual void muteLocalVideo(bool mute) = 0;

		virtual bool isLocalVideoMuted() = 0;

		virtual void muteAudio(int64_t pid, const std::string& mid, bool mute) = 0;

		virtual bool isAudioMuted(int64_t pid) = 0;

		virtual void muteVideo(int64_t pid, const std::string& mid, bool mute) = 0;

		virtual bool isVideoMuted(int64_t pid) = 0;
    };

	BEGIN_WEAK_PROXY_MAP(MediaController)
		WEAK_PROXY_THREAD_DESTRUCTOR()
		WEAK_PROXY_METHOD0(void, init)
		WEAK_PROXY_METHOD0(void, destroy)
		WEAK_PROXY_METHOD1(void, registerEventHandler, std::shared_ptr<IMediaControlEventHandler>)
		WEAK_PROXY_METHOD1(void, unregisterEventHandler, std::shared_ptr<IMediaControlEventHandler>)
		WEAK_PROXY_METHOD1(void, muteLocalAudio, bool)
		WEAK_PROXY_METHOD0(bool, isLocalAudioMuted)
		WEAK_PROXY_METHOD1(void, muteLocalVideo, bool)
		WEAK_PROXY_METHOD0(bool, isLocalVideoMuted)
		WEAK_PROXY_METHOD3(void, muteAudio, int64_t, const std::string&, bool)
		WEAK_PROXY_METHOD1(bool, isAudioMuted, int64_t)
		WEAK_PROXY_METHOD3(void, muteVideo, int64_t, const std::string&, bool)
		WEAK_PROXY_METHOD1(bool, isVideoMuted, int64_t)
	END_WEAK_PROXY_MAP()
}