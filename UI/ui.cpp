#include "ui.h"
#include <QDockWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QToolButton>
#include "Service/app_instance.h"
#include "webrtc_service_interface.h"
#include "video_room.h"
#include "message_models.h"
#include "string_utils.h"
#include "webrtc_service_events.h"
#include "x2struct.hpp"
#include "api/media_stream_interface.h"
#include "gl_video_renderer.h"
#include "participant.h"
#include "logger/logger.h"
#include "video_room_listener_proxy.h"
#include "video_room_dialog.h"
#include "i_video_room_api.h"
#include "room_info_dialog.h"

UI::UI(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
    //ui.mainToolBar->setFixedHeight(64);
    this->setWindowState(Qt::WindowMaximized);

	_videoRoomListenerProxy = std::make_shared<VideoRoomListenerProxy>(this);
	connect(_videoRoomListenerProxy.get(), &VideoRoomListenerProxy::mediaState, this, &UI::onMediaState, Qt::QueuedConnection);
	connect(_videoRoomListenerProxy.get(), &VideoRoomListenerProxy::createParticipant, this, &UI::onCreateParticipant, Qt::QueuedConnection);
	connect(_videoRoomListenerProxy.get(), &VideoRoomListenerProxy::updateParticipant, this, &UI::onUpdateParticipant, Qt::QueuedConnection);
	connect(_videoRoomListenerProxy.get(), &VideoRoomListenerProxy::removeParticipant, this, &UI::onRemoveParticipant, Qt::QueuedConnection);
	connect(_videoRoomListenerProxy.get(), &VideoRoomListenerProxy::createVideoTrack, this, &UI::onCreateVideoTrack, Qt::QueuedConnection);
	connect(_videoRoomListenerProxy.get(), &VideoRoomListenerProxy::removeVideoTrack, this, &UI::onRemoveVideoTrack, Qt::QueuedConnection);

    ui.actionAudio->setEnabled(false);
    ui.actionVideo->setEnabled(false);
}

UI::~UI()
{
	if (_galleryView) {
		_galleryView->removeAll();
	}

	if (_selfContentView) {
		_selfContentView->cleanup();
	}
}

void UI::init()
{
	auto wrs = rtcApp->getWebrtcService();
	_vr = std::make_shared<vi::VideoRoom>(wrs);
	_vr->init();
	_vr->addListener(_videoRoomListenerProxy);

	_galleryView = new GalleryView(this);
	setCentralWidget(_galleryView);


    QWidget* dockContentView = new QWidget(this);
    QVBoxLayout* dockContentViewLayout = new QVBoxLayout(dockContentView);
    dockContentView->setLayout(dockContentViewLayout);

    _selfView = new QWidget(this);
    QGridLayout* selfViewLayout = new QGridLayout(_selfView);
    _selfView->setLayout(selfViewLayout);
    dockContentViewLayout->addWidget(_selfView, 100);

    QToolButton* audioButton = new QToolButton(this);
    audioButton->setDefaultAction(ui.actionAudio);

    QToolButton* videoButton = new QToolButton(this);
    videoButton->setDefaultAction(ui.actionVideo);

    QWidget* buttonsView = new QWidget(this);
    QHBoxLayout* buttonsViewLayout = new QHBoxLayout(buttonsView);
    buttonsView->setLayout(buttonsViewLayout);
    buttonsViewLayout->addWidget(audioButton, 1);
    buttonsViewLayout->addWidget(videoButton, 1);
    dockContentViewLayout->addWidget(buttonsView, 1);

	_participantsListView = std::make_shared<ParticipantsListView>(_vr, this);
	_participantsListView->setFixedWidth(200);
    dockContentViewLayout->addWidget(_participantsListView.get(), 400);

	QDockWidget* dockWidget = new QDockWidget(this);
    dockWidget->setWindowTitle("Participants List");
    dockWidget->setWidget(dockContentView);

	this->addDockWidget(Qt::RightDockWidgetArea, dockWidget);
}

void UI::onStatus(vi::ServiceStauts status)
{
	if (vi::ServiceStauts::UP == status) {
	}
}

