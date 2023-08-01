#define FUSE_USE_VERSION 30

#include <fuse.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "data_structures.h"
#include "unimplemented_functions.c"


struct superblock superblock;
int inode_bitmap[AMOUNT_OF_INODES];
int data_bitmap[AMOUNT_OF_BLOCKS];
struct inode inodes[AMOUNT_OF_INODES];
struct block data_region_blocks[AMOUNT_OF_BLOCKS];


void create_default_file();
int unmarshall_default_file(FILE *file);
int get_number_of_blocks_in_inode(struct block *file_data[MAX_BLOCKS_PER_INODE]);
int get_inode_from_path(const char *path);
int get_free_block_index();
int get_free_inode_index();
int create_new_inode(const char *path, mode_t mode, int type);
int get_current_block_in_inode(int offset);
int is_in_dir(const char *dir_path, char *path);
int is_dentry_in_path(const char *path, char *dentry);
int get_last_dentry(const char *path);
static int fisopfs_unlink(const char *path);
void persist_fs_data(const char *file);
int amount_of_dentries_in_path(const char *path);
int minimo(int a, int b);
int get_free_block_for_inode(int inode_index, int offset);


/// Initialize the filesystem. This function can often be left unimplemented,
/// but it can be a handy way to perform one-time setup such as allocating
/// variable-sized data structures
// or initializing a new filesystem
static void *
fisopfs_init(struct fuse_conn_info *conn_info)
{
	printf("[implemented] fisop_init\n");
	FILE *file;
	// intento abrir archivo
	file = fopen(DEFAULT_FILE, "rb");
	if (!file) {
		// si no existe se crea y se escribe en disco
		create_default_file();
	} else {
		// si existe se lee de disco
		int res_of_unmarshalling = unmarshall_default_file(file);
		if (res_of_unmarshalling < 0) {
			errno = EIO;
		}
		fclose(file);
	}
	return NULL;
}

/// Clean up filesystem.
/// persiste todo el estado del filesystem en un archivo para que no se pierda la informacion
static void
fisopfs_destroy(void *private_data)
{
	printf("[implemented] fisop_destroy\n");
	persist_fs_data(DEFAULT_FILE);
}

/// Return file attributes.
/// For the given pathname, this should fill in the elements of the "stat" structure.
/// If a field is meaningless or semi-meaningless (e.g., st_ino) then it should be set to 0
/// or given a "reasonable" value
/// Upon successful completion, 0 is returned. Otherwise errno is set to error and the corresponding error is returned.
static int
fisopfs_getattr(const char *path, struct stat *st)
{
	printf("[implemented] fisop_getattr\n");
	if (strcmp(path, "/") == 0) {
		st->st_uid = 1717;
		st->st_mode = __S_IFDIR | 0755;
		st->st_nlink = 2;
	} else {
		int inode_index = get_inode_from_path(path);
		if (inode_index < 0) {
			errno = ENOENT;
			return -ENOENT;
		}
		struct inode inode = inodes[inode_index];
		// Completo los campos segun sys/stat.h (https://pubs.opengroup.org/onlinepubs/7908799/xsh/sysstat.h.html)
		st->st_dev = 0;  // ID of device containing file --> MEANINGLESS
		st->st_ino = inode.inum;           // file serial number
		st->st_mode = inode.mode;          // mode of file
		st->st_nlink = inode.links_count;  // number of links to the file
		st->st_uid = inode.user_id;        // user ID of file
		st->st_gid = 0;  // group ID of file --> MEANINGLESS
		st->st_rdev =
		        0;  // device ID (if file is character or block special) --> MEANINGLESS
		st->st_mode = __S_IFDIR | 0755;
		if (inode.type == REG_T) {
			// habia que poner los modos explicitos para que funcione
			st->st_mode = __S_IFREG | 0644;
			st->st_size =
			        inode.file_size;  // file size in bytes (if file is a regular file)
		}
		st->st_atime = inode.last_acceced_time;  // time of last access
		st->st_mtime =
		        inode.modification_time;  // time of last data modification
		st->st_ctime = inode.creation_time;  // time of last status change
		st->st_blksize = BLOCK_SIZE;  // block size for this object
		st->st_blocks =
		        inode.number_of_blocks;  // number of blocks allocated for this object
	}
	return 0;
}

