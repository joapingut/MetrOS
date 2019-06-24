/*
 * initrd.c
 *
 *  Created on: 25 mar. 2017
 *      Author: joaquin
 */


#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <filesystem/initrd.h>

typedef struct{
	uint32_t *parent;
	uint32_t *next;
	uint32_t *child;
	uint32_t identifier;
	uint8_t *content;
}initrd_tree_node;

static uint32_t initrd_read(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer);
static struct dirent *initrd_readdir(fs_node_t *node, uint32_t index);
static fs_node_t *initrd_finddir(fs_node_t *node, char *name);
static initrd_tree_node *buildInitrdTree(initrd_node_t *node, initrd_tree_node *parent, uint32_t *identifier);
static initrd_tree_node *findInitrdNode(initrd_tree_node *node, uint32_t identifier);

initrd_header_t *initrd_header;     // The header.
initrd_tree_node *initrd_nodes_Tree;
fs_node_t *initrd_root;             // Our root directory node.
fs_node_t *initrd_dev;              // We also add a directory node for /dev, so we can mount devfs later on.
fs_node_t *root_nodes;				// Array of intrd nodes
uint8_t *initrdLocation;
int nroot_nodes;                    // Number of file nodes.

struct dirent dirent;
extern uint8_t *initrd_addr;

static uint32_t initrd_read(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer){
	initrd_tree_node *tree_node = findInitrdNode(initrd_nodes_Tree, node->inode);
	initrd_node_t *file = (initrd_node_t *) tree_node->content;
	if (offset > file->length)
		return 0;
	if (offset + size > file->length)
		size = file->length - offset;
	memcpy(buffer, (uint8_t *) (initrdLocation + file->child + offset), size);
	return size;
}

static struct dirent *initrd_readdir(fs_node_t *node, uint32_t index){
	if (node == initrd_root && index == 0){
		strcpy(dirent.name, "dev");
		dirent.name[3] = 0; // Make sure the string is NULL-terminated.
		dirent.ino = 0;
		return &dirent;
	}

	if (index-1 >= nroot_nodes)
		return 0;
	strcpy(dirent.name, root_nodes[index-1].name);
	dirent.name[strlen(root_nodes[index-1].name)] = 0; // Make sure the string is NULL-terminated.
	dirent.ino = root_nodes[index-1].inode;
	return &dirent;
}

static fs_node_t *initrd_finddir(fs_node_t *node, char *name){
	if (node == initrd_root && !strcmp(name, "dev"))
       return initrd_dev;
	int i;
	for (i = 0; i < nroot_nodes; i++)
		if (!strcmp(name, root_nodes[i].name))
			return &root_nodes[i];
   return 0;
}

static initrd_tree_node *buildInitrdTree(initrd_node_t *node, initrd_tree_node *parent, uint32_t *identifier){
	initrd_tree_node *child = (initrd_tree_node*)kmalloc(sizeof(initrd_tree_node));
	child->parent = parent;
	memcpy(&(child->identifier), identifier, sizeof(uint32_t));
	//child->identifier = *identifier;
	*identifier = *identifier + 1;
	child->content = node;
	printf("\nNode: %s", node->name);
	printf("\nIdentifier: %d 0x%x 0x%x", *identifier, parent, node->magic);
	if(isDir(node->magic)){
		printf("\nDIR");
		if(node->child != NULL){
			child->child = buildInitrdTree((initrd_node_t *)(initrdLocation + node->child), child, identifier);
		}
	}else{
		printf("\nfile");
		child->child = NULL;
	}
	if(node->next != NULL){
		printf("\nhasnext 0x%x", node->next);
		child->next = buildInitrdTree((initrd_node_t *)(initrdLocation + node->next), parent, identifier);
	}
	return child;
}

static initrd_tree_node *findInitrdNode(initrd_tree_node *node, uint32_t identifier){
	if(node->identifier == identifier){
		return node;
	}
	if(node->next != NULL){
		initrd_tree_node *next = (initrd_tree_node *) node->next;
		if(identifier >= next->identifier){
			return findInitrdNode(next, identifier);
		}
	}
	if(node->child != NULL){
		initrd_tree_node *child = (initrd_tree_node *) node->child;
		if(identifier >= child->identifier){
			return findInitrdNode(child, identifier);
		}
	}
	return NULL;
}

