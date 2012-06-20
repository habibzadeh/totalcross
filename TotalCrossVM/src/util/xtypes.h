/*********************************************************************************
 *  TotalCross Software Development Kit                                          *
 *  Copyright (C) 2000-2012 SuperWaba Ltda.                                      *
 *  All Rights Reserved                                                          *
 *                                                                               *
 *  This library and virtual machine is distributed in the hope that it will     *
 *  be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of    *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                         *
 *                                                                               *
 *********************************************************************************/



#ifndef XTYPES_H
#define XTYPES_H

#include <stdlib.h>
#include <string.h>
#if defined (WIN32)
 #include <tchar.h>
#endif

#define TCVM_CRID 'TCvm'

#ifdef __cplusplus
extern "C" {
#endif

// Crossplatform Types

#define SWAP16_FORCED(n) (((((unsigned int) n) << 8) & 0xFF00) | \
      ((((unsigned int) n) >> 8) & 0x00FF))

#define SWAP32_FORCED(n) (((((unsigned long) n) << 24) & 0xFF000000) |   \
      ((((unsigned long) n) <<  8) & 0x00FF0000) |   \
      ((((unsigned long) n) >>  8) & 0x0000FF00) |   \
      ((((unsigned long) n) >> 24) & 0x000000FF))

#ifndef UNUSED
#define UNUSED(x) x=x;
#endif

#if !defined(linux) && !defined(__arm__) && (defined(WINCE) || defined(WIN32) || defined(__SYMBIAN32__)/* || (_MSC_VER <= 1200) - gcc4win will fail with this*/)  // 1200 = VC6 and earlier
 #define I64_CONST(x) x##L
#else
 #define I64_CONST(x) x##LL
#endif

#define PTRSIZE sizeof(void*)

#ifdef darwin
 #define inline_
#else
 #define inline_ inline
#endif

#if defined THEOS || !defined darwin
#define __unsafe_unretained
#endif

#if defined THEOS
#define __bridge
#endif

 /////////////////////////////////////////////////////////////////////////
// Basic types
// explicit signed types (some compilers consider "char" as unsigned by default for instance).
typedef signed char int8;
typedef unsigned char uint8;
typedef signed int int32;
typedef unsigned int uint32;
typedef signed short int16;
typedef unsigned short uint16;
#if defined(WINCE) || defined(WIN32) || (defined __SYMBIAN32__  && defined __WINS__)
 typedef __int64 int64;
 typedef unsigned __int64 uint64;
#else
 typedef signed long long int64;
 typedef unsigned long long uint64;
#endif
typedef void* VoidP;
typedef char* CharP;

#if defined (PALMOS) || defined (WINCE)
 typedef int intptr_t;
#endif

#if defined(linux) || defined(__SYMBIAN32__)
typedef unsigned char byte;
#endif

#if defined(__SYMBIAN32__)
 #include <e32def.h>
#else
 typedef int16 TInt16;
 typedef int32 TInt32;
 typedef int64 TInt64;
#endif
typedef uint16 TUInt16;
typedef uint32 TUInt32;
typedef uint8 TUInt8;
typedef double TDouble;

typedef uint16 JChar; // Java char
typedef JChar* JCharP;

#if defined(WINCE) || defined(WIN32)
 #define inline __inline
 #if defined(UNICODE)
  typedef uint16 TCHAR;
 #endif
#else
    #define TEXT(x) x
    typedef char TCHAR;
#endif
typedef TCHAR* TCHARP;

#if !defined(__cplusplus) && !defined(__OBJC__) && !defined(_STDBOOL_H)
#define _STDBOOL_H // prevent stdbool.h include on darwin9
#if defined (darwin) && !defined (THEOS)
#undef bool
#define bool int
#else
typedef int bool;
#endif
#ifndef true
#define true 1   // please do NOT change these to uppercase
#define false 0
#endif
#endif

#if !defined(PALMOS) && !defined(__HSDATATYPES__) && !(defined(darwin) && defined(__OBJC__)) // Palm OS defines these as non-ptr types, so we can't define them as ptr ones.
typedef TInt32 Int32;
typedef TUInt32 UInt32;
typedef TUInt16 UInt16;
typedef TInt16 Int16;
typedef TUInt8 UInt8;
#endif
typedef TDouble Double;
#if !defined(__SYMBIAN32__)
typedef TInt64 Int64;
#endif

#ifndef INT32_MAX
 #define INT32_MAX (2147483647)
#endif

//Error handling
#if !defined (PALMOS)
 typedef int32 Err;
#endif

#if defined (WINCE) || defined (WIN32)
#elif defined (PALMOS)
 #define NO_ERROR errNone
#else
 #define NO_ERROR 0
#endif

// arrays
typedef JCharP* JCharPArray;
typedef int32* Int32Array;
typedef CharP* CharPArray;
typedef TCHARP* TCHARPArray;
typedef double* DoubleArray;
typedef int64* Int64Array;
typedef uint32* UInt32Array;
typedef int16*  Int16Array;
typedef uint16*  UInt16Array;
typedef uint8* UInt8Array;
typedef uint16** UInt16Matrix;
typedef VoidP* VoidPArray;

