#pragma once

#include <QObject>
#include <memory>
#include "i_participants_control_event_handler.h"

class ParticipantsEventAdapter
	: public QObject
	, public vi::IParticipantsControlEventHandler
	, public std::enable_shared_from_this<ParticipantsEventAdapter>
{
	Q_OBJECT

public:
	ParticipantsEventAdapter(QObject* parent);

	~ParticipantsEventAdapter();

	void onCreateParticipant(std::shared_ptr<vi::Participant> participant) override;

	void onUpdateParticipant(std::shared_ptr<vi::Participant> participant) override;

	void onRemoveParticipant(std::shared_ptr<vi::Participant> participant) override;

signals:
	void createParticipant(std::shared_ptr<vi::Participant> participant);

	void updateParticipant(std::shared_ptr<vi::Participant> participant);

	void removeParticipant(std::shared_ptr<vi::Participant> participant);
};
