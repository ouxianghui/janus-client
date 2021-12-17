/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#pragma once

#include <memory>

namespace vi {
    class ThreadProvider;
    class IServiceFactory;
    class SignalingServiceInterface;
}

class IUnifiedFactory {
public:
    virtual ~IUnifiedFactory() {}

    virtual void init() = 0;

	virtual void destroy() = 0;

    virtual std::unique_ptr<vi::ThreadProvider>& getThreadProvider() = 0;

    virtual std::shared_ptr<vi::IServiceFactory> getServiceFactory() = 0;

	virtual std::shared_ptr<vi::SignalingServiceInterface> getSignalingService() = 0;
};

