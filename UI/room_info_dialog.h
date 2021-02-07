#ifndef ROOM_INFO_DIALOG_H
#define ROOM_INFO_DIALOG_H

#include <QDialog>

namespace Ui {
class RoomInfoDialog;
}

class RoomInfoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RoomInfoDialog(QWidget *parent = nullptr);
    ~RoomInfoDialog();

    int64_t getRoomId() const;

private slots:
    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

private:
    Ui::RoomInfoDialog *ui;
};

#endif // ROOM_INFO_DIALOG_H
