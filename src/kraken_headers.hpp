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

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#define sysexits_h_
#define EX_USAGE 64
#define EX_DATAERR 65
#define EX_NOINPUT 66
#define EX_SOFTWARE 70
#define EX_OSERR 71

// Memory mapping constants for Windows
#define PROT_READ 0x1
#define PROT_WRITE 0x2
#define MAP_SHARED 0x01
#define MAP_PRIVATE 0x02
#define MAP_ANONYMOUS 0x20
#define MAP_FAILED ((void *)-1)
#define MS_SYNC 0x04

// Memory mapping functions for Windows
inline void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
    HANDLE handle = INVALID_HANDLE_VALUE;
    DWORD protect = PAGE_READONLY;
    DWORD desiredAccess = FILE_MAP_READ;

    if (prot & PROT_WRITE) {
        protect = PAGE_READWRITE;
        desiredAccess = FILE_MAP_WRITE;
    }

    if (flags & MAP_ANONYMOUS) {
        handle = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, protect,
                                 (DWORD)((length >> 32) & 0xFFFFFFFF),
                                 (DWORD)(length & 0xFFFFFFFF), NULL);
    } else {
        HANDLE file = (HANDLE)_get_osfhandle(fd);
        handle = CreateFileMapping(file, NULL, protect, 0, 0, NULL);
    }

    if (handle == NULL) return MAP_FAILED;

    void *map = MapViewOfFile(handle, desiredAccess,
                             (DWORD)((offset >> 32) & 0xFFFFFFFF),
                             (DWORD)(offset & 0xFFFFFFFF), length);
    CloseHandle(handle);

    if (map == NULL) return MAP_FAILED;
    return map;
}

inline int munmap(void *addr, size_t length) {
    return UnmapViewOfFile(addr) ? 0 : -1;
}

inline int madvise(void *addr, size_t length, int advice) {
    return 0; // Windows doesn't have direct equivalent
}

inline int msync(void *addr, size_t length, int flags) {
    return FlushViewOfFile(addr, length) ? 0 : -1;
}

inline int getpagesize() {
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    return si.dwPageSize;
}

// Error functions
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
// Unix headers and functions
#include <err.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sysexits.h>
#include <unistd.h>
#endif

#endif // KRAKEN_HEADERS_HPP