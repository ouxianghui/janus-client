/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#pragma once

#include "utils/singleton.h"
#include "i_app_instance.h"
#include "i_unified_factory.h"
#include <memory>
#include <sstream>

class AppInstance : public IAppInstance, public vi::Singleton<AppInstance>
{
public:
    ~AppInstance() override;

    void init() override;

    void destroy() override;

    std::shared_ptr<IUnifiedFactory> getUnifiedFactory() override;

	std::shared_ptr<vi::SignalingServiceInterface> getSignalingService() override;

private:
    AppInstance();
    AppInstance(const AppInstance&) = delete;
    AppInstance& operator=(const AppInstance&) = delete;

private:
    friend class vi::Singleton<AppInstance>;

    std::shared_ptr<IUnifiedFactory> _unifiedFactory;
};


#define rtcApp AppInstance::instance()
#define UFactory rtcApp->getUnifiedFactory()
#define FetchService(S) UFactory->getServiceFactory()->getService<S>()