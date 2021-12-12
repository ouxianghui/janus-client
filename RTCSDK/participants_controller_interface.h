#pragma once

#include <memory>
#include <string>
#include <vector>

namespace vi {

	class Participant;
	class IParticipantsControlEventHandler;

	class ParticipantsContrllerInterface
	{
	public:

		virtual void init() = 0;

		virtual void destroy() = 0;

		virtual void registerEventHandler(std::shared_ptr<IParticipantsControlEventHandler> handler) = 0;

		virtual void unregisterEventHandler(std::shared_ptr<IParticipantsControlEventHandler> handler) = 0;

		virtual std::shared_ptr<Participant> participant(int64_t pid) = 0;

		virtual std::vector<std::shared_ptr<Participant>> participantList() = 0;

		virtual void kick(int64_t pid) = 0;
	};

}