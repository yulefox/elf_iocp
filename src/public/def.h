/**
 * @file   def.h
 * @date   2011-08-31
 * @author Fox (yulefox at gmail.com)
 * public 定义常用常数, 宏
 */

#ifndef ELF_DEF_H
#define ELF_DEF_H

#pragma once

#define MODULE_IMPORT_SWITCH    do { \
        static bool once = true; \
        if (!once) return -1; \
        once = false; \
    } while (0)

#ifndef SAFE_DELETE
#define SAFE_DELETE(x) do{ if (x) { delete (x); (x)=NULL;} } while (0)
#endif

#ifndef SAFE_DECREF
#define SAFE_DECREF(x) do{ if (x) { (x)->DecRef(); (x)=NULL; } } while (0)
#endif

#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(x) do{ delete[] (x); (x)=NULL; } while (0)
#endif

#ifndef SAFE_FREE
#define SAFE_FREE(x) do{ free(x); (x)=NULL; } while (0)
#endif

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(x)	do{ if (x) { (x)->Release(); (x)=NULL; } } while (0)
#endif

#ifndef SAFE_DEL_RELEASE
#define SAFE_DEL_RELEASE(x)	do{ (x)->Release(); delete (x); (x)=NULL; } while (0)
#endif

/* @{ */
/* notice: MSVC 6.0 does not supply ``inline`` */
#define ELF_INL             inline

#if defined(_WIN32) && defined(ELF_DYNAMIC)
#   ifdef ELF_EXPORTS
#       define ELF_API      __declspec(dllexport)
#       define ELF_DATA     __declspec(dllexport)
#   else
#       define  ELF_API     __declspec(dllimport)
#       define  ELF_DATA    __declspec(dllimport)
#   endif
#else
#   define ELF_API          extern
#   define ELF_DATA         extern
#endif
/* @} */

/* @{ */
#ifdef __cplusplus
#   define BEGIN_C_DECLS     extern "C" {
#   define END_C_DECLS       }
#else
typedef enum { false, true } bool;
#  define BEGIN_C_DECLS
#  define END_C_DECLS
#endif /* __cplusplus */

/* @} */

#endif /* !ELF_DEF_H */
