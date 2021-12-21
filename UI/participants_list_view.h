#pragma once

#include <QWidget>
#include "ui_participants_list_view.h"
#include <memory>

namespace vi {
	class Participant;
	class VideoRoomClientInterface;
}

class ParticipantsListView : public QWidget, public std::enable_shared_from_this<ParticipantsListView>
{
	Q_OBJECT

public:
	ParticipantsListView(std::shared_ptr<vi::VideoRoomClientInterface> vrc, QWidget *parent = Q_NULLPTR);

	~ParticipantsListView();

	void addParticipant(std::shared_ptr<vi::Participant> participant);

	void removeParticipant(std::shared_ptr<vi::Participant> participant);

	void removeAll();

private:
	Ui::ParticipantsListView ui;

	std::weak_ptr<vi::VideoRoomClientInterface> _vrc;
};
