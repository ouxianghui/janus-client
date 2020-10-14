 // Example usage:
 //
 // class TestInterface {
 //  public:
 //   std::string FooA() = 0;
 //   std::string FooB(bool arg1) const = 0;
 //   std::string FooC(bool arg1) = 0;
 //  };
 //
 // Note that return types can not be a const reference.
 //
 // class Test : public TestInterface, public std::enable_shared_from_this<Test> {
 // ... implementation of the interface.
 // };
 //
 // BEGIN_WEAK_PROXY_MAP(Test)
 //   PROXY_SIGNALING_THREAD_DESTRUCTOR()
 //   WEAK_PROXY_METHOD0(std::string, FooA)
 //   WEAK_PROXY_CONSTMETHOD1(std::string, FooB, arg1)
 //   WEAK_PROXY__METHOD1(std::string, FooC, arg1)
 // END_WEAK_PROXY_MAP()
 //
 // Where the destructor and all other methods are invoked on the thread
 //
 // The proxy can be created using
 //
 //   TestProxy::Create(Thread* thread, TestInterface*).
 //

#pragma once

#include <memory>
#include <string>
#include <utility>
#include "rtc_base/event.h"
#include "rtc_base/message_handler.h"
#include "rtc_base/system/rtc_export.h"
#include "rtc_base/thread.h"

namespace rtc {
	class Location;
}

namespace vi {

	template <typename R>
	class ReturnType {
	public:
		template <typename C, typename M>
		void Invoke(C* c, M m) {
			r_ = (c->*m)();
		}
		template <typename C, typename M>
		void Invoke(std::weak_ptr<C> c, M m) {
			if (auto cl = c.lock()) {
				r_ = (cl.get()->*m)();
			}
		}
		template <typename C, typename M, typename T1>
		void Invoke(std::weak_ptr<C> c, M m, T1 a1) {
			if (auto cl = c.lock()) {
				r_ = (cl.get()->*m)(std::move(a1));
			}
		}
		template <typename C, typename M, typename T1, typename T2>
		void Invoke(std::weak_ptr<C> c, M m, T1 a1, T2 a2) {
			if (auto cl = c.lock()) {
				r_ = (cl.get()->*m)(std::move(a1), std::move(a2));
			}
		}
		template <typename C, typename M, typename T1, typename T2, typename T3>
		void Invoke(std::weak_ptr<C> c, M m, T1 a1, T2 a2, T3 a3) {
			if (auto cl = c.lock()) {
				r_ = (cl.get()->*m)(std::move(a1), std::move(a2), std::move(a3));
			}
		}
		template <typename C,
			typename M,
			typename T1,
			typename T2,
			typename T3,
			typename T4>
			void Invoke(std::weak_ptr<C> c, M m, T1 a1, T2 a2, T3 a3, T4 a4) {
			if (auto cl = c.lock()) {
				r_ = (cl.get()->*m)(std::move(a1), std::move(a2), std::move(a3), std::move(a4));
			}
		}
		template <typename C,
			typename M,
			typename T1,
			typename T2,
			typename T3,
			typename T4,
			typename T5>
			void Invoke(std::weak_ptr<C> c, M m, T1 a1, T2 a2, T3 a3, T4 a4, T5 a5) {
			if (auto cl = c.lock()) {
				r_ = (cl.get()->*m)(std::move(a1), std::move(a2), std::move(a3), std::move(a4), std::move(a5));
			}
		}

		R moved_result() { return std::move(r_); }

	private:
		R r_;
	};

	template <>
	class ReturnType<void> {
	public:
		template <typename C, typename M>
		void Invoke(C* c, M m) {
			(c->*m)();
		}
		template <typename C, typename M>
		void Invoke(std::weak_ptr<C> c, M m) {
			if (auto cl = c.lock()) {
				(cl.get()->*m)();
			}
		}
		template <typename C, typename M, typename T1>
		void Invoke(std::weak_ptr<C> c, M m, T1 a1) {
			if (auto cl = c.lock()) {
				(cl.get()->*m)(std::move(a1));
			}
		}
		template <typename C, typename M, typename T1, typename T2>
		void Invoke(std::weak_ptr<C> c, M m, T1 a1, T2 a2) {
			if (auto cl = c.lock()) {
				(cl.get()->*m)(std::move(a1), std::move(a2));
			}
		}
		template <typename C, typename M, typename T1, typename T2, typename T3>
		void Invoke(std::weak_ptr<C> c, M m, T1 a1, T2 a2, T3 a3) {
			if (auto cl = c.lock()) {
				(cl.get()->*m)(std::move(a1), std::move(a2), std::move(a3));
			}
		}

