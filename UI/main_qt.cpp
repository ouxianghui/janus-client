/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#include "ui.h"
#include <QtWidgets/QApplication>
#include "rtc_base/checks.h"
#include "rtc_base/ssl_adapter.h"
#include "rtc_base/win32_socket_init.h"
#include "rtc_base/win32_socket_server.h"
#include <QObject>
#include <memory>
#include "Service/app_instance.h"
#include "webrtc_service_interface.h"
#include <QSurfaceFormat>

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
#include "task_scheduler.h"
#include "logger/logger_installer.h"

using namespace core;

static void registerMetaTypes()
{
	qRegisterMetaType<std::function<void()>>("std::function<void()>");
	qRegisterMetaType<std::string>("std::string");
	qRegisterMetaType<std::vector<std::string>>("std::vector<std::string>");

	//// For Network Model
	//qRegisterMetaType<std::shared_ptr<NetworkRequest>>("std::shared_ptr<NetworkRequest>");
	//qRegisterMetaType<std::shared_ptr<INetworkCallback>>("std::shared_ptr<INetworkCallback>");
	//qRegisterMetaType<std::shared_ptr<NetworkResponse>>("std::shared_ptr<NetworkResponse>");

	//// For Thread Model
	//qRegisterMetaType<int64_t>("int64_t");
	//qRegisterMetaType<std::shared_ptr<ITaskScheduler>>("std::shared_ptr<ITaskScheduler>");
	//qRegisterMetaType<std::shared_ptr<ITask>>("std::shared_ptr<ITask>");
}

static void initOpenGL() {
	//QSurfaceFormat format;
	//format.setDepthBufferSize(24);
	//format.setStencilBufferSize(8);
	//format.setVersion(4, 4);
	//format.setProfile(QSurfaceFormat::CoreProfile);
	//QSurfaceFormat::setDefaultFormat(format);
}

int main(int argc, char *argv[])
{
	vi::LoggerInstaller::instance()->install();

	registerMetaTypes();

	rtc::WinsockInitializer winsock_init;
	rtc::Win32SocketServer w32_ss;
	rtc::Win32Thread w32_thread(&w32_ss);
	rtc::ThreadManager::Instance()->SetCurrentThread(&w32_thread);

	rtc::InitializeSSL();

	QApplication a(argc, argv);

	initOpenGL();

	rtcApp->initApp();

	auto wrs = rtcApp->getWebrtcService();
	std::shared_ptr<UI> w = std::make_shared<UI>();
	wrs->addListener(w);
	w->show();

	w->init();

	int ret = a.exec();

	rtcApp->clearnup();

	rtc::CleanupSSL();

	vi::LoggerInstaller::instance()->uninstall();

	return ret; 
}
