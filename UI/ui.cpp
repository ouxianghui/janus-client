#include "ui.h"
#include <QDockWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QToolButton>
#include "Service/app_instance.h"
#include "signaling_service_interface.h"
#include "video_room_client.h"
#include "message_models.h"
#include "string_utils.h"
#include "signaling_events.h"
#include "api/media_stream_interface.h"
#include "gl_video_renderer.h"
#include "participant.h"
#include "logger/logger.h"
#include "video_room_event_adapter.h"
#include "video_room_dialog.h"
#include "i_video_room_api.h"
#include "room_info_dialog.h"
#include "media_event_adapter.h"
#include "participants_event_adapter.h"
#include "media_controller.h"
#include "participants_controller.h"
#include "video_room_client_proxy.h"

UI::UI(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
    //ui.mainToolBar->setFixedHeight(64);
    this->setWindowState(Qt::WindowMaximized);

	_videoRoomEventAdapter = std::make_shared<VideoRoomEventAdapter>(this);
	connect(_videoRoomEventAdapter.get(), &VideoRoomEventAdapter::create, this, &UI::onCreate, Qt::QueuedConnection);
	connect(_videoRoomEventAdapter.get(), &VideoRoomEventAdapter::join, this, &UI::onJoin, Qt::QueuedConnection);
	connect(_videoRoomEventAdapter.get(), &VideoRoomEventAdapter::leave, this, &UI::onLeave, Qt::QueuedConnection);

	_mediaEventAdapter = std::make_shared<MediaEventAdapter>(this);
	connect(_mediaEventAdapter.get(), &MediaEventAdapter::mediaStatus, this, &UI::onMediaStatus, Qt::QueuedConnection);
	connect(_mediaEventAdapter.get(), &MediaEventAdapter::createVideoTrack, this, &UI::onCreateVideoTrack, Qt::QueuedConnection);
	connect(_mediaEventAdapter.get(), &MediaEventAdapter::removeVideoTrack, this, &UI::onRemoveVideoTrack, Qt::QueuedConnection);
	connect(_mediaEventAdapter.get(), &MediaEventAdapter::localAudioMuted, this, &UI::onLocalAudioMuted, Qt::QueuedConnection);
	connect(_mediaEventAdapter.get(), &MediaEventAdapter::localVideoMuted, this, &UI::onLocalVideoMuted, Qt::QueuedConnection);
	connect(_mediaEventAdapter.get(), &MediaEventAdapter::remoteAudioMuted, this, &UI::onRemoteAudioMuted, Qt::QueuedConnection);
	connect(_mediaEventAdapter.get(), &MediaEventAdapter::remoteVideoMuted, this, &UI::onRemoteVideoMuted, Qt::QueuedConnection);


	_participantsEventAdapter = std::make_shared<ParticipantsEventAdapter>(this);
	connect(_participantsEventAdapter.get(), &ParticipantsEventAdapter::createParticipant, this, &UI::onCreateParticipant, Qt::QueuedConnection);
	connect(_participantsEventAdapter.get(), &ParticipantsEventAdapter::updateParticipant, this, &UI::onUpdateParticipant, Qt::QueuedConnection);
	connect(_participantsEventAdapter.get(), &ParticipantsEventAdapter::removeParticipant, this, &UI::onRemoveParticipant, Qt::QueuedConnection);

    ui.actionAudio->setEnabled(false);
    ui.actionVideo->setEnabled(false);
}

UI::~UI()
{
	if (_galleryView) {
		_galleryView->removeAll();
	}

	//if (_selfContentView) {
	//	_selfContentView->cleanup();
	//}
}

