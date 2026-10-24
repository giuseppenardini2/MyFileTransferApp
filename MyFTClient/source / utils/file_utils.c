/**
 * This source file contains implementations of the "file_utils.h" functions.
 *
 * @author peppo02
 */

#include <file_utils.h>


/**
 * This functions checks if a directory exists.
 *
 * @param path The directory path.
 * @return If the directory exists.
 */
bool check_directory(const char *path){
	if(path==NULL){
		fprintf(stderr, "The path is null.\n");
		return false;
	}
	struct stat stat_buffer;
	if(stat(path, &stat_buffer)==0){ //If the path exists and it corresponds to a directory, return true.
		if(S_ISDIR(stat_buffer.st_mode)!=0)
			return true;
		else{
			fprintf(stderr, "'%s' is a file.\n", path);
			return false;
		}
	}
	char error_message[256];
	strerror_r(errno, error_message, sizeof(error_message));
	fprintf(stderr, "stat() error for path '%s': %s (%d).\n", path, error_message, errno);
	return false;
}

/**
 * This function checks if a directory exists.
 * If the directory does not exists, it will be created.
 *
 * @param path The directory path.
 * @return If the directory exists or if it has been created.
 */
bool check_create_directory(const char *path){
	if(path==NULL){
		fprintf(stderr, "The path is null.\n");
		return false;
	}
	struct stat stat_buffer;
	if(stat(path, &stat_buffer)==0){ //If the path exists and it corresponds to a directory, return true.
		if(S_ISDIR(stat_buffer.st_mode)!=0)
			return true;
		else{
			fprintf(stderr, "'%s' is a file.\n", path);
			return false;
		}
	}
	else if(errno==ENOENT){ //If the path does not exist, try to create the folder.
		if(mkdir(path, 0775)==0)
			return true;
		else{
			char error_message[256];
			strerror_r(errno, error_message, sizeof(error_message));
			fprintf(stderr, "mkdir() error for path '%s': %s (%d).\n", path, error_message, errno);
			return false;
		}
	}
	char error_message[256];
	strerror_r(errno, error_message, sizeof(error_message));
	fprintf(stderr, "stat() error for path '%s': %s (%d).\n", path, error_message, errno);
	return false;
}

/**
 * This function checks if a file exists.
 *
 * @param path The file path.
 * @return If the file exists.
 */
bool check_file(const char *path){
	struct stat stat_buffer;
	if(stat(path, &stat_buffer)==0)
		return true;
	return false;
}

/**
 * This functions checks if a file exists.
 * If the file does not exist, it will be created.
 *
 * @param path The file path.
 * @return If the file exists or if it has been created.
 */
bool check_create_file(const char *path){
	if(check_file(path)==true)
		return true;
	FILE *file=fopen(path, "wb");
	if(file==NULL){
		char error_message[256];
		strerror_r(errno, error_message, sizeof(error_message));
		fprintf(stderr, "fopen() error for path '%s': %s (%d).\n", path, error_message, errno);
		return false;
	}
	fclose(file);
	return true;
}

/**
 * This files calulates the size of a file.
 *
 * @param path The file path.
 * @return The file size.
 */
off_t get_file_size(const char *path){
	struct stat stat_buffer;
	if(stat(path, &stat_buffer)!=0){
		char error_message[256];
		strerror_r(errno, error_message, sizeof(error_message));
		fprintf(stderr, "stat() error for path '%s': %s (%d).\n", path, error_message, errno);
	}
	return stat_buffer.st_size;
}

/**
 * This function checks if the space is available for a file.
 *
 * @param path The file path.
 * @param file_size The file size.
 * @return If the space is available.
 */
bool check_available_space(const char *path, off_t file_size){
	struct statvfs stat_buffer;
	if(statvfs(path, &stat_buffer)!=0){
		char error_message[256];
		strerror_r(errno, error_message, sizeof(error_message));
		fprintf(stderr, "statvfs() error for path '%s': %s (%d).\n", path, error_message, errno);
		return false;
	}
	off_t available_space=stat_buffer.f_bsize*stat_buffer.f_bavail;
	if(file_size<=available_space)
		return true;
	return false;
}
