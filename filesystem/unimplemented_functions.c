#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>


/* ----------------------------------------------------------- *
 *  NOT IMPLEMENTED FUSE FUNCTIONS -- FOR DEBBUGGING PURPOSES  *
 * ----------------------------------------------------------- */

static int
fisopfs_fgetattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi)
{
	fprintf(stderr, "[debug] fisopfs_fgetattr\n");
	return 0;
}

static int
fisopfs_access(const char *path, int mask)
{
	fprintf(stderr, "[debug] fisopfs_access\n");
	return 0;
}

static int
fisopfs_readlink(const char *path, char *buf, size_t size)
{
	fprintf(stderr, "[debug] fisopfs_readlink\n");
	return 0;
}

static int
fisopfs_mknod(const char *path, mode_t mode, dev_t rdev)
{
	fprintf(stderr, "[debug] fisopfs_mknod\n");
	return 0;
}

static int
fisopfs_symlink(const char *to, const char *from)
{
	fprintf(stderr, "[debug] fisopfs_symlink\n");
	return 0;
}


static int
fisopfs_rename(const char *from, const char *to)
{
	fprintf(stderr, "[debug] fisopfs_rename\n");
	return 0;
}

static int
fisopfs_link(const char *from, const char *to)
{
	fprintf(stderr, "[debug] fisopfs_link\n");
	return 0;
}

static int
fisopfs_chmod(const char *path, mode_t mode)
{
	fprintf(stderr, "[debug] fisopfs_chmod\n");
	return 0;
}

static int
fisopfs_chown(const char *path, uid_t uid, gid_t gid)
{
	fprintf(stderr, "[debug] fisopfs_chown\n");
	return 0;
}


static int
fisopfs_ftruncate(const char *path, off_t size, struct fuse_file_info *fi)
{
	fprintf(stderr, "[debug] fisopfs_ftruncate\n");
	return 0;
}

static int
fisopfs_open(const char *path, struct fuse_file_info *fi)
{
	fprintf(stderr, "[debug] fisopfs_open\n");
	return 0;
}


static int
fisopfs_statfs(const char *path, struct statvfs *stbuf)
{
	fprintf(stderr, "[debug] fisopfs_statfs\n");
	return 0;
}

static int
fisopfs_release(const char *path, struct fuse_file_info *fi)
{
	fprintf(stderr, "[debug] fisopfs_release\n");
	return 0;
}

static int
fisopfs_opendir(const char *path, struct fuse_file_info *fi)
{
	fprintf(stderr, "[debug] fisopfs_opendir path: %s\n", path);
	return 0;
}

static int
fisopfs_releasedir(const char *path, struct fuse_file_info *fi)
{
	fprintf(stderr, "[debug] fisopfs_releasedir\n");
	return 0;
}

static int
fisopfs_fsync(const char *path, int isdatasync, struct fuse_file_info *fi)
{
	fprintf(stderr, "[debug] fisopfs_fsync\n");
	return 0;
}

static int
fisopfs_fsyncdir(const char *path, int isdatasync, struct fuse_file_info *fi)
{
	fprintf(stderr, "[debug] fisopfs_fsyncdir\n");
	return 0;
}

static int
fisopfs_lock(const char *path, struct fuse_file_info *fi, int cmd, struct flock *locks)
{
	fprintf(stderr, "[debug] fisopfs_lock\n");
	return 0;
}

static int
fisopfs_bmap(const char *path, size_t blocksize, uint64_t *blockno)
{
	fprintf(stderr, "[debug] fisopfs_bmap\n");
	return 0;
}

static int
fisopfs_ioctl(const char *path,
              int cmd,
              void *arg,
              struct fuse_file_info *fi,
              unsigned int flags,
              void *data)
{
	fprintf(stderr, "[debug] fisopfs_ioctl\n");
	return 0;
}

static int
fisopfs_poll(const char *path,
             struct fuse_file_info *fi,
             struct fuse_pollhandle *ph,
             unsigned *reventsp)
{
	fprintf(stderr, "[debug] fisopfs_poll\n");
	return 0;
}