void UI::init()
{
	auto ss = rtcApp->getSignalingService();

	auto vrc = std::make_shared<vi::VideoRoomClient>(ss);
	_vrc = vi::VideoRoomClientProxy::Create(TMgr->thread("plugin-client"), vrc);
	_vrc->init();

	_vrc->registerEventHandler(_videoRoomEventAdapter);
	_vrc->mediaContrller()->registerEventHandler(_mediaEventAdapter);
	_vrc->participantsController()->registerEventHandler(_participantsEventAdapter);


	_galleryView = new GalleryView(this);
	setCentralWidget(_galleryView);


    QWidget* dockContentView = new QWidget(this);
    QVBoxLayout* dockContentViewLayout = new QVBoxLayout(dockContentView);
    dockContentView->setLayout(dockContentViewLayout);

    //_selfView = new QWidget(this);
    //QGridLayout* selfViewLayout = new QGridLayout(_selfView);
    //_selfView->setLayout(selfViewLayout);
    //dockContentViewLayout->addWidget(_selfView, 100);

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

	_participantsListView = std::make_shared<ParticipantsListView>(_vrc, this);
	_participantsListView->setFixedWidth(200);
    dockContentViewLayout->addWidget(_participantsListView.get(), 400);

	QDockWidget* dockWidget = new QDockWidget(this);
    dockWidget->setWindowTitle("Participants List");
    dockWidget->setWidget(dockContentView);

	this->addDockWidget(Qt::RightDockWidgetArea, dockWidget);
}

void UI::onSessionStatus(vi::SessionStatus status)
{
	if (vi::SessionStatus::CONNECTED == status) {

	}
}

void UI::onCreate()
{

}

void UI::onJoin()
{

}

void UI::onLeave()
{

}


void UI::onLocalAudioMuted(bool muted)
{

}

void UI::onLocalVideoMuted(bool muted)
{

}

void UI::onRemoteAudioMuted(const std::string& pid, bool muted)
{

}

void UI::onRemoteVideoMuted(const std::string& pid, bool muted)
{

}

void UI::onMediaStatus(bool isActive, const std::string& reason)
{
	//if (isActive) {
	//	ui.actionAudio->setEnabled(true);
	//	ui.actionVideo->setEnabled(true);
	//	if (_vrc) {
	//		if (_vrc->isAudioMuted("")) {
	//			ui.actionAudio->setChecked(false);
	//		}
	//		else {
	//			ui.actionAudio->setChecked(true);
	//		}

	//		if (_vrc->isVideoMuted("")) {
	//			ui.actionVideo->setChecked(false);
	//		}
	//		else {
	//			ui.actionVideo->setChecked(true);
	//		}
	//	}
	//}
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
        //if (_vrc->getId() != pid) {
			GLVideoRenderer* renderer = new GLVideoRenderer(_galleryView);
            renderer->init();
            renderer->show();

            std::shared_ptr<ContentView> view = std::make_shared<ContentView>(pid, track, renderer);
            view->init();

            _galleryView->insertView(view);
   //     }
   //     else {
			//GLVideoRenderer* renderer = new GLVideoRenderer(this);
   //         renderer->init();
   //         renderer->show();

   //         _selfContentView = std::make_shared<ContentView>(pid, track, renderer);
   //         _selfContentView->init();
   //         _selfView->layout()->addWidget(_selfContentView->view());
   //     }
	}
}

void UI::onRemoveVideoTrack(uint64_t pid, rtc::scoped_refptr<webrtc::VideoTrackInterface> track)
{
	_galleryView->removeView(pid);
}

void UI::closeEvent(QCloseEvent* event)
{
	_vrc->detach();
	if (_galleryView) {
		_galleryView->removeAll();
	}
}

void UI::on_actionAttachRoom_triggered(bool checked)
{
	if (checked) {
		if (_vrc) {
			_vrc->attach();
		}
	}
	else {

	}
}

void UI::on_actionPublishStream_triggered(bool checked)
{
	if (!_vrc) {
		return;
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
    if (_vrc) {
        RoomInfoDialog dlg;
        if (dlg.exec() == QDialog::Accepted) {
            auto req = std::make_shared<vi::vr::PublisherJoinRequest>();
            req->request = "join";
            req->room = dlg.getRoomId();
            req->ptype = "publisher";
            req->display = "ADSL32";
            _vrc->join(req);
        }
    }
}

void UI::on_actionAudio_triggered(bool checked)
{
	if (!_vrc) {
		return;
	}

	//if (checked) {
	//	_vrc->unmuteAudio("");
	//}
	//else {
	//	_vrc->muteAudio("");
	//}
}

void UI::on_actionVideo_triggered(bool checked)
{
	if (!_vrc) {
		return;
	}

	//if (checked) {
	//	_vrc->unmuteVideo("");
	//} else {
	//	_vrc->muteVideo("");
	//}
}
