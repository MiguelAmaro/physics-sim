/* date = November 14th 2021 0:32 pm */

#ifndef PHYSICS_SIM_PLATFORM_H
#define PHYSICS_SIM_PLATFORM_H

#include "physics_sim_types.h"

#define ASSERT(expression) if(!(expression)){ *(u32 *)0x00 = 0; }

#endif //PHYSICS_SIM_PLATFORM_H