/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#include "app_instance.h"
#include <mutex>
#include <memory>
#include "unified_factory.h"

AppInstance::AppInstance()
{
}

AppInstance::~AppInstance()
{
}

void AppInstance::init()
{
	_unifiedFactory = std::make_shared<UnifiedFactory>();
	_unifiedFactory->init();
}

void AppInstance::destroy()
{
	if (_unifiedFactory) {
		_unifiedFactory->destroy();
	}
}

std::shared_ptr<IUnifiedFactory> AppInstance::getUnifiedFactory()
{
    return _unifiedFactory;
}

std::shared_ptr<vi::SignalingServiceInterface> AppInstance::getSignalingService()
{
	if (_unifiedFactory) {
		return _unifiedFactory->getSignalingService();
	}

	return nullptr;
}