void UI::onMediaState(bool isActive, const std::string& reason)
{
	if (isActive) {
		ui.actionAudio->setEnabled(true);
		ui.actionVideo->setEnabled(true);
		if (_vr) {
			if (_vr->isAudioMuted("")) {
				ui.actionAudio->setChecked(false);
			}
			else {
				ui.actionAudio->setChecked(true);
			}

			if (_vr->isVideoMuted("")) {
				ui.actionVideo->setChecked(false);
			}
			else {
				ui.actionVideo->setChecked(true);
			}
		}
	}
}

void UI::onCreateParticipant(std::shared_ptr<vi::Participant> participant)
{
	_participantsListView->addParticipant(participant);
}

void UI::onUpdateParticipant(std::shared_ptr<vi::Participant> participant)
{

}

void UI::onRemoveParticipant(std::shared_ptr<vi::Participant> participant)
{
	if (participant) {
		_galleryView->removeView(participant->getId());
		_participantsListView->removeParticipant(participant);
	}
}

void UI::onCreateVideoTrack(uint64_t pid, rtc::scoped_refptr<webrtc::VideoTrackInterface> track)
{
	if (!track) {
		return;
	}
	if (track->kind() == webrtc::MediaStreamTrackInterface::kVideoKind) {
        if (_vr->getId() != pid) {
            GLVideoRenderer* renderer = new GLVideoRenderer(_galleryView);
            renderer->init();
            renderer->show();

            std::shared_ptr<ContentView> view = std::make_shared<ContentView>(pid, track, renderer);
            view->init();

            _galleryView->insertView(view);
        }
        else {
            GLVideoRenderer* renderer = new GLVideoRenderer(this);
            renderer->init();
            renderer->show();

            _selfContentView = std::make_shared<ContentView>(pid, track, renderer);
            _selfContentView->init();
            _selfView->layout()->addWidget(_selfContentView->view());
        }
	}
}

void UI::onRemoveVideoTrack(uint64_t pid, rtc::scoped_refptr<webrtc::VideoTrackInterface> track)
{
	_galleryView->removeView(pid);
}

void UI::closeEvent(QCloseEvent* event)
{
	//_vr->hangup(true);
	if (_galleryView) {
		_galleryView->removeAll();
	}
}

void UI::on_actionAttachRoom_triggered(bool checked)
{
	if (checked) {
		if (_vr) {
			_vr->attach();
		}
	}
	else {

	}
}

void UI::on_actionPublishStream_triggered(bool checked)
{
	if (!_vr) {
		return;
	}
	if (checked) {
		vi::vr::FetchParticipantsRequest req;
		req.room = _vr->getRoomId();
		_vr->getVideoRoomApi()->fetchParticipants(req, nullptr);
	}
	else {
	}
}

void UI::on_actionJanusGateway_triggered()
{

}

void UI::on_actionMyProfile_triggered()
{

}

void UI::on_actionAboutUs_triggered()
{

}

void UI::on_actionStatistics_triggered(bool checked)
{

}

void UI::on_actionConsole_triggered(bool checked)
{

}

void UI::on_actionCreateRoom_triggered()
{

}

void UI::on_actionJoinRoom_triggered(bool checked)
{
    if (_vr) {
        RoomInfoDialog dlg;
        if (dlg.exec() == QDialog::Accepted) {
            vi::vr::PublisherJoinRequest req;
            req.request = "join";
            req.room = dlg.getRoomId();
            req.ptype = "publisher";
            req.display = "ADSL2";
			_vr->setRoomId(req.room);
            _vr->getVideoRoomApi()->join(req, nullptr);
        }
    }
}

void UI::on_actionAudio_triggered(bool checked)
{
	if (!_vr) {
		return;
	}

	if (checked) {
		_vr->unmuteAudio("");
	}
	else {
		_vr->muteAudio("");
	}
}

void UI::on_actionVideo_triggered(bool checked)
{
	if (!_vr) {
		return;
	}

	if (checked) {
		_vr->unmuteVideo("");
	} else {
		_vr->muteVideo("");
	}
}
