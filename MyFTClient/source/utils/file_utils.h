/**
 * This header file declares useful functions for various contexts.
 *
 * @author peppo02
 */

#ifndef FILE_UTILS_H_
#define FILE_UTILS_H_


#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/statvfs.h>


bool check_directory(const char *path);

bool check_create_directory(const char *path);

bool check_file(const char *path);

bool check_create_file(const char *path);

off_t get_file_size(const char *path);

bool check_available_space(const char *path, off_t file_size);


#endif /* FILE_UTILS_H_ */
