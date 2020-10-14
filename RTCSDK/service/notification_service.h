/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#pragma once

#include <memory>
#include "i_notification_service.h"

namespace core {

class NotificationService : public INotificationService, public std::enable_shared_from_this<NotificationService>
{
public:
    NotificationService(const std::weak_ptr<IUnifiedFactory> unifiedFactory);
    ~NotificationService() {}

    void init() override;

    void cleanup() override;

private:
};


}
