#include "room_info_dialog.h"
#include "ui_room_info_dialog.h"

RoomInfoDialog::RoomInfoDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RoomInfoDialog)
{
    ui->setupUi(this);
}

RoomInfoDialog::~RoomInfoDialog()
{
    delete ui;
}

int64_t RoomInfoDialog::getRoomId() const
{
    return ui->roomIdLineEdit->text().toLongLong();
}

void RoomInfoDialog::on_buttonBox_accepted()
{

}

void RoomInfoDialog::on_buttonBox_rejected()
{

}
