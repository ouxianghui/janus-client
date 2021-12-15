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
	if (!_serviceFactory) {
		_serviceFactory = std::make_shared<vi::ServiceFactory>();
		_serviceFactory->init();
	}

	if (!_webrtcService) {
		_webrtcService = vi::SignalingServiceProxy::Create(TMgr->thread("service"), std::make_shared<vi::SignalingService>());
		_webrtcService->init();
	}
}

void UnifiedFactory::destroy()
{
	if (_webrtcService) {
		_webrtcService->cleanup();
	}

	if (_serviceFactory) {
		_serviceFactory->destroy();
	}
}

std::shared_ptr<vi::IServiceFactory> UnifiedFactory::getServiceFactory()
{
    return _serviceFactory;
}

std::shared_ptr<vi::SignalingServiceInterface> UnifiedFactory::getSignalingService()
{
	return _webrtcService;
}
