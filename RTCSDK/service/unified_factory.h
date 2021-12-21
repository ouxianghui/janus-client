/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#pragma once

#include <memory>
#include <map>
#include "i_unified_factory.h"
#include "utils/i_service.hpp"

namespace vi {
    class UnifiedFactory : public IUnifiedFactory, public std::enable_shared_from_this<UnifiedFactory>
    {
    public:
        UnifiedFactory();

        void init() override;

        void destroy() override;

        std::unique_ptr<vi::ThreadProvider>& getThreadProvider() override;

        std::shared_ptr<vi::IServiceFactory> getServiceFactory() override;

        std::shared_ptr<vi::SignalingClientInterface> getSignalingClient() override;

    private:
        std::unique_ptr<vi::ThreadProvider> _threadProvider;

        std::shared_ptr<vi::IServiceFactory> _serviceFactory;

        std::shared_ptr<vi::SignalingClientInterface> _signalingClient;
    };
}