#include "participants_controller.h"
#include "participant.h"

namespace vi {
    ParticipantsContrller::ParticipantsContrller() {

    }

    ParticipantsContrller::~ParticipantsContrller() 
    {

    }

    void ParticipantsContrller::init()
    {

    }

    void ParticipantsContrller::destroy()
    {

    }

    void ParticipantsContrller::registerEventHandler(std::shared_ptr<IParticipantsControlEventHandler> handler)
    {
        UniversalObservable<IParticipantsControlEventHandler>::addWeakObserver(handler, std::string("main"));
    }

    void ParticipantsContrller::unregisterEventHandler(std::shared_ptr<IParticipantsControlEventHandler> handler)
    {
        UniversalObservable<IParticipantsControlEventHandler>::removeObserver(handler);
    }

    std::shared_ptr<Participant> ParticipantsContrller::participant(int64_t pid)
    {
        return nullptr;
    }

    std::vector<std::shared_ptr<Participant>> ParticipantsContrller::participantList()
    {
        return {};
    }

    void ParticipantsContrller::kick(int64_t pid)
    {

    }

    std::map<int64_t, std::shared_ptr<Participant>>& ParticipantsContrller::participants()
    {
        return _participantsMap;
    }

    void ParticipantsContrller::createParticipant(std::shared_ptr<Participant> participant)
    {
        _participantsMap[participant->getId()] = participant;
        UniversalObservable<IParticipantsControlEventHandler>::notifyObservers([wself = weak_from_this(), participant](const auto& observer) {
            observer->onCreateParticipant(participant);
        });
    }

    void ParticipantsContrller::removeParticipant(int64_t id)
    {
        if (_participantsMap.find(id) != _participantsMap.end()) {
            UniversalObservable<IParticipantsControlEventHandler>::notifyObservers([wself = weak_from_this(), participant = _participantsMap[id]](const auto& observer) {
                observer->onRemoveParticipant(participant);
            });
        }
    }
}