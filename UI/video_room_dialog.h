#ifndef VIDEO_ROOM_DIALOG_H
#define VIDEO_ROOM_DIALOG_H

#include <QDialog>

namespace Ui {
class VideoRoomDialog;
}

class VideoRoomDialog : public QDialog
{
    Q_OBJECT

public:
    explicit VideoRoomDialog(QWidget *parent = nullptr);
    ~VideoRoomDialog();

private slots:
    void on_attachPushButton_clicked();

    void on_joinPushButton_clicked();

    void on_createPushButton_clicked();

private:
    Ui::VideoRoomDialog *ui;
};

#endif // VIDEO_ROOM_DIALOG_H