		void moved_result() {}
	};

	namespace internal {

		class SynchronousMethodCall : public rtc::MessageData,
			public rtc::MessageHandler {
		public:
			explicit SynchronousMethodCall(rtc::MessageHandler* proxy);
			~SynchronousMethodCall() override;

			void Invoke(const rtc::Location& posted_from, rtc::Thread* t);

		private:
			void OnMessage(rtc::Message*) override;

			rtc::Event e_;
			rtc::MessageHandler* proxy_;
		};

	}  // namespace internal

	template <typename C, typename R>
	class DestroyCall : public rtc::Message, public rtc::MessageHandler {
	public:
		typedef R(C::*Method)();
		DestroyCall(C* c, Method m) : c_(c), m_(m) {}

		R Marshal(const rtc::Location& posted_from, rtc::Thread* t) {
			internal::SynchronousMethodCall(this).Invoke(posted_from, t);
			return r_.moved_result();
		}

	private:
		void OnMessage(rtc::Message*) { r_.Invoke(c_, m_); }

		C* c_;
		Method m_;
		ReturnType<R> r_;
	};

	template <typename C, typename R>
	class MethodCall0 : public rtc::Message, public rtc::MessageHandler {
	public:
		typedef R(C::*Method)();
		MethodCall0(std::weak_ptr<C> c, Method m) : c_(c), m_(m) {}

		R Marshal(const rtc::Location& posted_from, rtc::Thread* t) {
			internal::SynchronousMethodCall(this).Invoke(posted_from, t);
			return r_.moved_result();
		}

	private:
		void OnMessage(rtc::Message*) { r_.Invoke(c_, m_); }

		std::weak_ptr<C> c_;
		Method m_;
		ReturnType<R> r_;
	};

	template <typename C, typename R>
	class ConstMethodCall0 : public rtc::Message, public rtc::MessageHandler {
	public:
		typedef R(C::*Method)() const;
		ConstMethodCall0(std::weak_ptr<C> c, Method m) : c_(c), m_(m) {}

		R Marshal(const rtc::Location& posted_from, rtc::Thread* t) {
			internal::SynchronousMethodCall(this).Invoke(posted_from, t);
			return r_.moved_result();
		}

	private:
		void OnMessage(rtc::Message*) { r_.Invoke(c_, m_); }

		std::weak_ptr<C> c_;
		Method m_;
		ReturnType<R> r_;
	};

	template <typename C, typename R, typename T1>
	class MethodCall1 : public rtc::Message, public rtc::MessageHandler {
	public:
		typedef R(C::*Method)(T1 a1);
		MethodCall1(std::weak_ptr<C> c, Method m, T1 a1) : c_(c), m_(m), a1_(std::move(a1)) {}

		R Marshal(const rtc::Location& posted_from, rtc::Thread* t) {
			internal::SynchronousMethodCall(this).Invoke(posted_from, t);
			return r_.moved_result();
		}

	private:
		void OnMessage(rtc::Message*) { r_.Invoke(c_, m_, std::move(a1_)); }

		std::weak_ptr<C> c_;
		Method m_;
		ReturnType<R> r_;
		T1 a1_;
	};

	template <typename C, typename R, typename T1>
	class ConstMethodCall1 : public rtc::Message, public rtc::MessageHandler {
	public:
		typedef R(C::*Method)(T1 a1) const;
		ConstMethodCall1(std::weak_ptr<C> c, Method m, T1 a1) : c_(c), m_(m), a1_(std::move(a1)) {}

		R Marshal(const rtc::Location& posted_from, rtc::Thread* t) {
			internal::SynchronousMethodCall(this).Invoke(posted_from, t);
			return r_.moved_result();
		}

	private:
		void OnMessage(rtc::Message*) { r_.Invoke(c_, m_, std::move(a1_)); }

		std::weak_ptr<C> c_;
		Method m_;
		ReturnType<R> r_;
		T1 a1_;
	};

	template <typename C, typename R, typename T1, typename T2>
	class MethodCall2 : public rtc::Message, public rtc::MessageHandler {
	public:
		typedef R(C::*Method)(T1 a1, T2 a2);
		MethodCall2(std::weak_ptr<C> c, Method m, T1 a1, T2 a2)
			: c_(c), m_(m), a1_(std::move(a1)), a2_(std::move(a2)) {}

