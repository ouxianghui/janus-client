#include "participants_event_proxy.h"


ParticipantsEventProxy::ParticipantsEventProxy(QObject* parent)
{

}

ParticipantsEventProxy::~ParticipantsEventProxy()
{

}

void ParticipantsEventProxy::onCreateParticipant(std::shared_ptr<vi::Participant> participant)
{
	emit createParticipant(participant);
}

void ParticipantsEventProxy::onUpdateParticipant(std::shared_ptr<vi::Participant> participant)
{
	emit updateParticipant(participant);
}

void ParticipantsEventProxy::onRemoveParticipant(std::shared_ptr<vi::Participant> participant)
{
	emit removeParticipant(participant);
}