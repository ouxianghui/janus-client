/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#pragma once

#include "plugin_client.h"
#include "utils/universal_observable.hpp"
#include "i_video_room_event_handler.h"


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

	class VideoRoomClient : public PluginClient, public UniversalObservable<IVideoRoomEventHandler>
	{
	public:
		VideoRoomClient(std::shared_ptr<SignalingServiceInterface> ss);

		~VideoRoomClient();

		void init();

		void registerEventHandler(std::shared_ptr<IVideoRoomEventHandler> handler);

		void unregisterEventHandler(std::shared_ptr<IVideoRoomEventHandler> handler);

		std::shared_ptr<Participant> getParticipant(int64_t pid);

		std::shared_ptr<IVideoRoomApi> getVideoRoomApi();

		uint64_t getId() { return PluginClient::getId(); }

		void setRoomId(int64_t roomId);

		int64_t getRoomId() const;

	protected:

		// signaling events

		void onAttached(bool success) override;

		void onMediaStatus(const std::string& media, bool on, const std::string& mid) override;

		void onWebrtcStatus(bool isActive, const std::string& desc) override;

		void onSlowLink(bool uplink, bool lost, const std::string& mid) override;

		void onMessage(const std::string& data, const std::string& jsep) override;

		void onTimeout()override;

		void onError(const std::string& desc) override;

		void onHangup() override;

		void onCleanup() override;

		void onDetached() override;

	protected:

		// webrtc events

		virtual void onLocalTrack(rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> track, bool on);

	protected:
		void publishStream(bool audioOn);

		void unpublishStream();

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
