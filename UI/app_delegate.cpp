#include "app_delegate.h"
#include "rtc_engine_factory.h"
#include "service/i_rtc_engine.h"

AppDelegate::AppDelegate()
{

}

AppDelegate::~AppDelegate()
{

}

void AppDelegate::init()
{
	if (!_rtcEngine) {
		_rtcEngine = vi::RTCEngineFactory::createEngine();
		_rtcEngine->init();
	}
}

void AppDelegate::destroy()
{
	if (_rtcEngine) {
		_rtcEngine->destroy();
	}
}

std::shared_ptr<vi::IRTCEngine> AppDelegate::getRtcEngine()
{
	return _rtcEngine;
}
