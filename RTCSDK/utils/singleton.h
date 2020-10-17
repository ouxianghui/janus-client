#pragma once

#include <memory>
#include <mutex>

namespace core {

	// 线程安全的懒汉单例模版
	template<typename T>
	class Singleton {
	public:
		virtual ~Singleton() {}
		static std::unique_ptr<T>& instance()
		{
			static std::unique_ptr<T> _instance = nullptr;
			static std::once_flag ocf;
			std::call_once(ocf, [](){
				_instance.reset(new T);
			});
			return _instance;
		}
	};

}
