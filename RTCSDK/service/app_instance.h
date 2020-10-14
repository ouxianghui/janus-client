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
#include "rtc_base/thread.h"

namespace core {

class AppInstance : public IAppInstance, public core::Singleton<AppInstance>
{
public:
    ~AppInstance() override;

    void initApp() override;

    void clearnup() override;

    std::shared_ptr<IUnifiedFactory> getUnifiedFactory() override;

	std::shared_ptr<vi::WebRTCServiceInterface> getWebrtcService() override;

	std::shared_ptr<vi::ThreadManager> getThreadManager() override;

	std::shared_ptr<vi::TaskQueueManager> getTaskQueueManager() override;

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
	std::shared_ptr<vi::ThreadManager> _threadManager;
	std::shared_ptr<vi::TaskQueueManager> _taskQueueManager;
};

}

#define rtcApp core::AppInstance::instance()
#define UFactory rtcApp->getUnifiedFactory()
#define FetchService(S) UFactory->getBizServiceFactory()->getService<S>()
