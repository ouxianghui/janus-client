/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#pragma once

#include "webrtc_service_events.h"

namespace vi {
	class IPluginClient
	{
	public:
		virtual ~IPluginClient() {}

		virtual void attach() = 0;

		virtual int32_t remoteVolume() = 0;

		virtual int32_t localVolume() = 0;

		virtual bool isAudioMuted() = 0;

		virtual bool isVideoMuted() = 0;

		virtual bool muteAudio() = 0;

		virtual bool muteVideo() = 0;

		virtual bool unmuteAudio() = 0;

		virtual bool unmuteVideo() = 0;

		virtual std::string getBitrate() = 0;

		virtual void sendMessage(std::shared_ptr<SendMessageEvent> event) = 0;

		virtual void sendData(std::shared_ptr<SendDataEvent> event) = 0;

		virtual void sendDtmf(std::shared_ptr<SendDtmfEvent> event) = 0;

		virtual void createOffer(std::shared_ptr<PrepareWebRTCEvent> event) = 0;

		virtual void createAnswer(std::shared_ptr<PrepareWebRTCEvent> event) = 0;

		virtual void handleRemoteJsep(std::shared_ptr<PrepareWebRTCPeerEvent> event) = 0;

		virtual void hangup(bool sendRequest) = 0;

		virtual void detach(std::shared_ptr<DetachEvent> event) = 0;
	};
}