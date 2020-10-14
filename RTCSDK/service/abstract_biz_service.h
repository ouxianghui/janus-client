/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#pragma once

#include "i_biz_service.h"
#include "observable.h"
#include "i_unified_factory.h"

namespace core {

class AbstractBizService : public IBizService, public Observable
{
public:
    AbstractBizService(const std::weak_ptr<IUnifiedFactory> unifiedFactory) : _unifiedFactory(unifiedFactory)
    {

    }

    virtual ~AbstractBizService() {}

    void init() override {}

    void cleanup() override {}

protected:
    const std::weak_ptr<IUnifiedFactory> _unifiedFactory;

};

}
