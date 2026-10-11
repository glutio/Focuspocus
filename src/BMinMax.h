#ifndef __BMINMAX_H__
#define __BMINMAX_H__

// For the library's .cpp files only: Arduino.h defines these, other
// platforms may not. Kept out of public headers so sketches are unaffected.
#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif

#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif

#endif