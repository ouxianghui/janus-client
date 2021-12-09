/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#pragma once

#include "plugin_client.h"
#include "utils/universal_observable.hpp"
#include "i_video_room_listener.h"


namespace webrtc {
	class MediaStreamInterface;
}

namespace vi {
	class IVideoRoomApi;
	class Participant;
	class VideoRoomPublisher;
	class VideoRoomSubscriber;

	struct ParticipantSt {
		int64_t id;
		std::string displayName;
	};

	class VideoRoom : public PluginClient, public UniversalObservable<IVideoRoomListener>
	{
	public:
		VideoRoom(std::shared_ptr<WebRTCServiceInterface> wrs);

		~VideoRoom();

		void init();

		void addListener(std::shared_ptr<IVideoRoomListener> listener);

		void removeListener(std::shared_ptr<IVideoRoomListener> listener);

		std::shared_ptr<Participant> getParticipant(int64_t pid);

		std::shared_ptr<IVideoRoomApi> getVideoRoomApi();

		uint64_t getId() { return PluginClient::getId(); }

		void setRoomId(int64_t roomId);

		int64_t getRoomId() const;

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

	protected:
		void publishOwnStream(bool audioOn);

		void unpublishOwnStream();

		void createParticipant(const ParticipantSt& info);

		void removeParticipant(int64_t id);

	private:
		int64_t _roomId;
		
		std::map<int64_t, std::shared_ptr<Participant>> _participantsMap;

		std::shared_ptr<IVideoRoomApi> _videoRoomApi;

		std::shared_ptr<VideoRoomSubscriber> _subscriber;

		// key: trackId
		std::map<std::string, rtc::scoped_refptr<webrtc::MediaStreamInterface>> _localStreams;
	};
}
