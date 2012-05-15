#include "params.h"

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <fuse.h>
#include <libgen.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/xattr.h>
#include <pthread.h>

/* Stores the absolute path in "fpath", based on the file name given in "path" */
static void svfs_fullpath(char fpath[PATH_MAX], const char *path) {
	strcpy(fpath, SVFS_DATA->rootdir);
	strncat(fpath, path, PATH_MAX);
}

#define DEBUG

#ifdef DEBUG
static FILE *m_debug;
#endif

static void my_log(char *function, const char *path) {
#ifdef DEBUG
        time_t now;
        char *str;
        time(&now);
        str = ctime(&now);
        str[strlen(str) - 1] = '\0';
	fprintf(m_debug, "%s [%s] %s\n", str, function, path);
	fflush(m_debug);
#endif
}

/** Get file attributes. */
int svfs_getattr(const char *path, struct stat *statbuf) {
	char fpath[PATH_MAX];

	my_log("svfs_getattr", path);
	svfs_fullpath(fpath, path);

	if (lstat(fpath, statbuf))
		return -errno;
	return 0;
}

/** Creates a file node */
int svfs_mknod(const char *path, mode_t mode, dev_t dev) {
	int retstat;
	char fpath[PATH_MAX];

	my_log("svfs_mknod", path);
	svfs_fullpath(fpath, path);

	retstat = mknod(fpath, mode, dev);
	if (retstat)
		return -errno;
	return 0;
}

/** Create a directory */
int svfs_mkdir(const char *path, mode_t mode) {
	char fpath[PATH_MAX];

	my_log("svfs_mkdir", path);
	svfs_fullpath(fpath, path);

	if (mkdir(fpath, mode))
		return -errno;
	return 0;
}

/** Remove a file */
int svfs_unlink(const char *path) {
	char fpath[PATH_MAX];

	my_log("svfs_unlink", path);
	svfs_fullpath(fpath, path);

	if(unlink(fpath))
		return -errno;
	return 0;
}

/** Remove a directory */
int svfs_rmdir(const char *path) {
	char fpath[PATH_MAX];

	my_log("svfs_rmdir", path);
	svfs_fullpath(fpath, path);

	if (rmdir(fpath))
		return -errno;
	return 0;
}

/** Rename a file */
int svfs_rename(const char *path, const char *newpath) {
	char fpath[PATH_MAX];
	char fnewpath[PATH_MAX];

	my_log("svfs_rename", path);
	svfs_fullpath(fpath, path);
	svfs_fullpath(fnewpath, newpath);

	if (rename(fpath, fnewpath))
		return -errno;
	return 0;
}

/** Change the permission bits of a file */
int svfs_chmod(const char *path, mode_t mode) {
	char fpath[PATH_MAX];

	my_log("svfs_chmod", path);
	svfs_fullpath(fpath, path);

	if (chmod(fpath, mode))
		return -errno;
	return 0;
}

/** Change the owner and group of a file */
int svfs_chown(const char *path, uid_t uid, gid_t gid) {
	char fpath[PATH_MAX];

	my_log("svfs_chown", path);
	svfs_fullpath(fpath, path);

	if (chown(fpath, uid, gid))
		return -errno;
	return 0;
}

/** Change the size of a file */
int svfs_truncate(const char *path, off_t newsize) {
	char fpath[PATH_MAX];

	my_log("svfs_truncate", path);
	svfs_fullpath(fpath, path);

	if (truncate(fpath, newsize))
		return -errno;
	return 0;
}

/** Change the access and/or modification times of a file */
int svfs_utime(const char *path, struct utimbuf *ubuf) {
	char fpath[PATH_MAX];

	my_log("svfs_utime", path);
	svfs_fullpath(fpath, path);

	if (utime(fpath, ubuf))
		return -errno;
	return 0;
}

/** File open operation */
int svfs_open(const char *path, struct fuse_file_info *fi) {
	int fd;
	char fpath[PATH_MAX];

	my_log("svfs_open", path);
	svfs_fullpath(fpath, path);

	fd = open(fpath, fi->flags);
	fi->fh = fd;
	if (fd < 0)
		return -1;

	return 0;
}

