#pragma once

#include <iostream>

#ifndef DEBUG
    #define DEBUG_ASSERT(condition)
#else
    #define DEBUG_ASSERT(condition) if(condition) std::abort(); else {}
#endif

#define ALWAYS_ASSERT(condition) if(condition) std::abort(); else {}

#ifndef DEBUG
    #define DEBUG_LOG(condition, message)
#else
    #define DEBUG_ASSERTION_LOG(condition, message) if(condition) std::cerr << "Assertion log. Line:" << __LINE__ << ". Message:" << message << std::endl; else {}
#endif

#define ALWAYS_LOG(condition, message)  if(condition) std::cerr << "Assertion log. Line:" << __LINE__ << ". Message:" << message << std::endl; else {}