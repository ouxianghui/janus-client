#pragma once

#include <memory>
#include <string>
#include <vector>

namespace vi {

	class Participant;
	class IVideoRoomEventHandler;
	class IVideoRoomApi;

	class VideoRoomClientInterface {
	public:
		virtual ~VideoRoomClientInterface() = default;

		virtual void init() = 0;

		virtual void destroy() = 0;

		virtual void registerEventHandler(std::shared_ptr<IVideoRoomEventHandler> handler) = 0;

		virtual void unregisterEventHandler(std::shared_ptr<IVideoRoomEventHandler> handler) = 0;

		virtual uint64_t getId() = 0;

		virtual void setRoomId(int64_t roomId) = 0;

		virtual int64_t getRoomId() const = 0;

		virtual std::shared_ptr<Participant> getParticipant(int64_t pid) = 0;

		virtual std::vector<std::shared_ptr<Participant>> getParticipantList() = 0;

		virtual std::shared_ptr<IVideoRoomApi> getVideoRoomApi() = 0;

		// media control
		 
		virtual int32_t remoteVolume(const std::string& pid) = 0;

		virtual int32_t localVolume(const std::string& pid) = 0;

		virtual bool isAudioMuted(const std::string& pid) = 0;

		virtual bool isVideoMuted(const std::string& pid) = 0;

		virtual bool muteAudio(const std::string& pid) = 0;

		virtual bool muteVideo(const std::string& pid) = 0;

		virtual bool unmuteAudio(const std::string& pid) = 0;

		virtual bool unmuteVideo(const std::string& pid) = 0;
	};

}