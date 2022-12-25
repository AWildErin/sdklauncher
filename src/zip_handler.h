
#ifndef ZIPPER_H_
#define ZIPPER_H_


#include <unzip.h>
#include <zip.h>

typedef enum {
	ZIPPER_RESULT_ERROR = 0,
	ZIPPER_RESULT_SUCCESS,
	ZIPPER_RESULT_SUCCESS_EOF
} zipper_result_t;

typedef void (*zipper_read_cb_t)(const unsigned char *buf, size_t size, void *thunk);

bool zipper_add_file(zipFile zfile, const char *filename);
bool zipper_add_buf(zipFile zfile, const char *zfilename, const unsigned char *buf, size_t buflen);
bool zipper_add_dir(zipFile zfile, const char *dirname);

zipper_result_t zipper_read(unzFile zfile, zipper_read_cb_t cb, void *thunk);
zipper_result_t zipper_read_buf(unzFile zfile, unsigned char **buf, size_t *buflen);

bool zipper_skip_file(unzFile zfile);
char *zipper_filename(unzFile zfile, bool *isutf8);
bool zipper_isdir(unzFile zfile);
uint64_t zipper_filesize(unzFile zfile);

#endif /* ZIPPER_H__ */