#pragma once

#include <functional>
#include "plugin_client.h"
#include "utils/universal_observable.hpp"
#include "video_room_models.h"

namespace vi {
	class IVideoRoomEventHandler;
	class IVideoRoomApi;

	using DelayedTask = std::function<void()>;

	class VideoRoomSubscriber : public PluginClient, public UniversalObservable<IVideoRoomEventHandler>
	{
	public:
		VideoRoomSubscriber(std::shared_ptr<WebRTCServiceInterface> wrs, const std::string& pluginName, const std::string& opaqueId);

		~VideoRoomSubscriber();

		void registerEventHandler(std::shared_ptr<IVideoRoomEventHandler> handler);

		void unregisterEventHandler(std::shared_ptr<IVideoRoomEventHandler> handler);

		void setRoomApi(std::shared_ptr<IVideoRoomApi> videoRoomApi);

		void setRoomId(int64_t roomId);

		int64_t getRoomId() const;

		void setPrivateId(int64_t id);

		void subscribeTo(const std::vector<vr::Publisher>& publishers);

		void unsubscribeFrom(int64_t id);

	protected:
		void onAttached(bool success) override;

		void onHangup() override;

		void onIceState(webrtc::PeerConnectionInterface::IceConnectionState iceState) override;

		void onMediaState(const std::string& media, bool on, const std::string& mid) override;

		void onWebrtcState(bool isActive, const std::string& reason) override;

		void onSlowLink(bool uplink, bool lost, const std::string& mid) override;

		void onMessage(const std::string& data, const std::string& jsep) override;

		void onLocalTrack(rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> track, bool on) override;

		void onRemoteTrack(rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> track, const std::string& mid, bool on) override;

		void onData(const std::string& data, const std::string& label) override;

		void onDataOpen(const std::string& label) override;

		void onCleanup() override;

		void onDetached() override;

		void onStatsReport(const rtc::scoped_refptr<const webrtc::RTCStatsReport>& report) override;

	private:
		void join(const std::vector<vr::Publisher>& publishers);

		void subscribe(const std::vector<vr::Publisher>& publishers);

	private:
		int64_t _roomId;

		std::weak_ptr<IVideoRoomApi> _videoRoomApi;

		// key: mid
		std::map<std::string, rtc::scoped_refptr<webrtc::MediaStreamInterface>> _remoteStreams;

		std::atomic_bool _attached;

		std::vector<vr::Publisher> _publishers;

		DelayedTask _joinTask;
	};
}
