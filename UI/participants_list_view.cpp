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
	ui.listWidgetParticipants->setViewMode(QListView::ListMode);  
}

ParticipantsListView::~ParticipantsListView()
{
	removeAll();
}

void ParticipantsListView::addParticipant(std::shared_ptr<vi::Participant> participant)
{
	if (!participant) {
		return;
	}
    ParticipantItemView* view = new ParticipantItemView(participant, this);
	view->setDisplayName(participant->displayName());

    QListWidgetItem* item = new QListWidgetItem(ui.listWidgetParticipants);
	item->setSizeHint(QSize(0, 50));
    QVariant var(participant->getId());
    item->setData(Qt::UserRole+1, var);
    ui.listWidgetParticipants->addItem(item);
    ui.listWidgetParticipants->setItemWidget(item, view);

}

void ParticipantsListView::removeParticipant(std::shared_ptr<vi::Participant> participant)
{
    if (!participant) {
        return;
    }
    int count = ui.listWidgetParticipants->count();
    for (int i = 0; i < count; ++i) {
        QListWidgetItem* item = ui.listWidgetParticipants->item(i);
        if (item) {
            QVariant var = item->data(Qt::UserRole+1);
            if (var.toULongLong() == participant->getId()) {
                QWidget *widget = ui.listWidgetParticipants->itemWidget(item);
                if (nullptr != widget)
                {
                    ui.listWidgetParticipants->removeItemWidget(item);
                    widget->deleteLater();
                }

                item = ui.listWidgetParticipants->takeItem(i);
                delete item;
                item = nullptr;

                ui.listWidgetParticipants->update();
                break;
            }
        }
     }
}

void ParticipantsListView::removeAll()
{
	while (0 != ui.listWidgetParticipants->count()) {
		QListWidgetItem *item = ui.listWidgetParticipants->item(0);
		QWidget *widget = ui.listWidgetParticipants->itemWidget(item);
		if (nullptr != widget)
		{
			ui.listWidgetParticipants->removeItemWidget(item);
			widget->deleteLater();
		}

		item = ui.listWidgetParticipants->takeItem(0);
		delete item;
		item = nullptr;
	}
}
