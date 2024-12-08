#ifndef KRAKEN_HEADERS_HPP
#define KRAKEN_HEADERS_HPP

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 1
#endif

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <map>
#ifdef _OPENMP
#include <omp.h>
#endif
#include <set>
#include <sstream>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <vector>
#include "memory_utils.hpp"

#ifdef _WIN32
// Windows-specific headers and definitions
#include <windows.h>
#include <io.h>
#define sysexits_h_
#define EX_USAGE 64
#define EX_DATAERR 65
#define EX_NOINPUT 66
#define EX_SOFTWARE 70
#define EX_OSERR 71

// Error handling functions
inline void err(int eval, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, ": %s\n", strerror(errno));
    exit(eval);
}

inline void errx(int eval, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
    exit(eval);
}

inline void warn(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, ": %s\n", strerror(errno));
}

inline void warnx(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
}

#else
// Unix-specific headers
#include <err.h>
#include <sys/time.h>
#include <sysexits.h>
#include <unistd.h>
#endif

#endif // KRAKEN_HEADERS_HPP