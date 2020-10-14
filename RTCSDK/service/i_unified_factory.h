/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#pragma once

#include <memory>
#include "i_biz_service_factory.h"

namespace core {

class IUnifiedFactory {
public:
    virtual ~IUnifiedFactory() {}

    virtual void init() = 0;

    virtual std::shared_ptr<IBizServiceFactory> getBizServiceFactory() = 0;
};

}
