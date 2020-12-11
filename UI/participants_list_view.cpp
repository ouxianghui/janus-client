#include "participants_list_view.h"
#include "participant.h"
#include "video_room.h"
#include "participant_item_view.h"
#include <QListWidgetItem>
#include <QVariant>



ParticipantsListView::ParticipantsListView(std::shared_ptr<vi::VideoRoom> vr, QWidget *parent)
	: QWidget(parent)
	, _videoRoom(vr)
{
	ui.setupUi(this);
}

ParticipantsListView::~ParticipantsListView()
{
}

void ParticipantsListView::addParticipant(std::shared_ptr<vi::Participant> participant)
{
	if (!participant) {
		return;
	}
    ParticipantItemView* view = new ParticipantItemView(participant, this);
    view->setFixedHeight(100);
    view->setAutoFillBackground(true);
	view->setDisplayName(participant->displayName());
    QListWidgetItem* item = new QListWidgetItem(ui.listWidgetParticipants);
    QVariant var((uint64_t)participant.get());
    item->setData(Qt::UserRole, var);
    ui.listWidgetParticipants->addItem(item);
    ui.listWidgetParticipants->setItemWidget(item, view);

}

void ParticipantsListView::removeParticipant(std::shared_ptr<vi::Participant> participant)
{
    if (!participant) {
        return;
    }
    int count = ui.listWidgetParticipants->count();
    for (int row = 0; row < count; ++row) {
        QListWidgetItem* item = ui.listWidgetParticipants->item(row);
        if (item) {
            QVariant var = item->data(Qt::UserRole);
            if ((uint64_t)var.data() == ((uint64_t)participant.get())) {
                ui.listWidgetParticipants->removeItemWidget(item);
                ui.listWidgetParticipants->takeItem(row);
                break;
            }
        }
     }
}
