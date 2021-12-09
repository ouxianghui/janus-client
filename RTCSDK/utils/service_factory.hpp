
#pragma once

#include <memory>
#include "i_service_factory.hpp"

namespace vi
{
    class IService;
    class ServiceFactory : public IServiceFactory, public std::enable_shared_from_this<ServiceFactory>
    {
    public:
        ServiceFactory();
        
        void init() override;
        
        void destroy() override; 

        void registerService(const std::string& key, const std::shared_ptr<IService>& service) override;

        void unregisterService(const std::string& key) override;
    };
}
