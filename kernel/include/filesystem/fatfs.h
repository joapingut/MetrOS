/*
 * fatfs.h
 *
 *  Created on: 19 mar. 2017
 *      Author: joaquin
 */

#ifndef KERNEL_FILESYSTEM_FATFS_H_
#define KERNEL_FILESYSTEM_FATFS_H_

#include <filesystem/vfs.h>

#define TOTAL_CLUSTERS_FAT12 4085
#define TOTAL_CLUSTERS_FAT16 65525
#define TOTAL_CLUSTERS_FAT32 268435445

typedef struct {
	//extended fat32 stuff
	unsigned int table_size_32;
	unsigned short extended_flags;
	unsigned short fat_version;
	unsigned int root_cluster;
	unsigned short fat_info;
	unsigned short backup_BS_sector;
	unsigned char reserved_0[12];
	unsigned char drive_number;
	unsigned char reserved_1;
	unsigned char boot_signature;
	unsigned int volume_id;
	unsigned char volume_label[11];
	unsigned char fat_type_label[8];

}__attribute__((packed)) fat_extBS_32_t;

typedef struct {
	//extended fat12 and fat16 stuff
	unsigned char bios_drive_num;
	unsigned char reserved1;
	unsigned char boot_signature;
	unsigned int volume_id;
	unsigned char volume_label[11];
	unsigned char fat_type_label[8];

}__attribute__((packed)) fat_extBS_16_t;

typedef struct {
	unsigned char bootjmp[3];
	unsigned char oem_name[8];
	unsigned short bytes_per_sector;
	unsigned char sectors_per_cluster;
	unsigned short reserved_sector_count;
	unsigned char table_count;
	unsigned short root_entry_count;
	unsigned short total_sectors_16;
	unsigned char media_type;
	unsigned short table_size_16;
	unsigned short sectors_per_track;
	unsigned short head_side_count;
	unsigned int hidden_sector_count;
	unsigned int total_sectors_32;

	//this will be cast to it's specific type once the driver actually knows what type of FAT this is.
	unsigned char extended_section[54];

}__attribute__((packed)) fat_BS_t;

typedef struct{
	char name[8];
	char ext[3];
	uint8_t attrib;
	uint8_t userattrib;

	char undelete;
	uint16_t createtime;
	uint16_t createdate;
	uint16_t accessdate;
	uint16_t clusterhigh;

	uint16_t modifiedtime;
	uint16_t modifieddate;
	uint16_t clusterlow;
	uint32_t filesize;

} __attribute__ ((packed)) fat_directory_entry;

uint32_t read_fatfs(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer);
uint32_t write_fatfs(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer);
void open_fatfs(fs_node_t *node, uint8_t read, uint8_t write);
void close_fatfs(fs_node_t *node);
struct dirent *readdir_fatfs(fs_node_t *node, uint32_t index);
fs_node_t *finddir_fatfs(fs_node_t *node, char *name);

#endif /* KERNEL_FILESYSTEM_FATFS_H_ */
