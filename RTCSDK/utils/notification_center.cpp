
#include "notification_center.hpp"
#include <type_traits>
#include <algorithm>
#include "i_observer.hpp"
#include "thread_provider.h"
#include "absl/types/optional.h"

namespace {
    static std::shared_ptr<vi::NotificationCenter> _defaultCenter = std::make_shared<vi::NotificationCenter>();
}

namespace vi {

    NotificationCenter::NotificationCenter()
    {
    }

    void NotificationCenter::addObserver(const IObserver& observer)
    {
        rtc::CritScope scope(&_criticalSection);
        if (!hasObserverInternal(observer)) {
            _observers.emplace_back(observer.clone());
        }
    }

    void NotificationCenter::removeObserver(const IObserver& observer)
    {
        rtc::CritScope scope(&_criticalSection);
        for (auto it = _observers.begin(); it != _observers.end(); ++it) {
            if (observer.equals(**it)) {
                _observers.erase(it);
                return;
            }
        }
    }

    bool NotificationCenter::hasObserver(const IObserver& observer)
    {
        rtc::CritScope scope(&_criticalSection);
        return hasObserverInternal(observer);
    }

    void NotificationCenter::postNotification(const std::shared_ptr<INotification>& notification)
    {
        notifyObservers(notification);
    }

    void NotificationCenter::clearObserver()
    {
        rtc::CritScope scope(&_criticalSection);
        _observers.clear();
    }

    std::size_t NotificationCenter::numOfObservers()
    {
        rtc::CritScope scope(&_criticalSection);
        return _observers.size();
    }

    bool NotificationCenter::hasObserverInternal(const IObserver& observer)
    {
        for (auto it = _observers.begin(); it != _observers.end(); ++it) {
            if (observer.equals(**it)) {
                return true;
            }
        }
        return false;
    }

    void NotificationCenter::removeInvalidObservers()
    {
        _observers.erase(std::remove_if(_observers.begin(), _observers.end(), [](const auto& observer) {
            return !observer->isValid();
            }), _observers.end());
    }

    void NotificationCenter::notifyObservers(const std::shared_ptr<INotification>& notification)
    {
        decltype(_observers) observers;
        {
            rtc::CritScope scope(&_criticalSection);
            removeInvalidObservers();
            observers = _observers;
        }

        for (const auto& observer : observers) {
            if (observer && observer->shouldAccept(notification)) {
                absl::optional<std::string> scheduleThread = observer->scheduleThread();
                rtc::Thread* thread = TMgr->thread(scheduleThread.value_or(""));
                assert(thread);
                if (thread->IsCurrent()) {
				    observer->notify(notification);
                }
                else {
					thread->PostTask(RTC_FROM_HERE, [obs = std::weak_ptr<IObserver>(observer), notification]() {
						if (auto observer = obs.lock()) {
							observer->notify(notification);
						}
					});
                }
            }
        }
    }

    const std::shared_ptr<NotificationCenter>& NotificationCenter::defaultCenter()
    {
        return _defaultCenter;
    }

}
