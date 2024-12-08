/*
 * Original file Copyright 2013-2015, Derrick Wood <dwood@cs.jhu.edu>
 * Portions (c) 2017-2018, Florian Breitwieser <fbreitwieser@jhu.edu> as part of KrakenUniq
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

#include "kraken_headers.hpp"
#include "quickfile.hpp"

using std::string;

namespace kraken {

QuickFile::QuickFile() {
  valid = false;
  fptr = NULL;
  filesize = 0;
  fd = -1;
}

QuickFile::QuickFile(string filename_str, string mode, size_t size) {
  open_file(filename_str, mode, size);
}

void QuickFile::open_file(string filename_str, string mode, size_t size) {
  const char *filename = filename_str.c_str();
  #ifdef _WIN32
  int o_flags = mode == "w"
                  ? _O_RDWR | _O_CREAT | _O_TRUNC | _O_BINARY
                  : mode == "r" ? _O_RDONLY | _O_BINARY : _O_RDWR | _O_BINARY;
  #else
  int o_flags = mode == "w"
                  ? O_RDWR | O_CREAT | O_TRUNC
                  : mode == "r" ? O_RDONLY : O_RDWR;
  #endif
  int m_flags = mode == "r" ? MAP_PRIVATE : MAP_SHARED;

  int m_prot = PROT_READ;
  if (mode != "r")
    m_prot |= PROT_WRITE;

  #ifdef _WIN32
  fd = _open(filename, o_flags, _S_IREAD | _S_IWRITE);
  #else
  fd = open(filename, o_flags, 0666);
  #endif
  // Second try for R/W if failure was due to non-existence
  if (fd < 0 && mode == "rw" && errno == ENOENT) {
    o_flags |= O_CREAT;
    #ifdef _WIN32
    fd = _open(filename, o_flags, _S_IREAD | _S_IWRITE);
    #else
    fd = open(filename, o_flags, 0666);
    #endif
  }
  if (fd < 0)
    err(EX_OSERR, "unable to open %s", filename);

  if (o_flags & (
    #ifdef _WIN32
    _O_CREAT
    #else
    O_CREAT
    #endif
    )) {
    #ifdef _WIN32
    if (_lseek(fd, size - 1, SEEK_SET) < 0)
    #else
    if (lseek(fd, size - 1, SEEK_SET) < 0)
    #endif
      err(EX_OSERR, "unable to lseek (%s)", filename);
    if (write(fd, "", 1) < 0)
      err(EX_OSERR, "write error (%s)", filename);
    filesize = size;
  }
  else {
    struct stat sb;
    #ifdef _WIN32
    if (_fstat(fd, &sb) < 0)
    #else
    if (fstat(fd, &sb) < 0)
    #endif
      err(EX_OSERR, "unable to fstat %s", filename);
    filesize = sb.st_size;
  }

  fptr = (char *)mmap(0, filesize, m_prot, m_flags, fd, 0);
  #ifndef _WIN32
  madvise(fptr, filesize, MADV_WILLNEED);
  #endif
  if (fptr == MAP_FAILED)
    err(EX_OSERR, "unable to mmap %s", filename);
  valid = true;
}

void QuickFile::load_file() {
  #ifndef _WIN32
  if (mlock(fptr,filesize) != 0)
  #endif
  {
    int thread_ct = 1;
    int thread = 0;
    #ifdef _OPENMP
    int old_thread_ct = omp_get_max_threads();
    if (old_thread_ct > 4)
      omp_set_num_threads(4);
    thread_ct = omp_get_max_threads();
    #endif

    size_t page_size = getpagesize();
    std::vector<std::vector<char>> buf(thread_ct, std::vector<char>(page_size));

    #ifdef _OPENMP
    #pragma omp parallel
    #endif
    {
      #ifdef _OPENMP
      thread = omp_get_thread_num();
      #endif

      #ifdef _OPENMP
      #pragma omp for schedule(dynamic)
      #endif
      for (int64_t pos = 0; pos < (int64_t)filesize; pos += page_size) {
        size_t this_page_size = filesize - pos;
        if (this_page_size > page_size)
          this_page_size = page_size;
        memcpy(buf[thread].data(), fptr + pos, this_page_size);
      }
    }

    #ifdef _OPENMP
    omp_set_num_threads(old_thread_ct);
    #endif
  }
}

char * QuickFile::ptr() {
  return valid ? fptr : NULL;
}

size_t QuickFile::size() {
  return valid ? filesize : 0;
}

QuickFile::~QuickFile() {
  close_file();
}

void QuickFile::sync_file() {
  msync(fptr, filesize, MS_SYNC);
}

void QuickFile::close_file() {
  if (! valid)
    return;
  sync_file();
  munmap(fptr, filesize);
  #ifdef _WIN32
  _close(fd);
  #else
  close(fd);
  #endif
  valid = false;
}

// from http://programanddesign.com/cpp/human-readable-file-size-in-c/
char* readable_fs(double size/*in bytes*/, char *buf) {
    int i = 0;
    const char* units[] = {"B", "kB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"};
    while (size > 1024) {
        size /= 1024;
        i++;
    }
    sprintf(buf, "%.*f %s", i, size, units[i]);
    return buf;
}

std::vector<char> slurp_file(std::string filename, size_t lSize) {
  FILE * pFile;
  size_t result;

  pFile = fopen(filename.c_str(), "rb");
  if (pFile==NULL) {fputs ("File error",stderr); exit (1);}

  if (lSize > 0) {
    // obtain file size:
    fseek(pFile, 0, SEEK_END);
    lSize = ftell(pFile);
    rewind(pFile);
  }

  char buf[50];
  readable_fs(lSize, buf);
  std::cerr << "Getting " << filename << " into memory (" << buf << ") ...";

  // copy the file into the vector:
  std::vector<char> buffer(lSize);
  result = fread(buffer.data(), 1, lSize, pFile);
  if (result != lSize) {fputs ("Reading error",stderr); exit (3);}
  fclose(pFile);

  std::cerr << " Done" << std::endl;
  return(buffer);
}

} // namespace