/**
 * @file   lastz.h
 * @author Jiangwen Su <lastz.cn@gmail.com>
 * @date   Mon Feb  2 17:31:14 2009
 * 
 * @brief  
 * 
 * 
 */

#ifndef __LASTZ_H__
#define __LASTZ_H__

#ifdef _WIN32
  #ifndef LASTZ_API
    #ifdef LASTZ_EXPORTS
      #define LASTZ_API __declspec(dllexport)
    #else 
      #define LASTZ_API __declspec(dllimport)
    #endif
  #endif
#else
  #define LASTZ_API
#endif

#pragma warning(disable:4786)
#pragma warning(disable:4251)
#pragma warning(disable:4267) // conversion from '' to '', possible losss of data
#pragma warning(disable:4150)
#pragma warning(disable:4710)
#pragma warning(disable:4668)
#pragma warning(disable:4819)
#pragma warning(disable:4820)
#pragma warning(disable:4619)
#pragma warning(disable:4996) // was declared deprecated
#pragma warning(disable:4127) // conditional expression is constant
#pragma warning(disable:4244) // conversion from xx to yy, possible loss of data
#pragma warning(disable:4201) // nonstandard extension used: nameless struct/union
#pragma warning(disable:4245) // conversion from xxx to yyy, signed/unsigned mismatch
#pragma warning(disable:4100) // unreferenced formal parameter

//#endif

#include <vector>
#include <list>
#include <map>
#include <set>
#include <string>
#include <assert.h>
#include <sstream>
#include <algorithm>

#endif // __LASTZ_H__
