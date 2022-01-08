#pragma once

#include <memory>
#include <string>
#include <vector>
#include "weak_proxy.h"

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

	BEGIN_WEAK_PROXY_MAP(ParticipantsContrller)
		WEAK_PROXY_THREAD_DESTRUCTOR()
		WEAK_PROXY_METHOD0(void, init)
		WEAK_PROXY_METHOD0(void, destroy)
		WEAK_PROXY_METHOD1(void, registerEventHandler, std::shared_ptr<IParticipantsControlEventHandler>)
		WEAK_PROXY_METHOD1(void, unregisterEventHandler, std::shared_ptr<IParticipantsControlEventHandler>)
		WEAK_PROXY_METHOD1(std::shared_ptr<Participant>, participant, int64_t)
		WEAK_PROXY_METHOD0(std::vector<std::shared_ptr<Participant>>, participantList)
		WEAK_PROXY_METHOD1(void, kick, int64_t)
	END_WEAK_PROXY_MAP()
}