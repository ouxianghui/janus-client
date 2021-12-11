/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#pragma once

#include <memory>
#include <string>
#include "i_webrtc_event_handler.h"
#include "i_signaling_event_handler.h"
#include "signaling_service_status.h"

namespace vi {
	class SignalingServiceInterface;
	class TaskScheduler;

	class PluginClient
		: public ISignalingEventHandler
		, public IWebrtcEventHandler
		, public std::enable_shared_from_this<PluginClient>
	{
	public:
		PluginClient(std::shared_ptr<SignalingServiceInterface> ss);

		~PluginClient();

		void init();

		void destroy();

		uint64_t getId() { return _id; }

		// IPluginClient
		void setHandleId(int64_t handleId);

		std::shared_ptr<PluginContext> pluginContext() { return _pluginContext; }

		void attach();

		void sendMessage(std::shared_ptr<MessageEvent> event);

		void sendSdp();

		void sendData(std::shared_ptr<ChannelDataEvent> event);

		void sendDtmf(std::shared_ptr<DtmfEvent> event);

		void createOffer(std::shared_ptr<PrepareWebrtcEvent> event);

		void createAnswer(std::shared_ptr<PrepareWebrtcEvent> event);

		void handleRemoteJsep(std::shared_ptr<PrepareWebrtcPeerEvent> event);

		void hangup(bool sendRequest);

		void detach(std::shared_ptr<DetachEvent> event);

		void startStatsMonitor();

		void stopStatsMonitor();


	protected:
		void prepareWebrtc(bool isOffer, std::shared_ptr<PrepareWebrtcEvent> event);

		void prepareStreams(std::shared_ptr<PrepareWebrtcEvent> event, rtc::scoped_refptr<webrtc::MediaStreamInterface> stream);


		void createDataChannel(const std::string& dcLabel, rtc::scoped_refptr<webrtc::DataChannelInterface> incoming);

		void _createOffer(std::shared_ptr<PrepareWebrtcEvent> event);

		void _createAnswer(std::shared_ptr<PrepareWebrtcEvent> event);

		void configTracks(const MediaConfig& media, rtc::scoped_refptr<webrtc::PeerConnectionInterface> pc);

		void stopAllTracks(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream);

		void cleanupWebrtc(bool hangupRequest = true);

	protected:
		// webrtc events

		void OnStandardizedIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state) override;

		void OnConnectionChange(webrtc::PeerConnectionInterface::PeerConnectionState new_state) override;

		void OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state) override;

		void OnIceCandidate(const webrtc::IceCandidateInterface* candidate) override;

		void OnTrack(rtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver) override;

		void OnRemoveTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver) override;

	protected:
		// plugin client events
		 
		virtual void onLocalTrack(rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> track, bool on) {}

		virtual void onRemoteTrack(rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> track, const std::string& mid, bool on) {}

		virtual void onChannelOpened(const std::string& label) {}

		virtual void onChannelClosed(const std::string& label) {}

		virtual void onChannelData(const std::string& label, const std::string& data) {}

		virtual void onStatsDelivered(const rtc::scoped_refptr<const webrtc::RTCStatsReport>& report) {}

	public:
		// signaling service events

		void onTrickle(const std::string& trickle) override;

		void onCleanup() override;

	protected:
		uint64_t _id = 0;

		uint64_t _privateId = 0;

		std::shared_ptr<PluginContext> _pluginContext;

		std::shared_ptr<TaskScheduler> _rtcStatsTaskScheduler;

		uint64_t _rtcStatsTaskId = 0;

		rtc::Thread* _eventHandlerThread = nullptr;

		// key: trackId, value: mid
		std::unordered_map<std::string, std::string> _trackIdsMap;
	};
}

