#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_ui.h"
#include <memory>
#include "i_webrtc_service_listener.h"
#include "video_room.h"
#include "i_video_room_listener.h"
#include "gl_video_renderer.h"
#include "api/create_peerconnection_factory.h"
#include "gallery_view.h"
#include <QCloseEvent>

namespace vi {
	class Participant;
}

class UI
	: public QMainWindow
	, public vi::IWebRTCServiceListener
	, public vi::IVideoRoomListener
	, public std::enable_shared_from_this<UI>
{
	Q_OBJECT

public:
	UI(QWidget *parent = Q_NULLPTR);

	~UI();

	void init();

	std::shared_ptr<GLVideoRenderer> _renderer;

private:
	// IWebRTCServiceListener
	void onStatus(vi::ServiceStauts status) override;

	// IVideoRoomListener
	void onCreateParticipant(std::shared_ptr<vi::Participant> participant) override;
																	   
	void onUpdateParticipant(std::shared_ptr<vi::Participant> participant) override;
																	   
	void onDeleteParticipant(std::shared_ptr<vi::Participant> participant) override;

	void onCreateStream(int64_t pid, rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) override;

	void onDeleteStream(int64_t pid, rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) override;

private slots:
	void onActionStartTriggered();

	void onActionNameTriggered();

	void onActionRegisterTriggered();

	void closeEvent(QCloseEvent* event);

private:
	Ui::UIClass ui;

	std::shared_ptr<vi::VideoRoom> _vr;

	GalleryView* _galleryView;
};
