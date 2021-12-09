
#pragma once

#include <memory>
#include <string>
#include "object_factory.hpp"

namespace vi
{
    class IService;

    class IServiceFactory : public ObjectFactory<IService>
    {
    public:
        virtual ~IServiceFactory() = default;

    public:
        virtual void init() = 0;

        virtual void destroy() = 0;

        virtual void registerService(const std::string& key, const std::shared_ptr<IService>& service) = 0;

        virtual void unregisterService(const std::string& key) = 0;

        template<class T>
        std::shared_ptr<T> getService() {
            return std::dynamic_pointer_cast<T>(getObject(typeid(T).name()));
        }        
    };
}

