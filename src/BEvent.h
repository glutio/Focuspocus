#ifndef __EVENTFUN_H__
#define __EVENTFUN_H__

#include "BPtr.h"
#include "BList.h"

#define BEVENT(EventName, SenderType, ArgumentType) \
public: \
    typedef BEventDelegate<SenderType, ArgumentType> EventName ## Event; \
    BEventSource<EventName ## Event> on ## EventName; \
protected: \
    void raiseOn ## EventName(ArgumentType arg) { on ## EventName(this, arg); }

template<typename TSender, typename TArgument>
class BEventDelegate {
protected:
  struct Callable {
    virtual ~Callable() {}
    virtual void Call(TSender* sender, TArgument argument) = 0;
    virtual char Type() const = 0;
    virtual bool operator == (const Callable& other) const = 0;
  };

  template<typename TClass>
  struct CallableMethodImpl : public Callable {
    TClass* _instance;
    void (TClass::*_method)(TSender*, TArgument);

    CallableMethodImpl(TClass* instance, void (TClass::*method)(TSender*, TArgument))
      : _instance(instance), _method(method) {}

    virtual void Call(TSender* sender, TArgument argument) {
      (_instance->*_method)(sender, argument);
    }

    virtual char Type() const {
      return 'M';
    }

    virtual bool operator == (const Callable& other) const {
      if (Type() != other.Type()) return false;
      auto callable = (const CallableMethodImpl<TClass>&)other;
      return _instance == callable._instance && _method == callable._method;
    }
  };

  struct CallableFunctionImpl : public Callable {
    void (*_func)(TSender*, TArgument);

    CallableFunctionImpl(void (*func)(TSender*, TArgument))
      : _func(func) { }

    virtual void Call(TSender* sender, TArgument argument) {
      (_func)(sender, argument);
    }

    virtual char Type() const {
      return 'F';
    }

    virtual bool operator == (const Callable& other) const {
      if (Type() != other.Type()) return false;
      auto callable = (CallableFunctionImpl&)other;
      return _func == callable._func;
    }
  };

protected:
  Buratino::BPtr<Callable> _callable;  // smart pointer

public:
  typedef TSender SenderType;
  typedef TArgument ArgumentType;

  template<typename TClass>
  BEventDelegate(TClass* instance, void (TClass::*method)(TSender* sender, TArgument argument))
    : _callable(new CallableMethodImpl<TClass>(instance, method)) {}

  BEventDelegate(void (*func)(TSender* sender, TArgument argument))
    : _callable(new CallableFunctionImpl(func)) {}

  BEventDelegate()
    : _callable(0) {}

  ~BEventDelegate() {
  }

  void operator()(TSender* sender, TArgument argument) {
    if (_callable) {
      _callable->Call(sender, argument);
    }
  }

protected:
  explicit operator bool() const {
    return (bool)_callable;
  }

  bool operator == (const BEventDelegate<TSender, TArgument>& other) const {
    return *_callable == *other._callable;
  }

  template<typename>
  friend class BEventSource;
};

template<typename T>
class BEventSource
{
protected:
  Buratino::BList<T> _list;

public:
  BEventSource() {}
  BEventSource(int numEvents) 
    : _list(numEvents) {}
  
  void operator += (T delegate)
  {
    auto k = _list.Length();
    for(unsigned i = 0; i < _list.Length(); ++i) {
      if (_list[i] == delegate) {
        return;
      }
      if (!_list[i]) {
        k = i;
      }
    }

    if (k == _list.Length()) {
      _list.Add(delegate);
    }
    else {
      _list[k] = delegate;
    }
  }

  void operator += (void (*delegate)(typename T::SenderType*, typename T::ArgumentType)) {
    *this += T(delegate);
  }

  void operator -= (T delegate) {
    for (unsigned i = 0; i < _list.Length(); ++i) {
      if (_list[i] == delegate) {
        _list[i] = T();
      }
    }
  }

  void operator -= (void (*delegate)(typename T::SenderType*, typename T::ArgumentType)) {
    *this -= T(delegate);
  }

protected:
  void operator () (typename T::SenderType* sender, typename T::ArgumentType argument) {
    for(unsigned i = 0; i < _list.Length(); ++i) {
      _list[i](sender, argument);
    }
  }

  friend typename T::SenderType;
};

#endif