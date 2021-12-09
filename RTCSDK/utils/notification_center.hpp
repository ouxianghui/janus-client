
#pragma once

#include <memory>
#include <list>
#include "rtc_base/deprecated/recursive_critical_section.h"

namespace vi {

    /// Main Features:
    /// 1. Thread safe
    /// 2. Use class method as callback, target class needn't inherit any listener
    /// 3. One target class can observe multiple types of notifications
    /// 4. The observer can only receive the type of notification it subscribes to (The observer subscribes the 'KeyValueNotification' will not accept the other types of notifications, better performance)
    /// 5. Can specify which thread to execute the callback on (default is MIAN)
    /// 6. Provide default 'KeyValueNotification'
    /// 7. Can implement custom notification types
    ///
    /// e.g:
    ///
    /// 1. Define a target class:
    ///
    /// class MyClass
    /// {
    /// public:
    ///     void onKeyValueChanged(const std::shared_ptr<KeyValueNotification>& kvnf) {}
    ///     void onCustomNotification(const std::shared_ptr<CustomNotification>& cnf) {}
    /// };
    ///
    /// 2. Add & Remove Key-Value Notification Observer:
    ///
    /// auto myClass = std::make_shared<MyClass>();
    /// if (auto nfc = NotificationCenter::defaultCenter()) {
    ///    nfc->addObserver(Observer<MyClass, KeyValueNotification>(myClass, &MyClass::onKeyValueChanged, ThreadName::WORKER));
    ///    nfc->removeObserver(Observer<MyClass, KeyValueNotification>(myClass, &MyClass::onKeyValueChanged));
    /// }
    ///
    /// 3. Post Key-Value Notification:
    ///
    /// if (auto nfc = NotificationCenter::defaultCenter()) {
    ///     auto nf = std::make_shared<KeyValueNotification>("jackie's key", std::string("hello"));
    ///     nfc->postNotification(nf);
    /// }
    ///
    /// 4. Define a custom notification:
    ///
    /// class CustomNotification : public INotification, public std::enable_shared_from_this<CustomNotification>
    /// {
    /// public:
    ///     CustomNotification(int32_t value1, int32_t value2) : m_value1(value1), m_value2(value2) { }
    ///     int32_t value1() const { return m_value1; }
    ///     int32_t value2() const { return m_value2; }
    /// private:
    ///     int32_t m_value1;
    ///     int32_t m_value2;
    /// };
    ///
    /// 5. Add Custom Notification Observer:
    ///
    /// auto myClass = std::make_shared<MyClass>();
    /// if (auto nfc = NotificationCenter::defaultCenter()) {
    ///    nfc->addObserver(Observer<MyClass, CustomNotification>(myClass, &MyClass::onCustomNotification));
    /// }
    ///
    /// 6. Post Custom Notification:
    ///
    /// if (auto nfc = NotificationCenter::defaultCenter()) {
    ///     auto nf = std::make_shared<CustomNotification>(10085, 10086);
    ///     nfc->postNotification(nf);
    /// }
    /// 
    
    class IObserver;
    class INotification;

    class NotificationCenter : public std::enable_shared_from_this<NotificationCenter> {
        
    public:
        NotificationCenter();
            
        virtual ~NotificationCenter() {}
            
        void addObserver(const IObserver& observer);
        
        void removeObserver(const IObserver& observer);
        
        bool hasObserver(const IObserver& observer);
            
        std::size_t numOfObservers();
            
        void clearObserver();
            
        void postNotification(const std::shared_ptr<INotification>& notification);

        static const std::shared_ptr<NotificationCenter>& defaultCenter();
                
    private:
        void notifyObservers(const std::shared_ptr<INotification>& notification);
        
        bool hasObserverInternal(const IObserver& observer);
        
        void removeInvalidObservers();
            
    private:
        rtc::RecursiveCriticalSection _criticalSection;
            
        std::list<std::shared_ptr<IObserver>> _observers;
    };
    
}
