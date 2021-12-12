#pragma once


#include "weak_proxy.h"

namespace vi {

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
