#ifndef  SINGLETON_H
#define  SINGLETON_H
#if defined(DLL_EXPORTS)
#define DLL_API __declspec(dllexport)
#else
#define DLL_API __declspec(dllimport)
#endif
template<class Base>
class Singleton
{
public:
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;
    virtual ~Singleton() {}
private:
    inline static Singleton& getInstance() {
        static Singleton instance;
        return instance;
    }
    Singleton() {}
};

#endif //  SINGLETON_H
