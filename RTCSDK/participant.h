/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#pragma once

#include "plugin_client.h"
#include "service/observable.h"
#include "i_video_room_listener.h"

namespace vi {
	class Participant
		: public PluginClient
		, public core::Observable
	{
	public:
		Participant(const std::string& plugin, 
			const std::string& opaqueId, 
			int64_t id,
			int64_t privateId,
			const std::string& displayName, 
			std::shared_ptr<WebRTCServiceInterface> wrs,
			std::shared_ptr<std::vector<std::weak_ptr<IVideoRoomListener>>> listeners);

		~Participant();

		uint64_t getId() { return PluginClient::getId(); }

	protected:
		void onAttached(bool success) override;

		void onHangup() override;

		void onIceState(webrtc::PeerConnectionInterface::IceConnectionState iceState) override;

		void onMediaState(const std::string& media, bool on) override;

		void onWebrtcState(bool isActive, const std::string& reason) override;

		void onSlowLink(bool uplink, bool lost) override;

		void onMessage(const std::string& data, const std::string& jsep) override;

		void onCreateLocalStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) override;

		void onDeleteLocalStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) override;

		void onCreateRemoteStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) override;

		void onDeleteRemoteStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) override;

		void onData(const std::string& data, const std::string& label) override;

		void onDataOpen(const std::string& label) override;

		void onCleanup() override;

		void onDetached() override;

	private:
		bool _videoOn;
		bool _audioOn;
		std::string _displayName;
		std::weak_ptr<rtc::VideoSinkInterface<webrtc::VideoFrame>> _renderer;
		std::weak_ptr<std::vector<std::weak_ptr<IVideoRoomListener>>> _listeners;
	};
}


