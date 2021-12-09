
#pragma once

#include <string>

namespace vi {

    class INotification
    {
    public:
        virtual ~INotification() = default;
        
        virtual std::string type() { return typeid(*this).name(); };
    };

}
