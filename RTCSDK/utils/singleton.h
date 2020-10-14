#pragma once

#include <memory>
#include <mutex>

namespace core {


//// 单例模版使用示例
/*class MyClass {
public:
    void print()
    {
        std::cout << "call print()" << std::endl;
    }

private:
    MyClass()
    {

    }
    MyClass(const MyClass&) = delete ;
    MyClass& operator=(const MyClass&) = delete;

private:
    friend class core::Singleton<MyClass>;
};

class MyClass2 : public core::Singleton<MyClass2> {
public:
    void print()
    {
        std::cout << "call print()" << std::endl;
    }

private:
    MyClass2()
    {

    }
    MyClas2(const MyClas2s&) = delete ;
    MyClass2& operator=(const MyClass2&) = delete;

private:
    friend class core::Singleton<MyClass>;
};

int main()
{
    core::Singleton<MyClass>::instance()->print();

    //继承的方式更合理：MyClass2 is-a Singleton
    MyClass2::instance()->print();
    return 0;
}
*/

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
