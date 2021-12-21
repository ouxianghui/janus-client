#pragma once

#include <memory>
#include "utils/singleton.h"

namespace vi {
    class IRTCEngine;
}

class AppDelegate
    : public vi::Singleton<AppDelegate>
{
public:
    ~AppDelegate();

    void init();

    void destroy();

    std::shared_ptr<vi::IRTCEngine> getRtcEngine();

private:
    AppDelegate();

    AppDelegate(const AppDelegate&) = delete;

    AppDelegate& operator=(const AppDelegate&) = delete;

private:
    friend class vi::Singleton<AppDelegate>;

    std::shared_ptr<vi::IRTCEngine> _rtcEngine;
};

#define appDelegate AppDelegate::instance()