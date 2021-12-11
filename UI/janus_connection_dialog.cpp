#include "janus_connection_dialog.h"
#include "service/app_instance.h"
#include "signaling_service_interface.h"
#include "utils/thread_provider.h"

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
	auto ss = rtcApp->getSignalingService();
	ss->registerObserver(shared_from_this());
}

void JanusConnectionDialog::cleanup()
{
	auto ss = rtcApp->getSignalingService();
	ss->unregisterObserver(shared_from_this());
}

void JanusConnectionDialog::on_connectJanusPushButton_clicked()
{
    auto url = ui.serverUrlLineEdit->text().toStdString();
	auto ss = rtcApp->getSignalingService();
	ss->connect(url);
}

void JanusConnectionDialog::on_cancelConnectPushButton_clicked()
{
    reject();
}

void JanusConnectionDialog::onSessionStatus(vi::SessionStatus status)
{
	if (vi::SessionStatus::CONNECTED == status) {
		TMgr->thread("main")->PostTask(RTC_FROM_HERE, [wself = weak_from_this()]() {
			if (auto self = wself.lock()) {
				self->accept();
			}
		});
	}
}
