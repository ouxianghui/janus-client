/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#include "biz_service_factory.h"
#include "notification_service.h"


namespace core {

BizServiceFactory::BizServiceFactory(const std::shared_ptr<IUnifiedFactory>& unifiedFactory)
    : _unifiedFactory(unifiedFactory)
{
}

void BizServiceFactory::registerService(const std::string& key , const std::shared_ptr<IBizService> &service)
{
    _serviceMap.insert(std::make_pair(key, service));
}

void BizServiceFactory::unregisterService(const std::string& key)
{
    _serviceMap.erase(key);
}

std::shared_ptr<IBizService> BizServiceFactory::getService(const std::string& key)
{
    auto it = _serviceMap.find(key);
    return it == _serviceMap.end() ? nullptr : it->second;
}

void BizServiceFactory::cleanup()
{
    for (auto item : _serviceMap) {
        if(!item.second) {
            continue;
        }
        item.second->cleanup();
    }
}

void BizServiceFactory::init()
{

}

}
