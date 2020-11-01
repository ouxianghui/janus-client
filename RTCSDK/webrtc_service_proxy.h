/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#pragma once

//#include "webrtc_service.h"
#include "weak_proxy.h"

namespace vi {
	BEGIN_WEAK_PROXY_MAP(WebRTCService)
		WEAK_PROXY_THREAD_DESTRUCTOR()
		WEAK_PROXY_METHOD0(void, init)
		WEAK_PROXY_METHOD0(void, cleanup)
		WEAK_PROXY_METHOD1(void, addListener, std::shared_ptr<IWebRTCServiceListener>)
		WEAK_PROXY_METHOD1(void, removeListener, std::shared_ptr<IWebRTCServiceListener>)
		WEAK_PROXY_METHOD1(void, connect, const std::string&)
		WEAK_PROXY_METHOD0(ServiceStauts, status)
		WEAK_PROXY_METHOD3(void, attach, const std::string&, const std::string&, std::shared_ptr<PluginClient>)
		WEAK_PROXY_METHOD1(void, destroy, std::shared_ptr<DestroySessionEvent>)
		WEAK_PROXY_METHOD1(int32_t, remoteVolume, int64_t)
		WEAK_PROXY_METHOD1(int32_t, localVolume, int64_t)
		WEAK_PROXY_METHOD1(bool, isAudioMuted, int64_t)
		WEAK_PROXY_METHOD1(bool, isVideoMuted, int64_t)
		WEAK_PROXY_METHOD1(bool, muteAudio, int64_t)
		WEAK_PROXY_METHOD1(bool, muteVideo, int64_t)
		WEAK_PROXY_METHOD1(bool, unmuteAudio, int64_t)
		WEAK_PROXY_METHOD1(bool, unmuteVideo, int64_t)
		WEAK_PROXY_METHOD1(std::string, getBitrate, int64_t)
		WEAK_PROXY_METHOD2(void, sendMessage, int64_t, std::shared_ptr<SendMessageEvent>)
		WEAK_PROXY_METHOD2(void, sendData, int64_t, std::shared_ptr<SendDataEvent>)
		WEAK_PROXY_METHOD2(void, sendDtmf, int64_t, std::shared_ptr<SendDtmfEvent>)
		WEAK_PROXY_METHOD2(void, createOffer, int64_t, std::shared_ptr<PrepareWebRTCEvent>)
		WEAK_PROXY_METHOD2(void, createAnswer, int64_t, std::shared_ptr<PrepareWebRTCEvent>)
		WEAK_PROXY_METHOD2(void, handleRemoteJsep, int64_t, std::shared_ptr<PrepareWebRTCPeerEvent>)
		WEAK_PROXY_METHOD2(void, hangup, int64_t, bool)
		WEAK_PROXY_METHOD2(void, detach, int64_t, std::shared_ptr<DetachEvent>)
	END_WEAK_PROXY_MAP()
}