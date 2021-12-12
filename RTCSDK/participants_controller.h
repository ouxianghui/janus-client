#pragma once

#include "participants_controller_interface.h"
#include "utils/universal_observable.hpp"
#include "i_participants_control_event_handler.h"

namespace vi {

    class ParticipantsContrller
        : public ParticipantsContrllerInterface
        , public UniversalObservable<IParticipantsControlEventHandler>
        , public std::enable_shared_from_this<ParticipantsContrller>
    {
    public:
        ParticipantsContrller();

        ~ParticipantsContrller();

        void init() override;

        void destroy() override;

        void registerEventHandler(std::shared_ptr<IParticipantsControlEventHandler> handler) override;

        void unregisterEventHandler(std::shared_ptr<IParticipantsControlEventHandler> handler) override;

        std::shared_ptr<Participant> participant(int64_t pid) override;

        std::vector<std::shared_ptr<Participant>> participantList() override;

        void kick(int64_t pid) override;


        std::map<int64_t, std::shared_ptr<Participant>>& participants();

        void createParticipant(std::shared_ptr<Participant> participant);

        void removeParticipant(int64_t id);

    private:

        std::map<int64_t, std::shared_ptr<Participant>> _participantsMap;
    };

}