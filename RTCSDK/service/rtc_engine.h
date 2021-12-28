/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#pragma once

#include <memory>
#include <vector>
#include "utils/singleton.h"
#include "i_rtc_engine.h"
#include "i_unified_factory.h"
#include "utils/observable.h"
#include "i_engine_event_handler.h"
#include "i_signaling_client_observer.h"
#include "api/scoped_refptr.h"
#include "api/create_peerconnection_factory.h"


namespace vi {
    class RTCEngine 
        : public IRTCEngine
        , public ISignalingClientObserver
        , public Observable
        , public std::enable_shared_from_this<RTCEngine>
    {
    public:
        static std::shared_ptr<IRTCEngine> instance()
        {
            static std::shared_ptr<IRTCEngine> _instance;
            static std::once_flag ocf;
            std::call_once(ocf, []() {
                _instance.reset(new RTCEngine());
            });
            return _instance;
        }

        ~RTCEngine() override;

        void init() override;

        void destroy() override;

        void registerEventHandler(std::weak_ptr<IEngineEventHandler> handler) override;

        void unregisterEventHandler(std::weak_ptr<IEngineEventHandler> handler) override;

        void setOptions(const Options& opts) override;

        void startup() override;

        void shutdown() override;

        std::shared_ptr<VideoRoomClientInterface> createVideoRoomClient() override;

        std::shared_ptr<IUnifiedFactory> getUnifiedFactory();

    protected:
        void onSessionStatus(SessionStatus status) override;

    private:
        RTCEngine();

        RTCEngine(const RTCEngine&) = delete;

        RTCEngine& operator=(const RTCEngine&) = delete;

    private:
        friend class Singleton<RTCEngine>;

        std::shared_ptr<IUnifiedFactory> _unifiedFactory;

        std::vector<std::weak_ptr<IEngineEventHandler>> _observers;

        Options _options;

        rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> _pcf;
        std::unique_ptr<rtc::Thread> _signaling;
        std::unique_ptr<rtc::Thread> _worker;
        std::unique_ptr<rtc::Thread> _network;
    };

}

#define rtcEngine std::dynamic_pointer_cast<RTCEngine>(vi::RTCEngine::instance())
#define uFactory rtcEngine->getUnifiedFactory()
#define fetchService(S) uFactory->getServiceFactory()->getService<S>()