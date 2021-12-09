
#pragma once

#include <memory>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <future>
#include "utils/thread_provider.h"


/// Example usage 1:
///
/// class IMyService {
/// public:
///     virtual void func1() = 0;
///     virtual void func2(const std::string& value) = 0;
///     virtual std::string func3(bool arg1) = 0;
///  };
///
/// class MyService : public IMyService, public std::enable_shared_from_this<MyService> {
/// public:
///     void func1() override {}
///     void func2(const std::string& value) override {}
///     std::string func3(bool arg1) override { return "func3"; }
/// };
///
/// BEGIN_PROXY_MAP(MyService)
///   PROXY_METHOD0(void, func1)
///   PROXY_METHOD1(void, func2, const std::string&)
///   PROXY_METHOD1(std::string, func3, bool)
/// END_PROXY_MAP()
///
/// TEST_F(InterfaceProxyTest, TestProxy)
/// {
///     auto proxy = MyServiceProxy::create(std::make_shared<MyService>(), "CORE");
///     proxy->func1();
/// }
///
/// Example usage 2:
///
/// class XServiceInterface {
/// public:
///     virtual void func1() = 0;
///     virtual void func2(const std::string& value) = 0;
///     virtual std::string func3(bool arg1) = 0;
///  };
///
/// class MyService2 : public XServiceInterface, public std::enable_shared_from_this<MyService2> {
/// public:
///     void func1() override {}
///     void func2(const std::string& value) override {}
///     std::string func3(bool arg1) override { return "func3"; }
/// };
///
/// BEGIN_PROXY_MAP_INTERFACE(MyService2, XServiceInterface)
///   PROXY_METHOD0(void, func1)
///   PROXY_METHOD1(void, func2, const std::string&)
///   PROXY_METHOD1(std::string, func3, bool)
/// END_PROXY_MAP()
///
/// TEST_F(InterfaceProxyTest, TestProxy)
/// {
///     auto proxy = MyService2Proxy::create(std::make_shared<MyService2>(), "CORE");
///     proxy->func1();
/// }


namespace vi {

template <typename R>
class ReturnType {
public:
    template <typename T, typename M, typename... Args>
    void invoke(T* t, M m, Args&&... args) {
        _result = (t->*m)(std::forward<Args>(args)...);
    }
    R get() { return std::move(_result); }
private:
    R _result;
};

template <>
class ReturnType<void> {
public:
    template <typename T, typename M, typename... Args>
    void invoke(T* t, M m, Args&&... args) {
        (t->*m)(std::forward<Args>(args)...);
    }
    void get() {}
};

template <typename T, typename R, typename... Args>
class MethodCall {
public:
    typedef R (T::*Method)(Args...);
    MethodCall(T* t, Method m, Args&&... args)
        : _target(t)
        , _method(m)
        , _args(std::forward_as_tuple(std::forward<Args>(args)...)) {
            
        }
    
    R marshal(const std::string& name) {
		const auto task = [&]() {
			this->invoke(std::index_sequence_for<Args...>());
			_promises.set_value();
		};

		rtc::Thread* thread = TMgr->thread(name);
		assert(thread);
		if (thread->IsCurrent()) {
            task();
		}
		else {
			thread->PostTask(RTC_FROM_HERE, task);
			std::future<void> future = _promises.get_future();
            future.get();
		}

        return _result.get();
    }
    
private:
    template <size_t... Is>
    void invoke(std::index_sequence<Is...>) {
            _result.invoke(_target, _method, std::move(std::get<Is>(_args))...);
    }
    T* _target;
    Method _method;
    ReturnType<R> _result;
    std::tuple<Args&&...> _args;
    std::promise<void> _promises;
};

#define PROXY_STRINGIZE_IMPL(x) #x

#define PROXY_STRINGIZE(x) PROXY_STRINGIZE_IMPL(x)

#define PROXY_MAP_BOILERPLATE(c)                                                    \
    template <class INTERNAL_CLASS>                                                 \
    class c##ProxyWithInternal;                                                     \
    typedef c##ProxyWithInternal<I##c> c##Proxy;                                    \
    template <class INTERNAL_CLASS>                                                 \
    class c##ProxyWithInternal : public I##c {                                      \
    protected:                                                                      \
    typedef I##c C;                                                                 \
    const char* className_ = PROXY_STRINGIZE(c);                                    \
    public:                                                                         \
    std::shared_ptr<INTERNAL_CLASS> internal() { return _c; }

#define PROXY_MAP_BOILERPLATE_INTERFACE(c, interface)                               \
    template <class INTERNAL_CLASS>                                                 \
    class c##ProxyWithInternal;                                                     \
    typedef c##ProxyWithInternal<interface> c##Proxy;                               \
    template <class INTERNAL_CLASS>                                                 \
    class c##ProxyWithInternal : public interface {                                 \
    protected:                                                                      \
    typedef interface C;                                                            \
    const char* className_ = PROXY_STRINGIZE(c);                                    \
    public:                                                                         \
    std::shared_ptr<INTERNAL_CLASS> internal() { return _c; }


#define END_PROXY_MAP() \
};

