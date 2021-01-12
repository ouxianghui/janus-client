/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#include "plugin_client.h"
#include "task_scheduler.h"
#include "thread_manager.h"
#include "logger/logger.h"

namespace vi {
	PluginClient::PluginClient(std::shared_ptr<WebRTCServiceInterface> wrs)
	{
		_pluginContext = std::make_shared<PluginContext>(wrs);

		_rtcStatsTaskScheduler = std::make_shared<vi::TaskScheduler>();
	}

	PluginClient::~PluginClient()
	{
		stopStatsReport();
	}

	void PluginClient::setHandleId(int64_t handleId)
	{
		_pluginContext->handleId = handleId;
	}

	const std::shared_ptr<PluginContext>& PluginClient::pluginContext() const
	{
		return _pluginContext;
	}

	void PluginClient::attach()
	{
		if (auto wrs = _pluginContext->webrtcService.lock()) {
			if (wrs->status() == ServiceStauts::UP) {
				wrs->attach(_pluginContext->plugin, _pluginContext->opaqueId, shared_from_this());
			}
		}
	}

	int32_t PluginClient::remoteVolume()
	{
		if (auto wrs = _pluginContext->webrtcService.lock()) {
			if (wrs->status() == ServiceStauts::UP) {
				return wrs->remoteVolume(_pluginContext->handleId);
			}
		}
		return 0;
	}

	int32_t PluginClient::localVolume()
	{
		if (auto wrs = _pluginContext->webrtcService.lock()) {
			if (wrs->status() == ServiceStauts::UP) {
				return wrs->localVolume(_pluginContext->handleId);
			}
		}
		return 0;
	}

	bool PluginClient::isAudioMuted()
	{
		if (auto wrs = _pluginContext->webrtcService.lock()) {
			if (wrs->status() == ServiceStauts::UP) {
				return wrs->isAudioMuted(_pluginContext->handleId);
			}
		}
		return false;
	}

	bool PluginClient::isVideoMuted()
	{
		if (auto wrs = _pluginContext->webrtcService.lock()) {
			if (wrs->status() == ServiceStauts::UP) {
				return wrs->isVideoMuted(_pluginContext->handleId);
			}
		}
		return false;
	}

	bool PluginClient::muteAudio()
	{
		if (auto wrs = _pluginContext->webrtcService.lock()) {
			if (wrs->status() == ServiceStauts::UP) {
				return wrs->muteAudio(_pluginContext->handleId);
			}
		}
		return false;
	}

	bool PluginClient::muteVideo()
	{
		if (auto wrs = _pluginContext->webrtcService.lock()) {
			if (wrs->status() == ServiceStauts::UP) {
				return wrs->muteVideo(_pluginContext->handleId);
			}
		}
		return false;
	}

	bool PluginClient::unmuteAudio()
	{
		if (auto wrs = _pluginContext->webrtcService.lock()) {
			if (wrs->status() == ServiceStauts::UP) {
				return wrs->unmuteAudio(_pluginContext->handleId);
			}
		}
		return false;
	}

	bool PluginClient::unmuteVideo()
	{
		if (auto wrs = _pluginContext->webrtcService.lock()) {
			if (wrs->status() == ServiceStauts::UP) {
				return wrs->unmuteVideo(_pluginContext->handleId);
			}
		}
		return false;
	}

	std::string PluginClient::getBitrate()
	{
		if (auto wrs = _pluginContext->webrtcService.lock()) {
			if (wrs->status() == ServiceStauts::UP) {
				return wrs->getBitrate(_pluginContext->handleId);
			}
		}
		return false;
	}

	void PluginClient::sendMessage(std::shared_ptr<SendMessageEvent> event)
	{
		if (auto wrs = _pluginContext->webrtcService.lock()) {
			if (wrs->status() == ServiceStauts::UP) {
				wrs->sendMessage(_pluginContext->handleId, event);
			}
		}
	}

	void PluginClient::sendData(std::shared_ptr<SendDataEvent> event)
	{
		if (auto wrs = _pluginContext->webrtcService.lock()) {
			if (wrs->status() == ServiceStauts::UP) {
				wrs->sendData(_pluginContext->handleId, event);
			}
		}
	}

	void PluginClient::sendDtmf(std::shared_ptr<SendDtmfEvent> event)
	{
		if (auto wrs = _pluginContext->webrtcService.lock()) {
			if (wrs->status() == ServiceStauts::UP) {
				wrs->sendDtmf(_pluginContext->handleId, event);
			}
		}
	}

	void PluginClient::createOffer(std::shared_ptr<PrepareWebRTCEvent> event)
	{
		if (auto wrs = _pluginContext->webrtcService.lock()) {
			if (wrs->status() == ServiceStauts::UP) {
				wrs->createOffer(_pluginContext->handleId, event);
			}
		}
	}

	void PluginClient::createAnswer(std::shared_ptr<PrepareWebRTCEvent> event)
	{
		if (auto wrs = _pluginContext->webrtcService.lock()) {
			if (wrs->status() == ServiceStauts::UP) {
				wrs->createAnswer(_pluginContext->handleId, event);
			}
		}
	}

	void PluginClient::handleRemoteJsep(std::shared_ptr<PrepareWebRTCPeerEvent> event)
	{
		if (auto wrs = _pluginContext->webrtcService.lock()) {
			if (wrs->status() == ServiceStauts::UP) {
				wrs->handleRemoteJsep(_pluginContext->handleId, event);
			}
		}
	}

	void PluginClient::hangup(bool sendRequest)
	{
		if (auto wrs = _pluginContext->webrtcService.lock()) {
			if (wrs->status() == ServiceStauts::UP) {
				wrs->hangup(_pluginContext->handleId, sendRequest);
			}
		}
	}

	void PluginClient::detach(std::shared_ptr<DetachEvent> event)
	{
		if (auto wrs = _pluginContext->webrtcService.lock()) {
			if (wrs->status() == ServiceStauts::UP) {
				wrs->detach(_pluginContext->handleId, event);
			}
		}
	}

	void PluginClient::startStatsReport()
	{
		_rtcStatsTaskId = _rtcStatsTaskScheduler->schedule([wself = weak_from_this()]() {
			auto self = wself.lock();
			if (!self) {
				return;
			}
			const auto& context = self->pluginContext()->webrtcContext;
			if (!context->statsObserver) {
				context->statsObserver = StatsObserver::create();

				auto socb = std::make_shared<StatsCallback>([wself](const rtc::scoped_refptr<const webrtc::RTCStatsReport>& report) {
					DLOG("RTC Stats Report: {}", report->ToJson());
					auto self = wself.lock();
					if (!self) {
						return;
					}

					auto wrs = self->pluginContext()->webrtcService.lock();
					if (!wrs) {
						return;
					}

					if (wrs->status() != ServiceStauts::UP) {
						return;
					}

					auto eventHandlerThread = TMgr->thread(vi::ThreadName::WORKER);
					eventHandlerThread->PostTask(RTC_FROM_HERE, [wself, report]() {
						if (auto self = wself.lock()) {
							self->onStatsReport(report);
						}
					});
				});
				context->statsObserver->setCallback(socb);
			}
			if (context->pc) {
				context->pc->GetStats(context->statsObserver.get());
			}
		}, 5000, true);
	}

	void PluginClient::stopStatsReport()
	{
		if (_rtcStatsTaskScheduler) {
			_rtcStatsTaskScheduler->cancelAll();
		}
	}
}


