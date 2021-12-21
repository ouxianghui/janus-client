#include "janus_connection_dialog.h"
#include "service/rtc_engine.h"
#include "signaling_client_interface.h"
#include "utils/thread_provider.h"
#include "service/i_unified_factory.h"
#include "app_delegate.h"

JanusConnectionDialog::JanusConnectionDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
}

JanusConnectionDialog::~JanusConnectionDialog()
{

}

void JanusConnectionDialog::init()
{
	appDelegate->getRtcEngine()->registerEventHandler(shared_from_this());
}

void JanusConnectionDialog::cleanup()
{
	appDelegate->getRtcEngine()->unregisterEventHandler(shared_from_this());
}

void JanusConnectionDialog::on_connectJanusPushButton_clicked()
{
	vi::Options opts;
	opts.serverUrl = ui.serverUrlLineEdit->text().toStdString();
	appDelegate->getRtcEngine()->setOptions(opts);
	appDelegate->getRtcEngine()->startup();
}

void JanusConnectionDialog::on_cancelConnectPushButton_clicked()
{
    reject();
}

void JanusConnectionDialog::onStatus(vi::EngineStatus status)
{
	if (vi::EngineStatus::CONNECTED == status) {
		accept();
	}
}

void JanusConnectionDialog::onError(int32_t code)
{

}