/// Return one or more directory entries to the caller.
/// Has the task of telling FUSE the exact structure of the accessed directory
static int
fisopfs_readdir(const char *path,
                void *buffer,
                fuse_fill_dir_t filler,
                off_t offset,
                struct fuse_file_info *fi)
{
	printf("[implemented] fisopfs_readdir - path: %s\n", path);
	// Los directorios '.' y '..'
	filler(buffer, ".", NULL, 0);
	filler(buffer, "..", NULL, 0);
	if (strcmp(path, "/") == 0) {
		for (int i = 0; i < AMOUNT_OF_INODES; i++) {
			// si hay un inodo ocupado y solo tiene un `/` en su
			// path significa que esta dentro de este directorio
			if (superblock.inode_bitmap[i] == USED &&
			    amount_of_dentries_in_path(inodes[i].name) == 1) {
				int pos_last_dentry = 1;
				// me salteo el `/` del nombre
				filler(buffer,
				       inodes[i].name + pos_last_dentry,
				       NULL,
				       0);
			}
		}
		return 0;
	}
	for (int i = 0; i < AMOUNT_OF_INODES; i++) {
		if (superblock.inode_bitmap[i] == USED &&
		    is_in_dir(path, inodes[i].name)) {
			int pos_last_dentry = get_last_dentry(inodes[i].name);
			if (pos_last_dentry < 0) {
				return -ENOENT;
			}
			// me salteo el `/` del nombre
			filler(buffer, inodes[i].name + pos_last_dentry + 1, NULL, 0);
		}
	}
	return 0;
}

/// @brief Crea un nuevo directorio
/// @param path nombre del nuevo directorio
/// @param mode modos con los que se va a crear el inodo que represente al nuevo directorio
/// @return 0 en caso de que se cree correctamente. Setea errno y devuelve error correspondiente en caso contrario.
static int
fisopfs_mkdir(const char *path, mode_t mode)
{
	printf("[implemented] fisop_mkdir\n");
	return create_new_inode(path, mode, DIR_T);
}

/// @brief Dado un path a un archivo, elimina el inodo que represennta al archivo a ser eliminado.
/// @param path path del archivo que se quiere eliminar
/// @return 0 si se elimino correctamente. En caso contrario,
/// se setea errno al error correspondiente y se devuelve ese error.
static int
fisopfs_unlink(const char *path)
{
	printf("[implemented] fisop_unlink\n");
	int inode_index = get_inode_from_path(path);
	if (inode_index < 0) {
		errno = ENOENT;
		return -ENOENT;
	}
	if (inodes[inode_index].type == DIR_T) {
		errno = EISDIR;
		return -EISDIR;
	}
	superblock.inode_bitmap[inode_index] = FREE;
	for (int i = 0; i < inodes[inode_index].number_of_blocks; i++) {
		// me fijo si tiene un indice valido (bloque en uso) o si tiene un 0 (bloque no usado)
		if (inodes[inode_index].blocks_index[i] != FREE) {
			// seteo el puntero a todo 0 asi vuelve a estar vacio el bloque
			memset(inodes[inode_index].file_data[i],
			       0,
			       sizeof(struct block));
			memset(inodes[inode_index].name, 0, MAX_NAME_LARGE);
			// libero el bloque en el bitmap
			int block_index = inodes[inode_index].blocks_index[i];
			superblock.data_bitmap[block_index] = FREE;
			// ese bloque esta vacio
			inodes[inode_index].blocks_index[i] = FREE;
		}
	}
	return 0;
}

