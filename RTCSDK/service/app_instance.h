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

namespace core {

class AppInstance : public IAppInstance, public core::Singleton<AppInstance>
{
public:
    ~AppInstance() override;

    void initApp() override;

    void clearnup() override;

    std::shared_ptr<IUnifiedFactory> getUnifiedFactory() override;

	std::shared_ptr<vi::WebRTCServiceInterface> getWebrtcService() override;

protected:
    void installBizServices();

	void installWebRTCService();

private:
    AppInstance();
    AppInstance(const AppInstance&) = delete;
    AppInstance& operator=(const AppInstance&) = delete;

private:
    friend class core::Singleton<AppInstance>;
    std::shared_ptr<IUnifiedFactory> _unifiedFactory;
	std::shared_ptr<vi::WebRTCServiceInterface> _webrtcService;
};

}

#define rtcApp core::AppInstance::instance()
#define UFactory rtcApp->getUnifiedFactory()
#define FetchService(S) UFactory->getBizServiceFactory()->getService<S>()
