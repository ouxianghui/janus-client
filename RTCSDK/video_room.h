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
	class IVideoRoomApi;
	class Participant;

	struct ParticipantSt {
		int64_t id;
		std::string displayName;
		std::string audioCodec;
		std::string videoCodec;
	};

	class VideoRoom
		: public PluginClient
		, public core::Observable
	{
	public:
		VideoRoom(std::shared_ptr<WebRTCServiceInterface> wrs);

		~VideoRoom();

		void init();

		void addListener(std::shared_ptr<IVideoRoomListener> listener);

		void removeListener(std::shared_ptr<IVideoRoomListener> listener);

		std::shared_ptr<Participant> getParticipant(int64_t pid);

		std::shared_ptr<IVideoRoomApi> getVideoRoomApi();

		//std::shared_ptr<PluginClient> getParticipant() { return (_participantsMap.begin()->second); }

		uint64_t getId() { return PluginClient::getId(); }

		void setRoomId(int64_t roomId);

		int64_t getRoomId() const;

	protected:
		void onAttached(bool success) override;

		void onHangup() override;

		void onIceState(webrtc::PeerConnectionInterface::IceConnectionState iceState) override;

		void onMediaState(const std::string& media, bool on) override;

		void onWebrtcState(bool isActive, const std::string& reason) override;

		void onSlowLink(bool uplink, bool lost) override;

		void onMessage(const std::string& data, const std::string& jsep) override;

		void onCreateLocalStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) override;

		void onRemoveLocalStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) override;

		void onCreateRemoteStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) override;

		void onRemoveRemoteStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) override;

		void onData(const std::string& data, const std::string& label) override;

		void onDataOpen(const std::string& label) override;

		void onCleanup() override;

		void onDetached() override;

		void onStatsReport(const rtc::scoped_refptr<const webrtc::RTCStatsReport>& report) override;

	protected:
		void publishOwnStream(bool audioOn);

		void unpublishOwnStream();

		void createParticipant(const ParticipantSt& info);

		void removeParticipant(int64_t id);

	private:
		int64_t _roomId;
		
		std::map<int64_t, std::shared_ptr<Participant>> _participantsMap;

		std::shared_ptr<std::vector<std::weak_ptr<IVideoRoomListener>>> _listeners;

		std::shared_ptr<IVideoRoomApi> _videoRoomApi;
	};
}
