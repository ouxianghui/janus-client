/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#include "app_instance.h"
#include <mutex>
#include <memory>
#include "unified_factory.h"
#include "logger/logger.h"
#include "notification_service.h"
#include "webrtc_service.h"
#include "webrtc_service_interface.h"
#include "webrtc_service_proxy.h"
#include "thread_manager.h"
#include "task_queue_manager.h"

namespace core {

AppInstance::AppInstance()
{
}

AppInstance::~AppInstance()
{
	DLOG("~AppInstance()");
	_webrtcService = nullptr;
	_threadManager = nullptr;
	_taskQueueManager = nullptr;
}

void AppInstance::initApp()
{
	if (!_threadManager) {
		_threadManager = std::make_shared<vi::ThreadManager>();
		_threadManager->init();
	}

	if (!_taskQueueManager) {
		_taskQueueManager = std::make_shared<vi::TaskQueueManager>();
		_taskQueueManager->init();
	}

    // init services here
    installBizServices();

	installWebRTCService();
}

void AppInstance::clearnup()
{
	//auto wrs = FetchService(vi::WebRTCServiceInterface);
	_webrtcService->cleanup();
}

std::shared_ptr<IUnifiedFactory> AppInstance::getUnifiedFactory()
{
    if (!_unifiedFactory) {
        _unifiedFactory = std::make_shared<UnifiedFactory>();
        _unifiedFactory->init();
    }
    return _unifiedFactory;
}

std::shared_ptr<vi::ThreadManager> AppInstance::getThreadManager()
{
	return _threadManager;
}

std::shared_ptr<vi::TaskQueueManager> AppInstance::getTaskQueueManager()
{
	return _taskQueueManager;
}

std::shared_ptr<vi::WebRTCServiceInterface> AppInstance::getWebrtcService()
{
	return _webrtcService;
}

void AppInstance::installBizServices()
{
    auto uf = getUnifiedFactory();

    auto ns = std::make_shared<NotificationService>(uf);
    ns->init();
}

void AppInstance::installWebRTCService()
{
	rtc::Thread* wst = getThreadManager()->getThread(vi::ThreadName::WEBRTC_SERVICE);

	_webrtcService = vi::WebRTCServiceProxy::Create(wst, std::make_shared<vi::WebRTCService>());

	_webrtcService->init();
}

}