/** Read data from an open file */
int svfs_read(const char *path, char *buf, size_t size, off_t offset,
		struct fuse_file_info *fi) {
	my_log("svfs_read", path);
	return (int) pread(fi->fh, buf, size, offset);
}

/** Write data to an open file */
int svfs_write(const char *path, const char *buf, size_t size, off_t offset,
		struct fuse_file_info *fi) {
	my_log("svfs_write", path);
	return pwrite(fi->fh, buf, size, offset);
}

int svfs_flush(const char *path, struct fuse_file_info *fi) {
	return 0;
}

/** Release an open file */
int svfs_release(const char *path, struct fuse_file_info *fi) {
	my_log("svfs_release", path);
	return close(fi->fh);
}

/** Open directory */
int svfs_opendir(const char *path, struct fuse_file_info *fi) {
	DIR *dp;
	char fpath[PATH_MAX];

	my_log("svfs_opendir", path);
	svfs_fullpath(fpath, path);

	dp = opendir(fpath);
	fi->fh = (intptr_t) dp;

	if (dp == NULL)
		return -errno;
	return 0;
}

/** Read directory */
int svfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		off_t offset, struct fuse_file_info *fi) {
	DIR *dp;
	struct dirent *de;

	my_log("svfs_readdir", path);
	dp = (DIR *) (uintptr_t) fi->fh;
    de = readdir(dp);

	if (de == NULL)
		return -errno;

	do {
		if (filler(buf, de->d_name, NULL, 0) != 0) {
			return -errno;
		}
	} while ((de = readdir(dp)) != NULL);

	return 0;
}

/** Release directory */
int svfs_releasedir(const char *path, struct fuse_file_info *fi) {
	my_log("svfs_releasedir", path);
	if (closedir((DIR *) (uintptr_t) fi->fh))
		return -errno;
	return 0;
}

/** Initialize filesystem */
void *svfs_init(struct fuse_conn_info *conn) {
	return SVFS_DATA;
}

/** Clean up filesystem */
void svfs_destroy(void *userdata) {
}

/** Create and open a file */
int svfs_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
	char fpath[PATH_MAX];
	int fd;

	my_log("svfs_create", path);
	svfs_fullpath(fpath, path);

	fd = creat(fpath, mode);
	fi->fh = fd;
	if (fd < 0)
		return -1;
	return 0;
}

struct fuse_operations svfs_oper = {
	.getattr = svfs_getattr,
	.mknod = svfs_mknod,
	.mkdir = svfs_mkdir,
	.unlink = svfs_unlink,
	.rmdir = svfs_rmdir,
	.rename = svfs_rename,
	.chmod = svfs_chmod,
	.chown = svfs_chown,
	.truncate = svfs_truncate,
	.utime = svfs_utime,
	.open = svfs_open,
	.read = svfs_read,
        .release = svfs_release,
	.write = svfs_write,
	.flush = svfs_flush,
	.opendir = svfs_opendir,
	.readdir = svfs_readdir,
	.releasedir = svfs_releasedir,
	.init = svfs_init,
	.destroy = svfs_destroy,
};

void svfs_usage() {
	fprintf(stderr, "Usage:  svfs rootDirectory mountPointDirectory\n");
	exit(1);
}

int main(int argc, char *argv[]) {
	int i;
	struct svfs_state *svfs_data;

	svfs_data = malloc(sizeof(struct svfs_state));
	if (svfs_data == NULL)
		exit(0);

#ifdef DEBUG
	m_debug = fopen("svfs.log", "a");
#endif

	for (i = 1; (i < argc) && (argv[i][0] == '-'); i++)
		if (argv[i][1] == 'o')
			i++;

	if ((argc - i) != 2)
		svfs_usage();

	svfs_data->rootdir = realpath(argv[i], NULL);

	argv[i] = argv[i + 1];
	argc--;

	// We should not return from here
	return fuse_main(argc, argv, &svfs_oper, svfs_data);
}
