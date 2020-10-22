/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#pragma once

#include <memory>
#include <string>
#include "i_plugin_client.h"
#include "i_webrtc_event_handler.h"

namespace vi {
	class WebRTCServiceInterface;
	class PluginClient
		: public IPluginClient
		, public IWebRTCEventHandler
		, public std::enable_shared_from_this<PluginClient>
	{
	public:
		PluginClient(std::shared_ptr<WebRTCServiceInterface> wrs);

		~PluginClient();

		// IPluginClient
		void setHandleId(int64_t handleId) override;

		const std::shared_ptr<PluginContext>& pluginContext() const override;

		void attach() override;

		int32_t remoteVolume() override;

		int32_t localVolume() override;

		bool isAudioMuted() override;

		bool isVideoMuted() override;

		bool muteAudio() override;

		bool muteVideo() override;

		bool unmuteAudio() override;

		bool unmuteVideo() override;

		std::string getBitrate() override;

		void sendMessage(std::shared_ptr<SendMessageEvent> event) override;

		void sendData(std::shared_ptr<SendDataEvent> event) override;

		void sendDtmf(std::shared_ptr<SendDtmfEvent> event) override;

		void createOffer(std::shared_ptr<PrepareWebRTCEvent> event) override;

		void createAnswer(std::shared_ptr<PrepareWebRTCEvent> event) override;

		void handleRemoteJsep(std::shared_ptr<PrepareWebRTCPeerEvent> event) override;

		void hangup(bool sendRequest) override;

		void detach(std::shared_ptr<DetachEvent> event) override;

	protected:
		uint64_t _id;
		uint64_t _privateId;
		std::shared_ptr<PluginContext> _pluginContext;
	};
}

