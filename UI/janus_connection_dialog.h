#pragma once

#include <QDialog>
#include "ui_janus_connection_dialog.h"
#include <memory>
#include "i_engine_event_handler.h"

class JanusConnectionDialog : public QDialog, public vi::IEngineEventHandler, public std::enable_shared_from_this<JanusConnectionDialog>
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

	// IEngineEventHandler

	void onStatus(vi::EngineStatus status) override;

	void onError(int32_t code) override;

private:
	Ui::JanusConnectionDialog ui;
};
