/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#pragma once

#include <memory>

namespace vi {
	class SignalingServiceInterface;
}

class IUnifiedFactory;
class NetworkRequestManager;

class IAppInstance {
public:
    virtual ~IAppInstance() {}

    virtual void init() = 0;

    virtual void destroy() = 0;

    virtual std::shared_ptr<IUnifiedFactory> getUnifiedFactory() = 0;

	virtual std::shared_ptr<vi::SignalingServiceInterface> getSignalingService() = 0;
};

