#include "participants_event_adapter.h"


ParticipantsEventAdapter::ParticipantsEventAdapter(QObject* parent)
{

}

ParticipantsEventAdapter::~ParticipantsEventAdapter()
{

}

void ParticipantsEventAdapter::onCreateParticipant(std::shared_ptr<vi::Participant> participant)
{
	emit createParticipant(participant);
}

void ParticipantsEventAdapter::onUpdateParticipant(std::shared_ptr<vi::Participant> participant)
{
	emit updateParticipant(participant);
}

void ParticipantsEventAdapter::onRemoveParticipant(std::shared_ptr<vi::Participant> participant)
{
	emit removeParticipant(participant);
}