// value 64 (double union long)
typedef int64 TValue64;      // guich@tc111_7: must use int64, because double assignments validates the number.
typedef int64* Value64;
typedef int64* Value64Array;

#define REGD(x) ((double*)(x))
#define REGL(x) x

/////////////////////////////////////////////////////////////////////////
// Cross-platform standard functions from stdlib.h
// no uppercase, please!

#define xstrncmp(str1, str2, len) strncmp(str1, str2, len)
#define xstrcmp(str1, str2) strcmp(str1, str2)
CharP xstrncpy(CharP dest, CharP src, int32 len); // this one makes sure that the string is terminated with \0
#define xstrcpy(dest, src) strcpy(dest, src)
#define xstrlen(str) strlen(str)
#define xstrcat(dest, src) strcat(dest, src)
CharP xstrrchr(CharP str, int32 what);
#define xstrchr(str, what) strchr(str, what)
#define xstrstr(str, what) strstr(str, what)
#define xstrprintf sprintf
#define xstrvprintf vsprintf
// faster routines to move 8, 4, 2 and PTRSIZE pointers
#define xmove8(dest,src)                         \
   do                                            \
   {                                             \
      ((uint8*)(dest))[0] = ((uint8*)(src))[0];  \
      ((uint8*)(dest))[1] = ((uint8*)(src))[1];  \
      ((uint8*)(dest))[2] = ((uint8*)(src))[2];  \
      ((uint8*)(dest))[3] = ((uint8*)(src))[3];  \
      ((uint8*)(dest))[4] = ((uint8*)(src))[4];  \
      ((uint8*)(dest))[5] = ((uint8*)(src))[5];  \
      ((uint8*)(dest))[6] = ((uint8*)(src))[6];  \
      ((uint8*)(dest))[7] = ((uint8*)(src))[7];  \
   } while(0)
#define xmove4(dest,src)                         \
   do                                            \
   {                                             \
      ((uint8*)(dest))[0] = ((uint8*)(src))[0];  \
      ((uint8*)(dest))[1] = ((uint8*)(src))[1];  \
      ((uint8*)(dest))[2] = ((uint8*)(src))[2];  \
      ((uint8*)(dest))[3] = ((uint8*)(src))[3];  \
   } while(0)
#define xmove2(dest,src)                         \
   do                                            \
   {                                             \
      ((uint8*)(dest))[0] = ((uint8*)(src))[0];  \
      ((uint8*)(dest))[1] = ((uint8*)(src))[1];  \
   } while(0)
#define xmoveptr(dest,src)                       \
   do                                            \
   {                                             \
      if (PTRSIZE == 4)                          \
         xmove4(dest,src);                       \
      else                                       \
         xmove8(dest,src);                       \
   } while(0)

#define xmemmove(dest, src, len) memmove(dest, src, len)
#define xmemzero(mem, len) memset(mem, 0, len)
#define tzero(x) xmemzero(&(x), sizeof(x)) // used in structures (Txxx)
#define xmemset(mem, what, len) memset(mem, what, len)
#define xmemcmp(mem1, mem2, len) memcmp(mem1, mem2, len)

#if defined (PALMOS)
#define xstrncasecmp(str1, str2, len) StrNCaselessCompare(str1, str2, len)
#else
int32 xstrncasecmp(const char *a1, const char *a2, int32 size);
#endif

#define strEq !xstrcmp
#define strEqn !xstrncmp
#define strCaseEqn !xstrncasecmp

#ifdef WIN32
 #define tcsncmp _tcsncmp
 #define tcscmp _tcscmp
 #define tcsncpy _tcsncpy
 #define tcscpy _tcscpy
 #define tcslen _tcslen
 #define tcscat _tcscat
 #define tcsrchr _tcsrchr
 #define tcschr _tcschr
 #define tcsstr _tcsstr
 #define tcsspn _tcsspn
#else
 #define tcsncmp strncmp
 #define tcscmp strcmp
 #define tcsncpy xstrncpy
 #define tcscpy strcpy
 #define tcslen strlen
 #define tcscat strcat
 #define tcsrchr xstrrchr
 #define tcschr strchr
 #define tcsstr strstr
 #define tcsspn strspn
#endif


#define null 0

#define TARGET_SLASH '/'
#define MAX_PATHNAME 256

#if defined(__SYMBIAN32__)
 #include <sys/param.h>
#elif !defined(MIN)
 #define MIN(x,y) (((x) < (y)) ? (x) : (y))
 #define MAX(x,y) (((x) < (y)) ? (y) : (x))
#endif

#if defined(linux) || defined(darwin)
#define __attribute_packed__ __attribute__((__packed__))
#else
#define __attribute_packed__
#endif

#if !defined (WIN32) && !defined (WINCE)
#define INVALID_HANDLE_VALUE 0
#endif

#ifdef __cplusplus
};
#endif

#endif