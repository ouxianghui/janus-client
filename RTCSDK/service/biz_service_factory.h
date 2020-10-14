/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#pragma once

#include <memory>
#include <map>
#include "i_unified_factory.h"
#include "i_biz_service.h"
#include "i_biz_service_factory.h"

namespace core {

class BizServiceFactory : public IBizServiceFactory, public std::enable_shared_from_this<BizServiceFactory>
{
public:
    BizServiceFactory(const std::shared_ptr<IUnifiedFactory>& unifiedFactory);

    void init() override;

    void registerService(const std::string& key , const std::shared_ptr<IBizService> &service) override;

    void unregisterService(const std::string& key) override;

    std::shared_ptr<IBizService> getService(const std::string& key) override;

    void cleanup() override;

private:
    const std::weak_ptr<IUnifiedFactory> _unifiedFactory;
    std::map<std::string, std::shared_ptr<IBizService>> _serviceMap;
};

}
