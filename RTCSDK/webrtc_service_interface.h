/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#pragma once

#include <memory>
#include <functional>
#include "webrtc_service_events.h"
#include "Service/abstract_biz_service.h"
#include "Service/i_unified_factory.h"
#include "i_webrtc_service_listener.h"

namespace vi {
	class PluginClient;
	using namespace core;
	class WebRTCServiceInterface
	{
	public:
		virtual ~WebRTCServiceInterface() {}

		virtual void init() = 0;

		virtual void cleanup() = 0;

		virtual void addListener(std::shared_ptr<IWebRTCServiceListener> listener) = 0;

		virtual void removeListener(std::shared_ptr<IWebRTCServiceListener> listener) = 0;

		virtual void connect(const std::string& url) = 0;

		virtual ServiceStauts status() = 0;

		virtual void attach(const std::string& plugin, const std::string& opaqueId, std::shared_ptr<PluginClient> pluginClient) = 0;

		virtual void destroy(std::shared_ptr<DestroySessionEvent> event) = 0;

		virtual int32_t remoteVolume(int64_t handleId, const std::string& mid) = 0;

		virtual int32_t localVolume(int64_t handleId, const std::string& mid) = 0;

		virtual bool isAudioMuted(int64_t handleId, const std::string& mid) = 0;

		virtual bool isVideoMuted(int64_t handleId, const std::string& mid) = 0;

		virtual bool muteAudio(int64_t handleId, const std::string& mid) = 0;

		virtual bool muteVideo(int64_t handleId, const std::string& mid) = 0;

		virtual bool unmuteAudio(int64_t handleId, const std::string& mid) = 0;

		virtual bool unmuteVideo(int64_t handleId, const std::string& mid) = 0;

		virtual std::string getBitrate(int64_t handleId, const std::string& mid) = 0;

		virtual void sendMessage(int64_t handleId, std::shared_ptr<SendMessageEvent> event) = 0;

		virtual void sendData(int64_t handleId, std::shared_ptr<SendDataEvent> event) = 0;

		virtual void sendDtmf(int64_t handleId, std::shared_ptr<SendDtmfEvent> event) = 0;

		virtual void createOffer(int64_t handleId, std::shared_ptr<PrepareWebRTCEvent> event) = 0;

		virtual void createAnswer(int64_t handleId, std::shared_ptr<PrepareWebRTCEvent> event) = 0;

		virtual void handleRemoteJsep(int64_t handleId, std::shared_ptr<PrepareWebRTCPeerEvent> event) = 0;

		virtual void hangup(int64_t handleId, bool sendRequest) = 0;

		virtual void detach(int64_t handleId, std::shared_ptr<DetachEvent> event) = 0;
	};
}