		R Marshal(const rtc::Location& posted_from, rtc::Thread* t) {
			internal::SynchronousMethodCall(this).Invoke(posted_from, t);
			return r_.moved_result();
		}

	private:
		void OnMessage(rtc::Message*) {
			r_.Invoke(c_, m_, std::move(a1_), std::move(a2_));
		}

		std::weak_ptr<C> c_;
		Method m_;
		ReturnType<R> r_;
		T1 a1_;
		T2 a2_;
	};

	template <typename C, typename R, typename T1, typename T2, typename T3>
	class MethodCall3 : public rtc::Message, public rtc::MessageHandler {
	public:
		typedef R(C::*Method)(T1 a1, T2 a2, T3 a3);
		MethodCall3(std::weak_ptr<C> c, Method m, T1 a1, T2 a2, T3 a3)
			: c_(c),
			m_(m),
			a1_(std::move(a1)),
			a2_(std::move(a2)),
			a3_(std::move(a3)) {}

		R Marshal(const rtc::Location& posted_from, rtc::Thread* t) {
			internal::SynchronousMethodCall(this).Invoke(posted_from, t);
			return r_.moved_result();
		}

	private:
		void OnMessage(rtc::Message*) {
			r_.Invoke(c_, m_, std::move(a1_), std::move(a2_), std::move(a3_));
		}

		std::weak_ptr<C> c_;
		Method m_;
		ReturnType<R> r_;
		T1 a1_;
		T2 a2_;
		T3 a3_;
	};

	template <typename C,
		typename R,
		typename T1,
		typename T2,
		typename T3,
		typename T4>
		class MethodCall4 : public rtc::Message, public rtc::MessageHandler {
		public:
			typedef R(C::*Method)(T1 a1, T2 a2, T3 a3, T4 a4);
			MethodCall4(std::weak_ptr<C> c, Method m, T1 a1, T2 a2, T3 a3, T4 a4)
				: c_(c),
				m_(m),
				a1_(std::move(a1)),
				a2_(std::move(a2)),
				a3_(std::move(a3)),
				a4_(std::move(a4)) {}

			R Marshal(const rtc::Location& posted_from, rtc::Thread* t) {
				internal::SynchronousMethodCall(this).Invoke(posted_from, t);
				return r_.moved_result();
			}

		private:
			void OnMessage(rtc::Message*) {
				r_.Invoke(c_, m_, std::move(a1_), std::move(a2_), std::move(a3_),
					std::move(a4_));
			}

			std::weak_ptr<C> c_;
			Method m_;
			ReturnType<R> r_;
			T1 a1_;
			T2 a2_;
			T3 a3_;
			T4 a4_;
	};

	template <typename C,
		typename R,
		typename T1,
		typename T2,
		typename T3,
		typename T4,
		typename T5>
		class MethodCall5 : public rtc::Message, public rtc::MessageHandler {
		public:
			typedef R(C::*Method)(T1 a1, T2 a2, T3 a3, T4 a4, T5 a5);
			MethodCall5(std::weak_ptr<C> c, Method m, T1 a1, T2 a2, T3 a3, T4 a4, T5 a5)
				: c_(c),
				m_(m),
				a1_(std::move(a1)),
				a2_(std::move(a2)),
				a3_(std::move(a3)),
				a4_(std::move(a4)),
				a5_(std::move(a5)) {}

			R Marshal(const rtc::Location& posted_from, rtc::Thread* t) {
				internal::SynchronousMethodCall(this).Invoke(posted_from, t);
				return r_.moved_result();
			}

		private:
			void OnMessage(rtc::Message*) {
				r_.Invoke(c_, m_, std::move(a1_), std::move(a2_), std::move(a3_),
					std::move(a4_), std::move(a5_));
			}

			std::weak_ptr<C> c_;
			Method m_;
			ReturnType<R> r_;
			T1 a1_;
			T2 a2_;
			T3 a3_;
			T4 a4_;
			T5 a5_;
	};

// Helper macros to reduce code duplication.
#define WEAK_PROXY_MAP_DEFINE_BOILERPLATE(c)			\
  template <class INTERNAL_CLASS>						\
  class c##ProxyWithInternal;							\
  typedef c##ProxyWithInternal<c##Interface> c##Proxy;	\
  template <class INTERNAL_CLASS>						\
  class c##ProxyWithInternal : public c##Interface {    \
  protected:											\
  typedef c##Interface C;                               

// clang-format off
// clang-format would put the semicolon alone,
// leading to a presubmit error (cpplint.py)
#define END_WEAK_PROXY_MAP() \
  };
