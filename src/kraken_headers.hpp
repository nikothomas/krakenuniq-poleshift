/*
 * Copyright 2013-2015, Derrick Wood <dwood@cs.jhu.edu>
 *
 * This file is part of the Kraken taxonomic sequence classification system.
 *
 * Kraken is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Kraken is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Kraken.  If not, see <http://www.gnu.org/licenses/>.
 */

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
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <vector>

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

// Replacements for Unix-specific functions
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

#define PROT_READ 0x1
#define PROT_WRITE 0x2
#define MAP_SHARED 0x01
#define MAP_FAILED ((void *) -1)

inline void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
    HANDLE handle = CreateFileMapping((HANDLE)_get_osfhandle(fd), NULL,
                                    PAGE_READWRITE, 0, 0, NULL);
    if (handle == NULL) return MAP_FAILED;

    void *map = MapViewOfFile(handle, FILE_MAP_ALL_ACCESS, 0, 0, length);
    CloseHandle(handle);

    if (map == NULL) return MAP_FAILED;
    return map;
}

inline int munmap(void *addr, size_t length) {
    return UnmapViewOfFile(addr) ? 0 : -1;
}

#else
// Unix-specific headers
#include <err.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sysexits.h>
#include <unistd.h>
#endif

#endif