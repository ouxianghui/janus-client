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
#include "utils/thread_provider.h"
#include "utils/task_queue_provider.h"


AppInstance::AppInstance()
{
}

AppInstance::~AppInstance()
{
}

void AppInstance::init()
{
	vi::Logger::init();

	initThreadProvider();

	initTaskQueueProvider();

	_unifiedFactory = std::make_shared<UnifiedFactory>();
	_unifiedFactory->init();

	registerServices();
}

void AppInstance::destroy()
{
	if (_unifiedFactory) {
		_unifiedFactory->destroy();
	}

	vi::Logger::destroy();
}

std::shared_ptr<IUnifiedFactory> AppInstance::getUnifiedFactory()
{
    return _unifiedFactory;
}

void AppInstance::initThreadProvider()
{
	TMgr->init();
	TMgr->create({ "service", "worker" });
}

void AppInstance::initTaskQueueProvider()
{
	//QMgr->init();
	//QMgr->create({ {"service_queue", webrtc::TaskQueueFactory::Priority::NORMAL}, {"core_queue", webrtc::TaskQueueFactory::Priority::NORMAL}, {"worker_queue", webrtc::TaskQueueFactory::Priority::NORMAL} });
}

void AppInstance::registerServices()
{
	auto sf = _unifiedFactory->getServiceFactory();
	assert(sf != nullptr);

	//sf->registerService(typeid(IBizService).name(), std::make_shared<BizService>());
}

void AppInstance::unregisterServices()
{
	auto sf = _unifiedFactory->getServiceFactory();
	assert(sf != nullptr);

	//sf->unregisterService(typeid(IBizService).name());
}

std::shared_ptr<vi::WebRTCServiceInterface> AppInstance::getWebrtcService()
{
	if (_unifiedFactory) {
		return _unifiedFactory->getWebrtcService();
	}

	return nullptr;
}