/// @brief Dado un path a un directorio, elimina el inodo que represennta al
/// directorio a ser eliminado.
/// @param path path del directorio que se quiere eliminar
/// @return 0 si se elimino correctamente. En caso contrario,
/// se setea errno al error correspondiente y se devuelve ese error.
static int
fisopfs_rmdir(const char *path)
{
	printf("[implemented] fisopfs_rmdir\n");
	int inode_index = get_inode_from_path(path);
	if (inode_index < 0) {
		errno = ENOENT;
		return -ENOENT;
	}
	// not a directory
	if (inodes[inode_index].type == REG_T) {
		errno = ENOTDIR;
		return -ENOTDIR;
	}
	// tengo que borrar todo lo que contenga el directorio a borrar
	for (int i = 0; i < AMOUNT_OF_INODES; i++) {
		if (superblock.inode_bitmap[i] == USED &&
		    is_in_dir(path, inodes[i].name)) {
			int type = inodes[i].type;
			switch (type) {
			case REG_T:
				fisopfs_unlink(inodes[i].name);
				break;
			case DIR_T:
				fisopfs_rmdir(inodes[i].name);
				break;
			default:
				break;
			}
		}
	}
	superblock.inode_bitmap[inode_index] = FREE;
	for (int i = 0; i < inodes[inode_index].number_of_blocks; i++) {
		// me fijo si tiene un indice valido (bloque en uso) o si tiene un 0 (bloque no usado)
		if (inodes[inode_index].blocks_index[i] != FREE) {
			// seteo el puntero a todo 0 asi vuelve a estar vacio el bloque
			memset(inodes[inode_index].file_data[i],
			       0,
			       sizeof(struct block));
			// memset(inodes[inode_index].name, 0, MAX_NAME_LARGE);
			//  libero el bloque en el bitmap
			int block_index = inodes[inode_index].blocks_index[i];
			superblock.data_bitmap[block_index] = FREE;
			// ese bloque esta vacio, lo marco con un 0
			inodes[inode_index].blocks_index[i] = FREE;
		}
	}
	return 0;
}

static int
fisopfs_truncate(const char *path, off_t offset)
{
	
	printf("[implemented] fisopfs_truncate: %s\n", path);
	int inode_index = get_inode_from_path(path);
	if (inode_index < 0) {
	    return 0;
	}
	int pos_in_inode = offset / BLOCK_SIZE;
	for (int i = pos_in_inode; i < MAX_BLOCKS_PER_INODE; i++) {
	    if (inodes[inode_index].blocks_index[i] != FREE) {
	        if (i > pos_in_inode) {
	            memset(inodes[inode_index].file_data[i], 0, sizeof(struct block)); 
				int block_index = inodes[inode_index].blocks_index[i]; 
				superblock.data_bitmap[block_index] = FREE; 
				inodes[inode_index].blocks_index[i] = FREE; 
				inodes[inode_index].file_size = inodes[inode_index].file_size - BLOCK_SIZE; 
			} else { 
				memset(&inodes[inode_index].file_data[i]->data[offset], 0, (BLOCK_SIZE - offset) * sizeof(char));
			}
	    }
	}
	return 0;
}

/// Changes the acces and modification time of a file with nanosecond resolution.
static int
fisopfs_utimens(const char *path, const struct timespec times[2])
{
	printf("[implemented] fisop_utimens\n");
	if (times == NULL) {
	}
	int inode_index = get_inode_from_path(path);
	if (inode_index < 0) {
		errno = ENOENT;
		return -ENOENT;
	}
	if (times == NULL) {
		errno = EACCES;
		return -EACCES;
	}
	// Both time specifications are given to nanosecond resolution, but your
	// filesystem doesn't have to be that precise
	inodes[inode_index].last_acceced_time = times[0].tv_sec;
	inodes[inode_index].modification_time = times[1].tv_sec;
	return 0;
}

/// @brief Crea un nuevo archivo
/// @param path nombre del nuevo archivo
/// @param mode modos con los que se va a crear el inodo que represente al nuevo archivo
/// @return 0 en caso de que se cree correctamente. Setea errno y devuelve error correspondiente en caso contrario.
static int
fisopfs_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
	printf("[implemented] fisop_create\n");
	return create_new_inode(path, mode, REG_T);
}

