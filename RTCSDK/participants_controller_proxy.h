#pragma once

#include "weak_proxy.h"

namespace vi {

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