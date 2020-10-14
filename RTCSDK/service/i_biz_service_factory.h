/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#pragma once

#include <memory>
#include <string>
#include "i_biz_service.h"

namespace core {

class IBizServiceFactory
{
public:
    virtual ~IBizServiceFactory() {}

    virtual void init() = 0;

    virtual void registerService(const std::string& key , const std::shared_ptr<IBizService> &service) = 0;

    virtual void unregisterService(const std::string& key) = 0;

    virtual std::shared_ptr<IBizService> getService(const std::string& key) = 0;

    virtual void cleanup() = 0;

    template<class T>
    std::shared_ptr<T> getService() {
        return std::dynamic_pointer_cast<T>(getService(typeid(T).name()));
    }
};

}