/// @brief Read size bytes from the given file into the buffer buf, beginning offset bytes into the file.
/// @param path Path al archivo del que se quiere leer.
/// @param buffer En donde se guarda lo leido del archivo.
/// @param size Cantidad de bytes que se quieren leer
/// @param offset La altura del archivo por la que va leyendo
/// @return bytes leidos. Setea errno y devuelve error correspondiente en caso de error.
static int
fisopfs_read(const char *path,
             char *buffer,
             size_t size,
             off_t offset,
             struct fuse_file_info *fi)
{
	printf("[implemented] fisopfs_read - path: %s, offset: %lu, size: "
	       "%lu\n",
	       path,
	       offset,
	       size);

	int inode_index = get_inode_from_path(path);
	if (inode_index < 0) {
		errno = ENOENT;
		return -ENOENT;
	}
	inodes[inode_index].last_acceced_time = time(NULL);
	// mientras el offset no se haya pasado del maximo largo que puede tener un archivo
	int max_file_size = BLOCK_SIZE * MAX_BLOCKS_PER_INODE;
	while (offset < max_file_size) {
		printf("entro a while\n");
		// division entera entre el offset y BLOCK_SIZE
		int pos_in_inode = offset / BLOCK_SIZE;
		if (inodes[inode_index].blocks_index[pos_in_inode] != FREE) {
			printf("entro a if\n");
			// el bloque esta siendo utilizado, se lee la informacion
			// calculo la posicion que se encuentra el offset dentro del bloque
			int pos_off_in_block = offset % BLOCK_SIZE;
			int block_available = BLOCK_SIZE - pos_off_in_block;
			int n = (size < block_available) ? size : block_available;
			memcpy(buffer,
			       inodes[inode_index].file_data[pos_in_inode]->data +
			               pos_off_in_block,
			       n);
			printf("LEO: %s\n", inodes[inode_index].file_data[pos_in_inode]->data +
			               pos_off_in_block);
			if (size < block_available) {
				// se leyo todo lo que se pidio
				printf("devuelvo\n");
				return size;
			} else {
				size = size - block_available;
				offset = (offset + BLOCK_SIZE) - block_available;
			}
		}
	}
	// EOF
	return 0;
}

/// @brief Write data to an open file.
/// @param path Path del archivo en el que se quiere escribir.
/// @param buf En donde viene el contenido que se quiere escribir.
/// @param size Cantidad de bytes que se quieren escribir.
/// @param offset La altura del archivo por la que se va escribiendo.
/// @return Write should return exactly the number of bytes requested except on error.
static int
fisopfs_write(const char *path,
              const char *buf,
              size_t size,
              off_t offset,
              struct fuse_file_info *fi)
{
	printf("[implemented] fisopfs_write: %s\n", path);
	int inode_index = get_inode_from_path(path);
	// si todavia no existe el archivo, se crea
	if (inode_index < 0) {
		int new_file = create_new_inode(path, 0755, REG_T);
		if (new_file < 0) {
			// error al crear nuevo archivo
			return new_file;
		} else {
			inode_index = get_inode_from_path(path);
		}
	}
	// no se puede escribir en un directorio
	if (inodes[inode_index].type == DIR_T) {
		errno = EISDIR;
		return -EISDIR;
	}
	int max_file_size = BLOCK_SIZE * MAX_BLOCKS_PER_INODE;
	int off_after_writing = offset + size;
	// actualizo stats del archivo
	inodes[inode_index].file_size = (off_after_writing < max_file_size)
	                                        ? off_after_writing
	                                        : max_file_size;
	inodes[inode_index].last_acceced_time = time(NULL);
	inodes[inode_index].modification_time = time(NULL);
	while (offset < max_file_size) {
		// division entera entre el offset y BLOCK_SIZE
		int pos_in_inode = get_current_block_in_inode(offset);
		if (inodes[inode_index].blocks_index[pos_in_inode] != FREE) {
			int pos_off_in_block = offset % BLOCK_SIZE;
			int size_of_block_available = BLOCK_SIZE - pos_off_in_block;
			int n_bytes = (size < size_of_block_available) ? size : size_of_block_available;
			memcpy(inodes[inode_index].file_data[pos_in_inode]->data +
			               pos_off_in_block,
			       buf,
			       n_bytes);
			return n_bytes;
		} else {
			// reservo un nuevo bloque para el inodo
			int new_block = get_free_block_for_inode(inode_index, pos_in_inode);
			if (new_block < 0) {
				break;
			}
		}
	}
	return -ENOMEM;
}

