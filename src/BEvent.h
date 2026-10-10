#ifndef __BEVENT_H__
#define __BEVENT_H__

#include <string.h>

// Declares a public event `on<Name>` and a protected `raiseOn<Name>(arg)`.
// Each event stores up to `Capacity` handlers in place (no heap).
#define BEVENT_N(EventName, SenderType, ArgumentType, Capacity) \
public: \
    typedef BEventDelegate<SenderType, ArgumentType> EventName ## Event; \
    BEventSource<EventName ## Event, Capacity> on ## EventName; \
protected: \
    void raiseOn ## EventName(ArgumentType arg) { on ## EventName(this, arg); }

#define BEVENT(EventName, SenderType, ArgumentType) BEVENT_N(EventName, SenderType, ArgumentType, 1)

// A handler: a plain function, or an object plus one of its methods.
template<typename TSender, typename TArgument>
class BEventDelegate {
  struct Unknown;
  typedef void (Unknown::*AnyMethod)(TSender*, TArgument);
  typedef void (*Function)(TSender*, TArgument);
  typedef void (*Invoker)(const BEventDelegate&, TSender*, TArgument);

  void* _instance;
  union {
    Function function;
    char method[sizeof(AnyMethod)];
  } _target;
  Invoker _invoke;

  template<typename TClass>
  static void invokeMethod(const BEventDelegate& d, TSender* sender, TArgument argument) {
    void (TClass::*method)(TSender*, TArgument);
    memcpy(&method, d._target.method, sizeof(method));
    (static_cast<TClass*>(d._instance)->*method)(sender, argument);
  }

  static void invokeFunction(const BEventDelegate& d, TSender* sender, TArgument argument) {
    d._target.function(sender, argument);
  }

public:
  typedef TSender SenderType;
  typedef TArgument ArgumentType;

  template<typename TClass>
  BEventDelegate(TClass* instance, void (TClass::*method)(TSender* sender, TArgument argument))
    : _instance(instance), _invoke(instance && method ? &invokeMethod<TClass> : 0) {
    static_assert(sizeof(method) <= sizeof(AnyMethod), "method pointer too large");
    memset(&_target, 0, sizeof(_target));
    memcpy(_target.method, &method, sizeof(method));
  }

  BEventDelegate(void (*function)(TSender* sender, TArgument argument))
    : _instance(0), _invoke(function ? &invokeFunction : 0) {
    memset(&_target, 0, sizeof(_target));
    _target.function = function;
  }

  BEventDelegate() : _instance(0), _invoke(0) {
    memset(&_target, 0, sizeof(_target));
  }

  void operator()(TSender* sender, TArgument argument) const {
    if (_invoke) _invoke(*this, sender, argument);
  }

  explicit operator bool() const {
    return _invoke != 0;
  }

  bool operator == (const BEventDelegate& other) const {
    return _invoke == other._invoke && _instance == other._instance
        && memcmp(&_target, &other._target, sizeof(_target)) == 0;
  }
};

template<typename T, unsigned Capacity = 1>
class BEventSource {
  T _handlers[Capacity];

public:
  // Returns false if the handler is empty or all slots are taken.
  // Adding a handler that is already there does nothing and returns true.
  bool operator += (T delegate) {
    if (!delegate) return false;
    T* slot = 0;
    for (unsigned i = 0; i < Capacity; ++i) {
      if (_handlers[i] == delegate) return true;
      if (!slot && !_handlers[i]) slot = &_handlers[i];
    }
    if (!slot) return false;
    *slot = delegate;
    return true;
  }

  bool operator += (void (*delegate)(typename T::SenderType*, typename T::ArgumentType)) {
    return *this += T(delegate);
  }

  void operator -= (T delegate) {
    for (unsigned i = 0; i < Capacity; ++i) {
      if (_handlers[i] == delegate) _handlers[i] = T();
    }
  }

  void operator -= (void (*delegate)(typename T::SenderType*, typename T::ArgumentType)) {
    *this -= T(delegate);
  }

protected:
  void operator () (typename T::SenderType* sender, typename T::ArgumentType argument) {
    for (unsigned i = 0; i < Capacity; ++i) {
      T handler = _handlers[i];  // a handler may remove itself while running
      handler(sender, argument);
    }
  }

  friend typename T::SenderType;
};

#endif
