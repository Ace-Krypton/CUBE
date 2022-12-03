#pragma once

#if defined(unix) || defined(__unix) || defined(__unix__)
#define UNIX
#endif

#if defined(__APPLE__)
#define APPLE
#endif

#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
#define WINDOWS
#endif

#if defined(__x86_64__) || defined(__x86_64) || defined(__amd64__) || defined(_M_X64)
#define X86_64
#elif defined(__i386__) || defined(_M_IX86)
#define X86_32
#endif

#if defined(X86_64) || defined(X86_64)
#define X86
#endif
