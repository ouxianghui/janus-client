#pragma once

#include <QDialog>
#include "ui_janus_connection_dialog.h"

class JanusConnectionDialog : public QDialog
{
	Q_OBJECT

public:
	JanusConnectionDialog(QWidget *parent = Q_NULLPTR);
	~JanusConnectionDialog();

private slots:
    void on_connectJanusPushButton_clicked();

    void on_cancelConnectPushButton_clicked();

private:
	Ui::JanusConnectionDialog ui;
};
