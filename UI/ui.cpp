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
#include "video_room_dialog.h"
#include "i_video_room_api.h"

UI::UI(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

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
		if (!_vr) {
			auto wrs = rtcApp->getWebrtcService();
			_vr = std::make_shared<vi::VideoRoom>(wrs);
			_vr->init();
			_vr->addListener(_videoRoomListenerProxy);
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
		vr::FetchParticipantsRequest req;
		req.room = 1234;
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
		vi::vr::PublisherJoinRequest req;
		req.request = "join";
		req.room = 1234;
		req.ptype = "publisher";
		req.display = "jackie";
		_vr->getVideoRoomApi()->join(req, nullptr);
    }
}
