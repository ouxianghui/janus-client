/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#include "rtc_engine.h"
#include <mutex>
#include <memory>
#include "unified_factory.h"
#include "video_room_client.h"
#include "video_room_client_proxy.h"

namespace vi {
	RTCEngine::RTCEngine()
	{

	}

	RTCEngine::~RTCEngine()
	{

	}

	void RTCEngine::init()
	{
		_unifiedFactory = std::make_shared<UnifiedFactory>();
		_unifiedFactory->init();

		_unifiedFactory->getSignalingClient()->registerObserver(shared_from_this());
	}

	void RTCEngine::destroy()
	{
		if (_unifiedFactory) {
			_unifiedFactory->getSignalingClient()->unregisterObserver(shared_from_this());
			_unifiedFactory->destroy();
		}
	}

	void RTCEngine::registerEventHandler(std::weak_ptr<IEngineEventHandler> handler)
	{
		Observable::addBizObserver<IEngineEventHandler>(_observers, handler);
	}

	void RTCEngine::unregisterEventHandler(std::weak_ptr<IEngineEventHandler> handler)
	{
		Observable::removeBizObserver<IEngineEventHandler>(_observers, handler);
	}

	void RTCEngine::setOptions(const Options& opts)
	{
		_options = opts;
	}

	void RTCEngine::startup()
	{
		auto ss = uFactory->getSignalingClient();
		ss->connect(_options.serverUrl);
	}

	void RTCEngine::shutdown()
	{

	}

	std::shared_ptr<VideoRoomClientInterface> RTCEngine::createVideoRoomClient()
	{
		auto ss = uFactory->getSignalingClient();
		return VideoRoomClientProxy::Create(TMgr->thread("plugin-client"), std::make_shared<vi::VideoRoomClient>(ss));
	}

	std::shared_ptr<IUnifiedFactory> RTCEngine::getUnifiedFactory()
	{
		return _unifiedFactory;
	}

	void RTCEngine::onSessionStatus(SessionStatus status)
	{
		Observable::notifyObserver4Change<IEngineEventHandler>(_observers, [status](const auto& observer) {
			EngineStatus es = status == SessionStatus::CONNECTED ? EngineStatus::CONNECTED : EngineStatus::DISCONNECTED;
			observer->onStatus(es);
		});
	}
}