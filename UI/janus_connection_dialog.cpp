#include "janus_connection_dialog.h"
#include "service/app_instance.h"
#include "webrtc_service_interface.h"
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
	auto wrs = rtcApp->getWebrtcService();
	wrs->addListener(shared_from_this());
}

void JanusConnectionDialog::cleanup()
{
	auto wrs = rtcApp->getWebrtcService();
	wrs->removeListener(shared_from_this());
}

void JanusConnectionDialog::on_connectJanusPushButton_clicked()
{
    auto url = ui.serverUrlLineEdit->text().toStdString();
	auto wrs = rtcApp->getWebrtcService();
	wrs->connect(url);
}

void JanusConnectionDialog::on_cancelConnectPushButton_clicked()
{
    reject();
}

void JanusConnectionDialog::onStatus(vi::ServiceStauts status)
{
	if (vi::ServiceStauts::UP == status) {
		TMgr->thread("main")->PostTask(RTC_FROM_HERE, [wself = weak_from_this()]() {
			if (auto self = wself.lock()) {
				self->accept();
			}
		});
	}
}
