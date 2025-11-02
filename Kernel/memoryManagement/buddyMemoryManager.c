#include "memoryManager.h"

// // Enable this file by compiling with -DUSE_BUDDY_MM to avoid duplicate symbols
// #ifdef USE_BUDDY_MM

// Constants and helpers exactly as in the reference
#define FREE 0
#define OCCUPIED 1
#define SPLIT 2
#define MINEXPONENT 4
#define POW_2(x) ((uint64_t)1ULL << (x))
// Fixed MAXEXPONENT and derived sizes to match the working reference
#define MAXEXPONENT 28 
#define HEAPSIZE POW_2(MAXEXPONENT)
#define CANTNODES (POW_2(MAXEXPONENT - MINEXPONENT + 1) - 1)

typedef struct node{
	uint8_t state;
} node;

typedef struct node * TNode;

typedef struct MemoryManagerCDT{
	uint8_t * treeStart;
	TNode tree;
	uint64_t size;
	uint64_t used;
} MemoryManagerCDT;

// Internal globals
static MemoryManagerADT memoryManager = 0;
static memory_info_t buddy_info;

// Helpers forward declarations (names like the reference)
static uint8_t getExponentPtr(void * memoryToFree);
static uint8_t getExponent(uint64_t size);
static int64_t findFreeNode(uint64_t node, uint8_t level, uint8_t targetLevel);
static uint64_t getNodeLevel(uint8_t exponent);
static int64_t getNodeIndex(uint8_t * ptr, uint8_t * exponent);
static void setMerge(uint64_t node);
static void splitTree(uint64_t node);
static void setSplitedChildren(uint64_t node);

void memory_init(void * start, uint64_t managedMemory) {
    if (managedMemory < POW_2(MINEXPONENT)) return;
    memoryManager = (MemoryManagerADT) start;
    memoryManager->treeStart = ((uint8_t *)start) + sizeof(MemoryManagerCDT);
    memoryManager->tree = (TNode)(memoryManager->treeStart + HEAPSIZE);
    memoryManager->size = HEAPSIZE;
    memoryManager->used = 0;
    for (uint64_t i = 0; i < CANTNODES; i++) {
        memoryManager->tree[i].state = FREE;
    }
    buddy_info.size = memoryManager->size;
    buddy_info.used = 0;
    buddy_info.free = memoryManager->size;
}

void * memory_alloc(uint64_t memoryToAllocate) {
	if (memoryManager == 0) return NULL;
	if (memoryToAllocate == 0 || memoryToAllocate > memoryManager->size - memoryManager->used) return NULL;

    uint8_t exponent = getExponent(memoryToAllocate);
    int64_t node = findFreeNode(0, 0, (uint8_t)(MAXEXPONENT - exponent));
	if (node == -1) return NULL;

	splitTree((uint64_t)node);
	setSplitedChildren((uint64_t)node);
	memoryManager->used += POW_2(exponent);

    return (void *)(memoryManager->treeStart + (((uint64_t)node - getNodeLevel(exponent)) * POW_2(exponent)));
}

void memory_free(void * ptr) {
	if (memoryManager == 0 || ptr == NULL) return;

    uint8_t exponent = getExponentPtr(ptr);
    uint8_t node = (uint8_t)getNodeIndex((uint8_t *)ptr, &exponent);
	memoryManager->tree[node].state = FREE;
	setMerge(node);
	memoryManager->used -= POW_2(exponent);

	buddy_info.used = memoryManager->used;
	buddy_info.free = memoryManager->size - memoryManager->used;
}

memory_info_t * memory_get_info() {	
    buddy_info.free = buddy_info.size - buddy_info.used;
    return &buddy_info;
}


// ===== Helper implementations (mirroring reference) =====

static int64_t getNodeIndex(uint8_t * ptr, uint8_t * exponent) {
    int64_t node = 0;
    uint8_t levelExponent = *exponent;
    while (levelExponent > 0 && memoryManager->tree[node].state != OCCUPIED) {
        node = (int64_t)(((ptr - memoryManager->treeStart) >> levelExponent) + getNodeLevel(levelExponent));
        levelExponent--;
    }
    *exponent = levelExponent;
    return node;
}

static uint8_t getExponentPtr(void * memoryToFree) {
    uint64_t address = (uint8_t *)memoryToFree - memoryManager->treeStart;
    uint8_t exponent = MAXEXPONENT;
    uint64_t size = POW_2(exponent);
    while ((address % size) != 0ULL) {
        exponent--;
        size >>= 1ULL;
    }
    return exponent;
}

static uint8_t getExponent(uint64_t size) {
	uint8_t exponent = MINEXPONENT;
	uint64_t pot = POW_2(MINEXPONENT);
	while (pot < size) {
		pot <<= 1ULL;
		exponent++;
	}
	return exponent;
}

static int64_t findFreeNode(uint64_t node, uint8_t level, uint8_t targetLevel) {
	if (level == targetLevel) {
		if (memoryManager->tree[node].state == FREE) return (int64_t)node;
		return -1;
	}
	if (memoryManager->tree[node].state != SPLIT && memoryManager->tree[node].state != FREE) return -1;
	int64_t left = findFreeNode(node * 2ULL + 1ULL, (uint8_t)(level + 1U), targetLevel);
	if (left != -1) return left;
	return findFreeNode(node * 2ULL + 2ULL, (uint8_t)(level + 1U), targetLevel);
}

static uint64_t getNodeLevel(uint8_t exponent) {
    return (POW_2((uint8_t)(MAXEXPONENT - exponent)) - 1ULL);
}

static void setMerge(uint64_t node) {
	if (node == 0ULL) return;

	uint64_t buddy;
    if ((node % 2ULL) == 0ULL) {
		if (node == 0ULL) return;
		buddy = node - 1ULL;
	} else {
		buddy = node + 1ULL;
        if (buddy >= CANTNODES) return;
	}

	if (memoryManager->tree[buddy].state == FREE && memoryManager->tree[node].state == FREE) {
		uint64_t parent = (node - 1ULL) / 2ULL;
        if (parent < CANTNODES && memoryManager->tree[parent].state == SPLIT) {
			memoryManager->tree[parent].state = FREE;
			setMerge(parent);
		}
	}
}

static void splitTree(uint64_t node) {
	while (node) {
		node = (node - 1ULL) / 2ULL;
		memoryManager->tree[node].state = SPLIT;
	}
}

static void setSplitedChildren(uint64_t node) {
    if (node < CANTNODES) {
		memoryManager->tree[node].state = OCCUPIED;
	}
}

// #endif // USE_BUDDY_MM
