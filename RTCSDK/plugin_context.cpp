#include "plugin_context.h"

namespace vi {
	rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> PluginContext::pcf;
	std::unique_ptr<rtc::Thread> PluginContext::signaling;
	std::unique_ptr<rtc::Thread> PluginContext::worker;
	std::unique_ptr<rtc::Thread> PluginContext::network;
}