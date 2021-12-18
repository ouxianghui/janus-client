/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#include "unified_factory.h"
#include "utils/service_factory.hpp"
#include "signaling_service.h"
#include "signaling_service_interface.h"
#include "signaling_service_proxy.h"
#include "utils/thread_provider.h"

UnifiedFactory::UnifiedFactory()
{

}

void UnifiedFactory::init()
{
	if (!_threadProvider) {
		_threadProvider = std::make_unique<vi::ThreadProvider>();
		_threadProvider->init();
		_threadProvider->create({ "signaling-service", "plugin-client", "message-transport", "capture-session"});
	}

	if (!_serviceFactory) {
		_serviceFactory = std::make_shared<vi::ServiceFactory>();
		_serviceFactory->init();
	}

	if (!_signalingService) {
		_signalingService = vi::SignalingServiceProxy::Create(_threadProvider->thread("signaling-service"), std::make_shared<vi::SignalingService>());
		_signalingService->init();
	}
}


void UnifiedFactory::destroy()
{
	if (_signalingService) {
		_signalingService->cleanup();
	}

	if (_serviceFactory) {
		_serviceFactory->destroy();
	}

	if (_threadProvider) {
		_threadProvider->destroy();
	}
}

std::unique_ptr<vi::ThreadProvider>& UnifiedFactory::getThreadProvider()
{
	return _threadProvider;
}

std::shared_ptr<vi::IServiceFactory> UnifiedFactory::getServiceFactory()
{
    return _serviceFactory;
}

std::shared_ptr<vi::SignalingServiceInterface> UnifiedFactory::getSignalingService()
{
	return _signalingService;
}