/// Called on each close so that the filesystem has a chance to report delayed
/// errors. Hace lo mismo que destroy pero devulve un int.
static int
fisopfs_flush(const char *path, struct fuse_file_info *fi)
{
	printf("[implemented] fisop_flush\n");
	persist_fs_data(DEFAULT_FILE);
	return 0;
}


static struct fuse_operations operations = {
	.init = fisopfs_init,        // --> IMPLEMENTED
	.destroy = fisopfs_destroy,  // --> IMPLEMENTED
	.getattr = fisopfs_getattr,  // --> IMPLEMENTED
	.fgetattr = fisopfs_fgetattr,
	.access = fisopfs_access,
	.readlink = fisopfs_readlink,
	.readdir = fisopfs_readdir,  // --> IMPLEMENTED
	.mknod = fisopfs_mknod,
	.mkdir = fisopfs_mkdir,      // --> IMPLEMENTED
	.symlink = fisopfs_symlink,
	.unlink = fisopfs_unlink,    // --> IMPLEMENTED
	.rmdir = fisopfs_rmdir,      // --> IMPLEMENTED
	.rename = fisopfs_rename,
	.link = fisopfs_link,
	.chmod = fisopfs_chmod,
	.chown = fisopfs_chown,
	.truncate = fisopfs_truncate,  // implemented
	.ftruncate = fisopfs_ftruncate,
	.utimens = fisopfs_utimens,    // --> IMPLEMENTED
	.create = fisopfs_create,      // --> IMPLEMENTED
	.open = fisopfs_open,
	.read = fisopfs_read,          // implemented
	.write = fisopfs_write,        // --> IMPLEMENTED
	.statfs = fisopfs_statfs,
	.release = fisopfs_release,
	.opendir = fisopfs_opendir,
	.releasedir = fisopfs_releasedir,
	.fsync = fisopfs_fsync,
	.flush = fisopfs_flush,  // -> IMPLEMENTED
	.fsyncdir = fisopfs_fsyncdir,
	.lock = fisopfs_lock,
	.bmap = fisopfs_bmap,
	.ioctl = fisopfs_ioctl,
	.poll = fisopfs_poll,
};

int
main(int argc, char *argv[])
{
	return fuse_main(argc, argv, &operations, NULL);
}


/* ----------------------------------------------------------- *
 *                  AUXILIAR FUNCTIONS                         *
 * ----------------------------------------------------------- */


/* ----------------------------------------------------------- *
 *                  DISK PERSISTENCE                           *
 * ----------------------------------------------------------- */


/// Crea el archivo donde se persistira el file_system, escribe los datos en el
/// archivo e inicializa todos los structs necesarios para el manejo del file system
void
create_default_file()
{
	superblock.blocks = AMOUNT_OF_BLOCKS;
	superblock.inodes = AMOUNT_OF_INODES;
	superblock.inode_bitmap = inode_bitmap;
	superblock.data_bitmap = data_bitmap;
	persist_fs_data(DEFAULT_FILE);
}


void
write_data(void *data, size_t size, size_t nitems, FILE *file)
{
	if (fwrite(data, size, nitems, file) < 0) {
		printf("Error al persistir file system en archivo!\n");
	}
}

