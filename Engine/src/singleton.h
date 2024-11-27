#ifndef SINGLETON_H
#define SINGLETON_H

//Used for global access to scene and manager objects
//if you want multiple scenes in one application for some reason
//you'll need to make sceneobjects and resourceproperties know what
//scene they're in as part of the refactoring


//in your class's cpp file you need to put like:
//template<> SceneManager* Singleton<SceneManager>::_instance_ = nullptr;
//because the variable has to "live" somewhere, thats just how it works

template <class T>
class Singleton
{
public:
    static T* Instance() {
        return _instance_;
    }

protected:
    Singleton() {
        assert(_instance_ == nullptr);
        _instance_ = static_cast<T*>(this);
    }

    ~Singleton() {
        assert(_instance_ == this);
        _instance_ = nullptr;
    }

private:
    static T* _instance_;
};

#endif // SINGLETON_H
