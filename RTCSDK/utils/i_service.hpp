
#pragma once

namespace vi
{
    class IService
    {
    public:
        virtual ~IService() = default;
        
    public:
        virtual void init() = 0;
        
        virtual void destroy() = 0;
    };
}