/// Escribe en disco todas las estructuras del filesystem que contienen el
/// estado actual con todos sus bloques e inodos para que puedan ser persistidos
void
persist_fs_data(const char *file)
{
	FILE *r_file;
	r_file = fopen(file, "wb");
	if (!r_file) {
		printf("Error al abrir el archivo para escritura.\n");
		return;
	}
	// Guardar superbloque en disco
	write_data(&superblock, sizeof(struct superblock), 1, r_file);
	// Guardar bitmap de inodos en disco
	write_data(inode_bitmap, sizeof(int), AMOUNT_OF_INODES, r_file);
	// Guardar bitmap de bloques de datos en disco
	write_data(data_bitmap, sizeof(int), AMOUNT_OF_BLOCKS, r_file);
	// Guardar inodos en disco
	write_data(inodes, sizeof(struct inode), AMOUNT_OF_INODES, r_file);
	// Guardar los bloques de datos en disco
	write_data(data_region_blocks, sizeof(struct block), AMOUNT_OF_BLOCKS, r_file);
	fclose(r_file);
}

/// Lee la informacion que esta persistida sobre el file system y la carga en
/// los distintos structs que se necesitan para manejar el file system
int
unmarshall_default_file(FILE *file)
{
	// para guardar el resultado de las lecturas del archivo
	int read_result;
	read_result = fread(&superblock, sizeof(struct superblock), 1, file);
	if (read_result < 0)
		return -1;
	read_result = fread(inode_bitmap, sizeof(int), AMOUNT_OF_INODES, file);
	if (read_result < 0)
		return -1;
	read_result = fread(data_bitmap, sizeof(int), AMOUNT_OF_BLOCKS, file);
	if (read_result < 0)
		return -1;
	read_result = fread(inodes, sizeof(struct inode), AMOUNT_OF_INODES, file);
	if (read_result < 0)
		return -1;
	read_result = fread(
	        data_region_blocks, sizeof(struct block), AMOUNT_OF_BLOCKS, file);
	if (read_result < 0)
		return -1;
	superblock.inode_bitmap = inode_bitmap;
	superblock.data_bitmap = data_bitmap;
	return 0;
}


/* ----------------------------------------------------------- *
 *                  INODE/BLOCK OPERATIONS                     *
 * ----------------------------------------------------------- */


/// @brief Busca por el indice del inodo que representa al path pasado por parametro.
/// @param path Path del inodo que se quiere obtener.
/// @return Indice del inodo en la lista que contiene todos los inodos. -1 en caso de no encotrarlo.
int
get_inode_from_path(const char *path)
{
	for (int i = 0; i < AMOUNT_OF_INODES; i++) {
		if (superblock.inode_bitmap[i] == USED &&
		    strcmp(inodes[i].name, path) == 0) {
			return i;
		}
	}
	return -1;
}

/// @brief Busca el primer bloque libre que encuentre en el bitmap.
/// @return Indice del bloque si encuentra un bloque libre. -1 en caso de que esten todos los bloques ocupados.
int
get_free_block_index()
{
	// asi siempre devuelve un numero mayor a 0. el indice 0 esta reservado
	// para marcar los bloque que no se usan de un inodo
	int block = 1;
	while (data_bitmap[block] != FREE) {
		if (block >= AMOUNT_OF_BLOCKS) {
			return -1;
		}
		block++;
	}
	return block;
}

int
get_free_block_for_inode(int inode_index, int pos_offset)
{
	int free_block_index = get_free_block_index();
	if (free_block_index < 0) {
		errno = ENOMEM;
		return -ENOMEM;
	}
	superblock.data_bitmap[free_block_index] = USED;
	inodes[inode_index].blocks_index[pos_offset] = free_block_index;
	inodes[inode_index].file_data[pos_offset] =
	        &data_region_blocks[free_block_index];
	return 0;
}

/// @brief Busca el primer inodo libre que encuentre en el bitmap.
/// @return Indice del inodo si encuentra un inodo libre. -1 en caso de que esten todos los inodos ocupados.
int
get_free_inode_index()
{
	int inode = 0;
	while (inode_bitmap[inode] != FREE) {
		if (inode >= AMOUNT_OF_INODES) {
			return -1;
		}
		inode++;
	}
	return inode;
}

