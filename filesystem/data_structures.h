#include <sys/stat.h>
#include <time.h>

#define BLOCK_SIZE 4096  // 4kb
#define MAX_NAME_LARGE 100
#define AMOUNT_OF_INODES 250
#define MAX_BLOCKS_PER_INODE 16
#define AMOUNT_OF_BLOCKS 4000  // 250 * 16
#define DEFAULT_FILE "group26fs.fisopfs"
#define DIR_T 0
#define REG_T 1
#define FREE 0
#define USED 1

struct block {
	char data[BLOCK_SIZE];
};

struct inode {
	int inum;
	mode_t mode;
	int type;
	uid_t user_id;
	off_t file_size;
	time_t creation_time;
	time_t last_acceced_time;
	time_t modification_time;
	time_t deletion_time;
	nlink_t links_count;
	blkcnt_t number_of_blocks;
	int blocks_index[MAX_BLOCKS_PER_INODE];
	struct block *file_data[MAX_BLOCKS_PER_INODE];
	char name[MAX_NAME_LARGE];
};

struct superblock {
	int inodes;
	int blocks;
	int *inode_bitmap;
	int *data_bitmap;
};
