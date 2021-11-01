/* date = September 23rd 2021 9:41 pm */

#ifndef ACC_ARDUINO_DEBUG_H
#define ACC_ARDUINO_DEBUG_H


#define ASSERT(expression) if(!(expression)){ *(u32 *)0x00 = 0; }
//#if SLOW
//#else
//#define ASSERT(expression)
//#endif
//


#endif //ACC_ARDUINO_DEBUG_H
