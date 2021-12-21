/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#include "unified_factory.h"
#include "utils/service_factory.hpp"
#include "signaling_client.h"
#include "signaling_client_interface.h"
#include "signaling_client_proxy.h"
#include "utils/thread_provider.h"

namespace vi {
	UnifiedFactory::UnifiedFactory()
	{

	}

	void UnifiedFactory::init()
	{
		if (!_threadProvider) {
			_threadProvider = std::make_unique<vi::ThreadProvider>();
			_threadProvider->init();
			_threadProvider->create({ "signaling-service", "plugin-client", "message-transport", "capture-session" });
		}

		if (!_serviceFactory) {
			_serviceFactory = std::make_shared<vi::ServiceFactory>();
			_serviceFactory->init();
		}

		if (!_signalingClient) {
			_signalingClient = vi::SignalingClientProxy::Create(_threadProvider->thread("signaling-service"), std::make_shared<vi::SignalingClient>());
			_signalingClient->init();
		}
	}

	void UnifiedFactory::destroy()
	{
		if (_signalingClient) {
			_signalingClient->cleanup();
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

	std::shared_ptr<vi::SignalingClientInterface> UnifiedFactory::getSignalingClient()
	{
		return _signalingClient;
	}
}