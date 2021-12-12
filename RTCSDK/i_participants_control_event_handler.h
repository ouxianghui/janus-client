#pragma once

#include <memory>

namespace vi {

	class Participant;

	class IParticipantsControlEventHandler {
	public:
		virtual ~IParticipantsControlEventHandler() = default;

		virtual void onCreateParticipant(std::shared_ptr<Participant> participant) {}

		virtual void onUpdateParticipant(std::shared_ptr<Participant> participant) {}

		virtual void onRemoveParticipant(std::shared_ptr<Participant> participant) {}
	};

}