fs_node_t *initialise_initrd() {
	// Initialise the root directory (/)
	if(initrd_addr == NULL){
		printf("\nInitrd initializatin failed");
		return NULL;
	}
	initrdLocation = initrd_addr;
	printf("\nInitrd Location: %x", initrdLocation);
	printf("\nsizeof(fs_node_t): %x", sizeof(fs_node_t));
	initrd_root = (fs_node_t*) kmalloc(sizeof(fs_node_t));
	strcpy(initrd_root->name, "initrd");
	printf("\nMemory used: %x", memory_used());
	initrd_root->mask = initrd_root->uid = initrd_root->gid = initrd_root->inode = initrd_root->length = 0;
	initrd_root->flags = FS_DIRECTORY;
	initrd_root->read = 0;
	initrd_root->write = 0;
	initrd_root->open = 0;
	initrd_root->close = 0;
	initrd_root->readdir = &initrd_readdir;
	initrd_root->finddir = &initrd_finddir;
	initrd_root->ptr = 0;
	initrd_root->impl = 0;
	// Initialise the /dev directory (required!)
	initrd_dev = (fs_node_t*) kmalloc(sizeof(fs_node_t));
	strcpy(initrd_dev->name, "dev");
	initrd_dev->mask = initrd_dev->uid = initrd_dev->gid = initrd_dev->inode = initrd_dev->length = 0;
	initrd_dev->flags = FS_DIRECTORY;
	initrd_dev->read = 0;
	initrd_dev->write = 0;
	initrd_dev->open = 0;
	initrd_dev->close = 0;
	initrd_dev->readdir = &initrd_readdir;
	initrd_dev->finddir = &initrd_finddir;
	initrd_dev->ptr = 0;
	initrd_dev->impl = 0;
	initrd_header = (initrd_header_t *) initrdLocation;
	uint32_t identifier = 0;
	uint32_t frn = initrd_header->firstNode;
	printf("\nMemory used: %x", memory_used());
	printf("\InitrdFrn: 0x%x, frn %x and id: %d", initrdLocation,  frn, &identifier);
	initrd_nodes_Tree = buildInitrdTree((initrd_node_t *) ((uint8_t *)(initrdLocation + frn)), NULL, &identifier);
	STOP()
	root_nodes = (fs_node_t*) kmalloc(sizeof(fs_node_t) * identifier);
	nroot_nodes = identifier;
	printf("\Total: %d at 0x%x", nroot_nodes, initrd_addr);
	// For every file...
	for (uint32_t i = 0; i < identifier; i++) {
		printf("\nRd: %d : %x\n : %s", initrd_nodes_Tree->identifier, initrd_nodes_Tree->content, ((initrd_node_t *)initrd_nodes_Tree->content)->name);
		initrd_tree_node *treeNode = findInitrdNode(initrd_nodes_Tree, i);
		printf("\ntreeNode: %x", (uint32_t *)treeNode);
		printf("\ni: %x : %x", i, treeNode->identifier);
		initrd_node_t * content = (initrd_node_t *) treeNode->content;
		if (isDir(content->magic)) {
			strcpy(root_nodes[i].name, content->name);
			root_nodes[i].mask = root_nodes[i].uid = root_nodes[i].gid = root_nodes[i].inode = root_nodes[i].length = 0;
			root_nodes[i].flags = FS_DIRECTORY;
			root_nodes[i].read = 0;
			root_nodes[i].write = 0;
			root_nodes[i].open = 0;
			root_nodes[i].close = 0;
			root_nodes[i].readdir = &initrd_readdir;
			root_nodes[i].finddir = &initrd_finddir;
			root_nodes[i].ptr = 0;
			root_nodes[i].impl = 0;
		} else {
			// Create a new file node.
			strcpy(root_nodes[i].name, content->name);
			root_nodes[i].mask = root_nodes[i].uid = root_nodes[i].gid = 0;
			root_nodes[i].length = content->length;
			root_nodes[i].inode = i;
			root_nodes[i].flags = FS_FILE;
			root_nodes[i].read = &initrd_read;
			root_nodes[i].write = 0;
			root_nodes[i].readdir = 0;
			root_nodes[i].finddir = 0;
			root_nodes[i].open = 0;
			root_nodes[i].close = 0;
			root_nodes[i].impl = 0;
		}

	}
	return initrd_root;
}
