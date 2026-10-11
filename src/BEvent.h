#ifndef __BEVENT_H__
#define __BEVENT_H__

#include <Eventfun.h>

// Declares a public event `on<Name>` (an Eventfun EventSource), its handler
// type `<Name>Event`, and a protected `raiseOn<Name>(arg)`. Each event stores
// up to `Capacity` handlers in place (no heap).
#define BEVENT_N(EventName, SenderType, ArgumentType, Capacity) \
public: \
    typedef EventDelegate<SenderType, ArgumentType> EventName ## Event; \
    EventSource<SenderType, ArgumentType, Capacity> on ## EventName; \
protected: \
    void raiseOn ## EventName(ArgumentType arg) { on ## EventName(this, arg); }

#define BEVENT(EventName, SenderType, ArgumentType) BEVENT_N(EventName, SenderType, ArgumentType, 1)

#endif
