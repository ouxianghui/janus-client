#include "join_room_dialog.h"
#include "ui_join_room_dialog.h"

JoinRoomDialog::JoinRoomDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::JoinRoomDialog)
{
    ui->setupUi(this);
}

JoinRoomDialog::~JoinRoomDialog()
{
    delete ui;
}

int64_t JoinRoomDialog::roomId()
{
    return ui->roomIdLineEdit->text().toLongLong();
}

std::string JoinRoomDialog::displayName()
{
    return ui->displayNameLineEdit->text().toLocal8Bit().constData();
}

std::string JoinRoomDialog::pin()
{
    return ui->pinLineEdit->text().toLocal8Bit().constData();
}