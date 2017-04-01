/*
 * vfs.c
 *
 *  Created on: 19 mar. 2017
 *      Author: joaquin
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <filesystem/vfs.h>


fs_node_t *fs_root = 0; // The root of the filesystem.

uint32_t read_fs(fs_node_t *node, uint32_t offset, uint32_t size,uint8_t *buffer) {
	// Has the node got a read callback?
	if (node->read != 0)
		return node->read(node, offset, size, buffer);
	else
		return NULL;
}

uint32_t write_fs(fs_node_t *node, uint32_t offset, uint32_t size,uint8_t *buffer) {
	// Has the node got a write callback?
	if (node->write != 0)
		return node->write(node, offset, size, buffer);
	else
		return NULL;
}

void open_fs(fs_node_t *node, uint8_t read, uint8_t write) {
	// Has the node got a open callback?
	if (node->open != 0)
		node->open(node, read, write);
}

void close_fs(fs_node_t *node){
	// Has the node got a close callback?
	if (node->close != 0)
		node->close(node);
}

struct dirent *readdir_fs(fs_node_t *node, uint32_t index){
	if ((node->flags&0x7) == FS_DIRECTORY && node->readdir != 0 ){
		return node->readdir(node, index);
	}else
		return NULL;
}

fs_node_t *finddir_fs(fs_node_t *node, char *name){
	if ((node->flags&0x7) == FS_DIRECTORY && node->finddir != 0 ){
		return node->finddir(node, name);
	}else
		return NULL;
}
