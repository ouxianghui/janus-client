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

namespace core {

class UnifiedFactory : public IUnifiedFactory, public std::enable_shared_from_this<UnifiedFactory>
{
public:
    UnifiedFactory();

    void init() override;

    std::shared_ptr<IBizServiceFactory> getBizServiceFactory() override;   

private:
    std::shared_ptr<IBizServiceFactory> _bizServiceFactory = nullptr;
};

}

