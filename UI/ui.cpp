#include "ui.h"
#include "Service/app_instance.h"
#include "webrtc_service_interface.h"
#include "video_room.h"
#include "message_models.h"
#include "string_utils.h"
#include "webrtc_service_events.h"
#include "x2struct.hpp"
#include "api/media_stream_interface.h"
#include "api/create_peerconnection_factory.h"
#include "api/video_codecs/builtin_video_decoder_factory.h"
#include "api/video_codecs/builtin_video_encoder_factory.h"
#include "api/audio_codecs/builtin_audio_decoder_factory.h"
#include "api/audio_codecs/builtin_audio_encoder_factory.h"
#include "modules/audio_device/include/audio_device.h"
#include "modules/audio_processing/include/audio_processing.h"
#include "modules/video_capture/video_capture_factory.h"
#include "pc/video_track_source.h"
#include "local_video_capture.h"
#include "gl_video_renderer.h"
#include "participant.h"
#include "logger/logger.h"
#include "video_room_listener_proxy.h"
#include <qtoolbar.h>

UI::UI(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	connect(ui.actionStart, &QAction::triggered, this, &UI::onActionStartTriggered);
	connect(ui.actionRegister, &QAction::triggered, this, &UI::onActionRegisterTriggered);

	_videoRoomListenerProxy = std::make_shared<VideoRoomListenerProxy>(this);
	connect(_videoRoomListenerProxy.get(), &VideoRoomListenerProxy::createParticipant, this, &UI::onCreateParticipant, Qt::QueuedConnection);
	connect(_videoRoomListenerProxy.get(), &VideoRoomListenerProxy::updateParticipant, this, &UI::onUpdateParticipant, Qt::QueuedConnection);
	connect(_videoRoomListenerProxy.get(), &VideoRoomListenerProxy::deleteParticipant, this, &UI::onDeleteParticipant, Qt::QueuedConnection);
	connect(_videoRoomListenerProxy.get(), &VideoRoomListenerProxy::createStream, this, &UI::onCreateStream, Qt::QueuedConnection);
	connect(_videoRoomListenerProxy.get(), &VideoRoomListenerProxy::deleteStream, this, &UI::onDeleteStream, Qt::QueuedConnection);


}

UI::~UI()
{
	if (_galleryView) {
		_galleryView->removeAll();
	}
}

void UI::init()
{
	_galleryView = new GalleryView(this);
	setCentralWidget(_galleryView);
}

void UI::onStatus(vi::ServiceStauts status)
{
	if (vi::ServiceStauts::UP == status) {

	}
}

void UI::onCreateParticipant(std::shared_ptr<vi::Participant> participant)
{

}

void UI::onUpdateParticipant(std::shared_ptr<vi::Participant> participant)
{

}

void UI::onDeleteParticipant(std::shared_ptr<vi::Participant> participant)
{

}

void UI::onCreateStream(uint64_t pid, rtc::scoped_refptr<webrtc::MediaStreamInterface> stream)
{
	for (auto track : stream->GetVideoTracks()) {
		GLVideoRenderer* renderer = new GLVideoRenderer(_galleryView);
		renderer->init();
		renderer->show();

		// TODO:
		static int64_t id = -1;
		std::shared_ptr<ContentView> view = std::make_shared<ContentView>(++id, track, renderer);
		view->init();

		_galleryView->insertView(view);
	}
}

void UI::onDeleteStream(uint64_t pid, rtc::scoped_refptr<webrtc::MediaStreamInterface> stream)
{
}

void UI::onActionStartTriggered()
{
	if (!_vr) {
		//auto wrs = FetchService(vi::WebRTCServiceInterface);
		auto wrs = rtcApp->getWebrtcService();
		_vr = std::make_shared<vi::VideoRoom>(wrs);
		_vr->init();
		_vr->addListener(_videoRoomListenerProxy);
	}
	_vr->attach();
}

void UI::onActionNameTriggered()
{

}

void UI::onActionRegisterTriggered()
{
	vi::RegisterRequest request;
	request.request = "join";
	request.room = 1234;
	request.ptype = "publisher";
	request.display = "jackie";// ui.lineEditUserName->text().toStdString();

	if (_vr) {
		std::shared_ptr<vi::SendMessageEvent> event = std::make_shared<vi::SendMessageEvent>();
		auto lambda = [](bool success, const std::string& message) {
			TLOG("message: {}", message.c_str());
		};
		std::shared_ptr<vi::EventCallback> callback = std::make_shared<vi::EventCallback>(lambda);
		event->message = x2struct::X::tojson(request);
		event->callback = callback;
		_vr->sendMessage(event);
	}
}

void UI::closeEvent(QCloseEvent* event)
{
	//_vr->hangup(true);
	if (_galleryView) {
		_galleryView->removeAll();
	}
}


void UI::on_actionJanusGateway_triggered()
{

}

void UI::on_actionMyProfile_triggered()
{

}

void UI::on_actionConnectJanus_triggered()
{

}

void UI::on_actionVideoRoom_triggered(bool checked)
{

}

void UI::on_actionVideoCall_triggered(bool checked)
{

}

void UI::on_actionTextRoom_triggered(bool checked)
{

}

void UI::on_actionVoiceMail_triggered(bool checked)
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
