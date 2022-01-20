#include "create_room_dialog.h"
#include "ui_create_room_dialog.h"

CreateRoomDialog::CreateRoomDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CreateRoomDialog)
{
    ui->setupUi(this);

    ui->permanentRadioButtonNo->setChecked(true);

    ui->privateRadioButtonNo->setChecked(true);
}

CreateRoomDialog::~CreateRoomDialog()
{
    delete ui;
}

int64_t CreateRoomDialog::roomId()
{
    return ui->roomIdLineEdit->text().toLongLong();
}

std::string CreateRoomDialog::description()
{
    return ui->roomNameLineEdit->text().toLocal8Bit().constData();
}

std::string CreateRoomDialog::secret()
{
    return ui->secretLineEdit->text().toLocal8Bit().constData();
}

std::string CreateRoomDialog::pin()
{
    return ui->pinLineEdit->text().toLocal8Bit().constData();
}

bool CreateRoomDialog::permanent()
{
    if (ui->permanentRadioButtonYes->isChecked()) {
        return true;
    }
    else if (ui->permanentRadioButtonNo->isChecked()) {
        return false;
    }

    return false;
}

bool CreateRoomDialog::isPrivate()
{
    if (ui->privateRadioButtonYes->isChecked()) {
        return true;
    }
    else if (ui->privateRadioButtonYes->isChecked()) {
        return false;
    }

    return false;
}

std::string CreateRoomDialog::displayName()
{
    return ui->displayNameLineEdit->text().toLocal8Bit().constData();
}
