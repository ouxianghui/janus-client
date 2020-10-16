/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#pragma once

#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include "api/peer_connection_interface.h"
#include "i_sfu_client.h"
#include "i_sfu_client_listener.h"
#include "webrtc_service_events.h"
#include "webrtc_service_interface.h"

namespace vi {
	class TaskScheduler;
	class CapturerTrackSource;
	class WebRTCService
		: public WebRTCServiceInterface
		, public ISFUClientListener
		, public core::Observable
		, public std::enable_shared_from_this<WebRTCService>
	{
	public:
		WebRTCService();

		~WebRTCService() override;

		void init() override;

		void cleanup() override;

	protected:
		// IWebRTCService implement

	    void addListener(std::shared_ptr<IWebRTCServiceListener> listener) override;

		void removeListener(std::shared_ptr<IWebRTCServiceListener> listener) override;

		ServiceStauts status() override;

		void attach(const std::string& plugin, const std::string& opaqueId, std::shared_ptr<IWebRTCEventHandler> wreh) override;

		void destroy(std::shared_ptr<DestroySessionEvent> event) override;

		void reconnect() override;

		int32_t remoteVolume(int64_t handleId) override;

		int32_t localVolume(int64_t handleId) override;

		bool isAudioMuted(int64_t handleId) override;

		bool isVideoMuted(int64_t handleId) override;

		bool muteAudio(int64_t handleId) override;

		bool muteVideo(int64_t handleId) override;

		bool unmuteAudio(int64_t handleId) override;

		bool unmuteVideo(int64_t handleId) override;

		std::string getBitrate(int64_t handleId) override;

		void sendMessage(int64_t handleId, std::shared_ptr<SendMessageEvent> event) override;

		void sendData(int64_t handleId, std::shared_ptr<SendDataEvent> event) override;

		void sendDtmf(int64_t handleId, std::shared_ptr<SendDtmfEvent> event) override;

		void createOffer(int64_t handleId, std::shared_ptr<PrepareWebRTCEvent> event) override;

		void createAnswer(int64_t handleId, std::shared_ptr<PrepareWebRTCEvent> event) override;

		void handleRemoteJsep(int64_t handleId, std::shared_ptr<PrepareWebRTCPeerEvent> event) override;

		void hangup(int64_t handleId, bool sendRequest) override;

		void detach(int64_t handleId, std::shared_ptr<DetachEvent> event) override;

	protected:
		// ISFUClientListener

	    void onOpened() override;

		void onClosed() override;

		void onFailed(int errorCode, const std::string& reason) override;

		void onMessage(std::shared_ptr<JanusResponse> model) override;

	private:
		int32_t getVolume(int64_t handleId, bool isRemote);

		bool isMuted(int64_t handleId, bool isVideo);

		bool mute(int64_t handleId, bool isVideo, bool mute);

		void prepareWebrtc(int64_t handleId, bool isOffer, std::shared_ptr<PrepareWebRTCEvent> event);

		void prepareWebrtcPeer(int64_t handleId, std::shared_ptr<PrepareWebRTCPeerEvent> event);

		void cleanupWebrtc(int64_t handleId, bool hangupRequest = true);

		void destroyHandle(int64_t handleId, std::shared_ptr<DetachEvent> event);

	private:
		void createSession(std::shared_ptr<CreateSessionEvent> event);

		void startHeartbeat();

		std::shared_ptr<IWebRTCEventHandler> getHandler(int64_t handleId);

		void prepareStreams(int64_t handleId, std::shared_ptr<PrepareWebRTCEvent> event, rtc::scoped_refptr<webrtc::MediaStreamInterface> stream);

		void sendSDP(int64_t handleId, std::shared_ptr<PrepareWebRTCEvent> event);

		void createDataChannel(int64_t handleId, const std::string& dcLabel, rtc::scoped_refptr<webrtc::DataChannelInterface> incoming);

		void _createOffer(int64_t handleId, std::shared_ptr<PrepareWebRTCEvent> event);

		void _createAnswer(int64_t handleId, std::shared_ptr<PrepareWebRTCEvent> event);

		void destroySession(std::shared_ptr<DestroySessionEvent> event);

		void configTracks(const MediaConfig& media, rtc::scoped_refptr<webrtc::PeerConnectionInterface> pc);

		void stopAllTracks(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream);

	private:
		std::string _server;	

		std::string _token;

		std::string _opaqueId;

		std::string _apiSecret;

		bool _unifiedPlan = true;

		std::vector<std::string> _iceServers;

		bool _ipv6Support;

		webrtc::PeerConnectionInterface::BundlePolicy _bundlePolicy;

		webrtc::PeerConnectionInterface::IceTransportsType _iceTransportPolicy;

		webrtc::PeerConnectionInterface::TlsCertPolicy _tlsCertPolicy;

		bool _destroyOnUnload;

		int64_t _sessionId = -1;

		int32_t _keepAlivePeriod;

		bool _connected = false;

		int32_t _retries = 3;

		bool _reconnect = false;

		std::unordered_map<int64_t, std::shared_ptr<IWebRTCEventHandler>> _wrehs;

		std::shared_ptr<ISFUClient> _client;

		std::shared_ptr<TaskScheduler> _taskScheduler;

		ServiceStauts _serviceStatus = ServiceStauts::DOWN;

		std::vector<std::weak_ptr<IWebRTCServiceListener>> _listeners;

		rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> _pcf;

		uint64_t _heartbeatTaskId = 0;

		std::unique_ptr<rtc::Thread> _signaling;
		std::unique_ptr<rtc::Thread> _worker;
		std::unique_ptr<rtc::Thread> _network;
		rtc::scoped_refptr<CapturerTrackSource> _videoDevice;
	};
}

