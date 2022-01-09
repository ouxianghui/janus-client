#ifndef CREATE_ROOM_DIALOG_H
#define CREATE_ROOM_DIALOG_H

#include <QDialog>

namespace Ui {
class CreateRoomDialog;
}

class CreateRoomDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CreateRoomDialog(QWidget *parent = nullptr);

    ~CreateRoomDialog();

    int64_t roomId();

    std::string description();

    std::string secret();

    std::string pin();

    bool permanent();

    bool isPrivate();

private:
    Ui::CreateRoomDialog *ui;
};

#endif // CREATE_ROOM_DIALOG_H
