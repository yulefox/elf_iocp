/**
 * @file   type.h
 * @date   2010-01-06
 * @author Fox (yulefox at gmail.com)
 * 对项目中使用的部分内建类型进行约定:
 * * 不使用Windows内建类型: 包括但不限于BOOL, BYTE, WORD, DWORD;
 * * 不建议使用 unsigned 类型;
 * * buffer 可直接使用 char.
 */

#ifndef ELF_DEF_TYPE_H
#define ELF_DEF_TYPE_H

#pragma once

#define FLOATEQ(f1, f2) (abs((f1)-(f2)) < 0.001f)

#define MAKE_UINT_64(a, b) \
    ((ulonglong)(((ulonglong)((ulong)(a))) << 32 | ((ulong)(b))))

#define LO_UINT_64(l) \
    ((ulong)(l))

#define HI_UINT_64(l) \
    ((ulong)(((ulonglong)(l) >> 32) & 0xFFFFFFFF))

#ifdef WIN32
typedef unsigned char      uchar;
typedef unsigned short     ushort;
typedef unsigned int       uint;
typedef unsigned long      ulong;
typedef unsigned __int64   u64;
#endif

/**
 * API return codes
 */
typedef enum {
    ELF_RC_OK   = 0,        /* everything ok    */
    ELF_RC_ARG  = 1,        /* invalid argument */
    ELF_RC_MEM  = 2,        /* out of memory    */
    ELF_RC_SYS  = 3,        /* system error     */
    ELF_RC_INT  = 4,        /* internal error   */
    ELF_RC_IMP  = 5         /* not implemented  */
} elf_rc;

typedef int(*hash_func)(const void *);
typedef int(*compare_func)(const void *, const void *);
typedef void *(*dup_func)(const void *);
typedef int(*release_func)(void *);
typedef int(*traverse_func)(void *, ...);
typedef int(*init_func)(void);
typedef int(*fini_func)(void);

#ifdef _WIN32
typedef unsigned int (__stdcall *thread_func)(void *);
#else
typedef int(*thread_func)(void *);
#endif /* _WIN32 */

#endif /* !ELF_DEF_TYPE_H */
