/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#pragma once

#include <memory>
#include <string>
namespace vi {
    class VideoRoomClientInterface;
    class IEngineEventHandler;

    struct Options {
        std::string serverUrl;
    };

    class IRTCEngine {
    public:
        virtual ~IRTCEngine() {}

        virtual void init() = 0;

        virtual void destroy() = 0;

        virtual void registerEventHandler(std::weak_ptr<IEngineEventHandler> handler) = 0;

        virtual void unregisterEventHandler(std::weak_ptr<IEngineEventHandler> handler) = 0;

        virtual void setOptions(const Options& opts) = 0;

        virtual void startup() = 0;

        virtual void shutdown() = 0;

        virtual std::shared_ptr<VideoRoomClientInterface> createVideoRoomClient() = 0;

    };

}