/// @brief Crea un nuevo inodo, lo incializa, lo marca como ocupado y lo guarda en la lista de inodos.
/// @param path nombre con el que se va a crear el inodo.
/// @param mode modos que va a tener el inodo.
/// @param type si es directorio o archivo regular.
/// @return 0 en caso de que se cree correctamente. Setea errno y devuelve error correspondiente en caso contrario.
int
create_new_inode(const char *path, mode_t mode, int type)
{
	if (strlen(path) > MAX_NAME_LARGE) {
		errno = ENAMETOOLONG;
		return -ENAMETOOLONG;
	}
	int block_index = get_free_block_index();
	int inode_index = get_free_inode_index();
	if (inode_index < 0 || block_index < 0) {
		errno = ENOSPC;
		return -ENOSPC;
	}
	// marco bloque e inodo como usados
	superblock.data_bitmap[block_index] = USED;
	superblock.inode_bitmap[inode_index] = USED;
	// completo toda la informacion del inodo
	inodes[inode_index].inum = inode_index;
	inodes[inode_index].mode = mode;
	inodes[inode_index].type = type;
	inodes[inode_index].user_id = geteuid();
	inodes[inode_index].file_size = 0;
	time_t curr_time = time(NULL);
	inodes[inode_index].creation_time = curr_time;
	inodes[inode_index].last_acceced_time = curr_time;
	inodes[inode_index].modification_time = curr_time;
	// si es archivo regular tiene un solo link, si es direcotorio tiene 2
	inodes[inode_index].links_count = (type == REG_T) ? 1 : 2;
	inodes[inode_index].number_of_blocks = 1;
	// es el primer bloque (por ahora vacio). Apunta a un bloque vacio
	inodes[inode_index].blocks_index[0] = block_index;
	inodes[inode_index].file_data[0] = &data_region_blocks[block_index];
	strcpy(inodes[inode_index].name, path);
	printf("NUEVO INDODO CREADOO!!!!!: %s, index: %d\n", path, inode_index);
	return 0;
}

/// @brief Devuelve el indice de la lista de bloques del inodo en el que se encuentra el offset
/// @param offset Repsenta la altura por la que se esta parado en el archivo
/// @return Indice del bloque en el que se encuentra el offset
int get_current_block_in_inode(int offset) {
	return offset / BLOCK_SIZE;
}


/* ----------------------------------------------------------- *
 *                  DIRECTORY OPERATIONS                       *
 * ----------------------------------------------------------- */


/// Recorre cada carácter del path proporcionado y cuenta cuántas barras
/// diagonales ("/") se encuentran en él. Cada "/" representa una dentry
/// (entrada de directorio o componente simple de un path)
/// @param path El path del archivo o directorio a recorrer
/// @return cantidad de "/" que hay en el path
int
amount_of_dentries_in_path(const char *path)
{
	int count = 0;
	size_t length = strlen(path);
	for (size_t i = 0; i < length; i++) {
		if (path[i] == '/')
			count++;
	}
	return count;
}

/// @brief Verifica si un path está contenido dentro de un directorio
/// @param dir_path El path del directorio
/// @param path El path del archivo o directorio a verificar
/// @return 1 si el path está contenido dentro del directorio, 0 en caso contrario.
int
is_in_dir(const char *dir_path, char *path)
{
	if (dir_path == NULL || path == NULL) {
		return 0;
	}
	size_t dir_length = strlen(dir_path);
	size_t path_length = strlen(path);
	// el path tiene que ser mas largo del directorio que lo contiene
	if (dir_length >= path_length) {
		return 0;
	}
	if (strncmp(dir_path, path, dir_length) == 0) {
		// entonces el path esta contenido en el path del directorio
		return 1;
	}
	return 0;
}

/// @brief Dado un path se encarga de devolver la posicion dentro del arrgelo de
/// caracteres donde se encuentra el ultimo /
/// @param path path en donde se quiere encontrar el ultimo /
/// @return la posicion en el arreglo de caracteres que se encuentra el ultimo
/// /. -1 en caso que no se encuentre
int
get_last_dentry(const char *path)
{
	if (path == NULL) {
		return -1;
	}
	size_t path_length = strlen(path);
	int curr_slash = -1;
	for (int i = 0; i < path_length; i++) {
		if (path[i] == '/') {
			curr_slash = i;
		}
	}
	return curr_slash;
}
