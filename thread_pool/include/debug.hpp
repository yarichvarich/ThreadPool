#pragma once

#ifndef DEBUG
    #define DEBUG_ASSERT(condition)
#else
    #define DEBUG_ASSERT(condition) if(condition) std::abort(); else {}
#endif