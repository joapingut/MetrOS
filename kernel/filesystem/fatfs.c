/*
 * fatfs.c
 *
 *  Created on: 19 mar. 2017
 *      Author: joaquin
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <filesystem/fatfs.h>


uint32_t read_fatfs(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer);
uint32_t write_fatfs(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer);
void open_fatfs(fs_node_t *node, uint8_t read, uint8_t write);
void close_fatfs(fs_node_t *node);
struct dirent *readdir_fatfs(fs_node_t *node, uint32_t index);
fs_node_t *finddir_fatfs(fs_node_t *node, char *name);