// clang-format on

#define WEAK_PROXY_MAP_THREAD_BOILERPLATE(c)									\
 public:																		\
  c##ProxyWithInternal(rtc::Thread* thread, std::shared_ptr<INTERNAL_CLASS> c)	\
      : thread_(thread), c_(c) {}												\
																				\
 private:																		\
  mutable rtc::Thread* thread_;


#define WEAK_PROXY_MAP_METHOD_BOILERPLATE(c)					       \
 public:														       \
  ~c##ProxyWithInternal() {										       \
	DestroyCall<c##ProxyWithInternal, void> call(					   \
		this, &c##ProxyWithInternal::DestroyInternal);				   \
    call.Marshal(RTC_FROM_HERE, destructor_thread());			       \
  }																       \
																       \
 private:														       \
  void DestroyInternal() { c_ = nullptr; }							   \
  std::shared_ptr<INTERNAL_CLASS> c_;

#define BEGIN_WEAK_PROXY_MAP(c)									\
  WEAK_PROXY_MAP_DEFINE_BOILERPLATE(c)							\
  WEAK_PROXY_MAP_THREAD_BOILERPLATE(c)							\
  WEAK_PROXY_MAP_METHOD_BOILERPLATE(c)							\
 public:														\
  static std::shared_ptr<c##ProxyWithInternal> Create(			\
      rtc::Thread* thread, std::shared_ptr<INTERNAL_CLASS> c) {	\
    return std::make_shared<c##ProxyWithInternal>(thread, c);	\
  }

#define WEAK_PROXY_THREAD_DESTRUCTOR()							\
 private:														\
  rtc::Thread* destructor_thread() const { return thread_; }	\
																\
 public:  // NOLINTNEXTLINE

#define WEAK_PROXY_METHOD0(r, method)                      \
  r method() override {                                    \
    MethodCall0<C, r> call(c_, &C::method);                \
    return call.Marshal(RTC_FROM_HERE, thread_);		   \
  }

#define WEAK_PROXY_CONSTMETHOD0(r, method)                 \
  r method() const override {                              \
    ConstMethodCall0<C, r> call(c_, &C::method);           \
    return call.Marshal(RTC_FROM_HERE, thread_);		   \
  }

#define WEAK_PROXY_METHOD1(r, method, t1)                      \
  r method(t1 a1) override {                                   \
    MethodCall1<C, r, t1> call(c_, &C::method, std::move(a1)); \
    return call.Marshal(RTC_FROM_HERE, thread_);			   \
  }

#define WEAK_PROXY_CONSTMETHOD1(r, method, t1)                      \
  r method(t1 a1) const override {                                  \
    ConstMethodCall1<C, r, t1> call(c_, &C::method, std::move(a1)); \
    return call.Marshal(RTC_FROM_HERE, thread_);					\
  }

#define WEAK_PROXY_METHOD2(r, method, t1, t2)                     \
  r method(t1 a1, t2 a2) override {                               \
    MethodCall2<C, r, t1, t2> call(c_, &C::method, std::move(a1), \
                                   std::move(a2));                \
    return call.Marshal(RTC_FROM_HERE, thread_);				  \
  }

#define WEAK_PROXY_METHOD3(r, method, t1, t2, t3)                     \
  r method(t1 a1, t2 a2, t3 a3) override {                            \
    MethodCall3<C, r, t1, t2, t3> call(c_, &C::method, std::move(a1), \
                                       std::move(a2), std::move(a3)); \
    return call.Marshal(RTC_FROM_HERE, thread_);					  \
  }

#define WEAK_PROXY_METHOD4(r, method, t1, t2, t3, t4)                     \
  r method(t1 a1, t2 a2, t3 a3, t4 a4) override {                         \
    MethodCall4<C, r, t1, t2, t3, t4> call(c_, &C::method, std::move(a1), \
                                           std::move(a2), std::move(a3),  \
                                           std::move(a4));                \
    return call.Marshal(RTC_FROM_HERE, thread_);						  \
  }

#define WEAK_PROXY_METHOD5(r, method, t1, t2, t3, t4, t5)                     \
  r method(t1 a1, t2 a2, t3 a3, t4 a4, t5 a5) override {                      \
    MethodCall5<C, r, t1, t2, t3, t4, t5> call(c_, &C::method, std::move(a1), \
                                               std::move(a2), std::move(a3),  \
                                               std::move(a4), std::move(a5)); \
    return call.Marshal(RTC_FROM_HERE, thread_);							  \
  }

}  
