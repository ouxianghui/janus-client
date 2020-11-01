#pragma once

#include <QDialog>
#include "ui_janus_connection_dialog.h"
#include <memory>
#include "i_webrtc_service_listener.h"

class JanusConnectionDialog : public QDialog, public vi::IWebRTCServiceListener, public std::enable_shared_from_this<JanusConnectionDialog>
{
	Q_OBJECT

public:
	JanusConnectionDialog(QWidget *parent = Q_NULLPTR);
	~JanusConnectionDialog();

    void init();

	void cleanup();

private slots:
    void on_connectJanusPushButton_clicked();

    void on_cancelConnectPushButton_clicked();

private:
	void onStatus(vi::ServiceStauts status) override;

private:
	Ui::JanusConnectionDialog ui;
};