#define CALL_THREAD_PROXY_MAP_BOILERPLATE(c)                                        \
    public:                                                                         \
    c##ProxyWithInternal(std::shared_ptr<INTERNAL_CLASS> c,                         \
    const std::string& threadName)                                                  \
    : _threadName(threadName)                                                      \
    , _c(c) {}                                                                     \
    private:                                                                        \
    const std::string _threadName;

#define SHARED_PROXY_MAP_BOILERPLATE(c)                                             \
    public:                                                                         \
    ~c##ProxyWithInternal() {                                                       \
    MethodCall<c##ProxyWithInternal, void> call(                                    \
    this, &c##ProxyWithInternal::destroyInternal);                                  \
    call.marshal(_threadName);                                                     \
    }                                                                               \
    private:                                                                        \
    void destroyInternal() { _c = nullptr; }                                       \
    std::shared_ptr<INTERNAL_CLASS> _c;

#define BEGIN_PROXY_MAP(c)                                                          \
    PROXY_MAP_BOILERPLATE(c)                                                        \
    CALL_THREAD_PROXY_MAP_BOILERPLATE(c)                                            \
    SHARED_PROXY_MAP_BOILERPLATE(c)                                                 \
    public:                                                                         \
    static std::shared_ptr<c##ProxyWithInternal> create(                            \
    std::shared_ptr<INTERNAL_CLASS> c, const std::string& threadName) {             \
    return std::make_shared<c##ProxyWithInternal>(c, threadName);                   \
}

#define BEGIN_PROXY_MAP_INTERFACE(c, interface)                                     \
    PROXY_MAP_BOILERPLATE_INTERFACE(c, interface)                                   \
    CALL_THREAD_PROXY_MAP_BOILERPLATE(c)                                            \
    SHARED_PROXY_MAP_BOILERPLATE(c)                                                 \
    public:                                                                         \
    static std::shared_ptr<c##ProxyWithInternal> create(                            \
    std::shared_ptr<INTERNAL_CLASS> c, const std::string& threadName) {             \
    return std::make_shared<c##ProxyWithInternal>(c, threadName);                   \
}

#define PROXY_METHOD0(r, method)                                                    \
    r method() override {                                                           \
    MethodCall<C, r> call(_c.get(), &C::method);                                   \
    return call.marshal(_threadName);                                              \
}

#define PROXY_METHOD1(r, method, t1)                                                \
    r method(t1 a1) override {                                                      \
    MethodCall<C, r, t1> call(_c.get(), &C::method, std::move(a1));                \
    return call.marshal(_threadName);                                              \
}

#define PROXY_METHOD2(r, method, t1, t2)                                            \
    r method(t1 a1, t2 a2) override {                                               \
    MethodCall<C, r, t1, t2> call(_c.get(), &C::method, std::move(a1),             \
    std::move(a2));                                                                 \
    return call.marshal(_threadName);                                              \
}

#define PROXY_METHOD3(r, method, t1, t2, t3)                                        \
    r method(t1 a1, t2 a2, t3 a3) override {                                        \
    MethodCall<C, r, t1, t2, t3> call(_c.get(), &C::method, std::move(a1),         \
    std::move(a2), std::move(a3));                                                  \
    return call.marshal(_threadName);                                              \
}

#define PROXY_METHOD4(r, method, t1, t2, t3, t4)                                    \
    r method(t1 a1, t2 a2, t3 a3, t4 a4) override {                                 \
    MethodCall<C, r, t1, t2, t3, t4> call(_c.get(), &C::method, std::move(a1),     \
    std::move(a2), std::move(a3),                                                   \
    std::move(a4));                                                                 \
    return call.marshal(_threadName);                                              \
}

#define PROXY_METHOD5(r, method, t1, t2, t3, t4, t5)                                \
    r method(t1 a1, t2 a2, t3 a3, t4 a4, t5 a5) override {                          \
    MethodCall<C, r, t1, t2, t3, t4, t5> call(_c.get(), &C::method, std::move(a1), \
    std::move(a2), std::move(a3),                                                   \
    std::move(a4), std::move(a5));                                                  \
    return call.marshal(_threadName);                                              \
}

}
