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
#include "api/video_codecs/builtin_video_decoder_factory.h"
#include "api/video_codecs/builtin_video_encoder_factory.h"
#include "api/audio_codecs/builtin_audio_decoder_factory.h"
#include "api/audio_codecs/builtin_audio_encoder_factory.h"
#include "modules/audio_device/include/audio_device.h"
#include "modules/audio_processing/include/audio_processing.h"

namespace vi {
	RTCEngine::RTCEngine()
	{

	}

	RTCEngine::~RTCEngine()
	{
		_pcf = nullptr;
	}

	void RTCEngine::init()
	{
		if (!_pcf) {
			_signaling = rtc::Thread::Create();
			_signaling->SetName("pc_signaling_thread", nullptr);
			_signaling->Start();
			_worker = rtc::Thread::Create();
			_worker->SetName("pc_worker_thread", nullptr);
			_worker->Start();
			_network = rtc::Thread::CreateWithSocketServer();
			_network->SetName("pc_network_thread", nullptr);
			_network->Start();
			_pcf = webrtc::CreatePeerConnectionFactory(
				_network.get() /* network_thread */,
				_worker.get() /* worker_thread */,
				_signaling.get() /* signaling_thread */,
				nullptr /* default_adm */,
				webrtc::CreateBuiltinAudioEncoderFactory(),
				webrtc::CreateBuiltinAudioDecoderFactory(),
				webrtc::CreateBuiltinVideoEncoderFactory(),
				webrtc::CreateBuiltinVideoDecoderFactory(),
				nullptr /* audio_mixer */,
				nullptr /* audio_processing */);
		}

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
		return VideoRoomClientProxy::Create(TMgr->thread("plugin-client"), std::make_shared<vi::VideoRoomClient>(ss, _pcf));
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