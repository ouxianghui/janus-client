#pragma once

#include <functional>
#include "plugin_client.h"
#include "utils/universal_observable.hpp"
#include "video_room_models.h"

namespace vi {
	class IVideoRoomEventHandler;
	class IVideoRoomApi;
	class MediaController;

	using DelayedTask = std::function<void()>;

	class VideoRoomSubscriber : public PluginClient, public UniversalObservable<IVideoRoomEventHandler>
	{
	public:
		VideoRoomSubscriber(std::shared_ptr<SignalingServiceInterface> ss, const std::string& pluginName, const std::string& opaqueId, std::shared_ptr<MediaController> mediaController);

		~VideoRoomSubscriber();

		void init();

		void registerEventHandler(std::shared_ptr<IVideoRoomEventHandler> handler);

		void unregisterEventHandler(std::shared_ptr<IVideoRoomEventHandler> handler);

		void setRoomApi(std::shared_ptr<IVideoRoomApi> videoRoomApi);

		void setRoomId(int64_t roomId);

		int64_t getRoomId() const;

		void setPrivateId(int64_t id);

		void subscribeTo(const std::vector<vr::Publisher>& publishers);

		void unsubscribeFrom(int64_t id);

	protected:

		// signaling event

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

		void onRemoteTrack(rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> track, const std::string& mid, bool on) override;

	private:
		void join(const std::vector<vr::Publisher>& publishers);

		void subscribe(const std::vector<vr::Publisher>& publishers);

	private:
		int64_t _roomId;

		std::weak_ptr<IVideoRoomApi> _videoRoomApi;

		std::atomic_bool _attached;

		std::vector<vr::Publisher> _publishers;

		DelayedTask _joinTask;

		std::weak_ptr<MediaController> _mediaController;
	};
}
