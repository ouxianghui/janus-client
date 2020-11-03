#include "video_room_dialog.h"
#include "ui_video_room_dialog.h"

VideoRoomDialog::VideoRoomDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::VideoRoomDialog)
{
    ui->setupUi(this);
}

VideoRoomDialog::~VideoRoomDialog()
{
    delete ui;
}

void VideoRoomDialog::on_attachPushButton_clicked()
{

}

void VideoRoomDialog::on_joinPushButton_clicked()
{

}

void VideoRoomDialog::on_createPushButton_clicked()
{

}
