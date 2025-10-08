#ifndef DOUBLE_LINKED_LIST_H
#define DOUBLE_LINKED_LIST_H

#include <stdint.h>

typedef struct Node{
    void *info;
    struct Node *next;
    struct Node *prev;
} Node;

typedef struct Node * TNode;

typedef struct DListCDT{
    TNode head;
    TNode tail;
    int size;
} DListCDT;

typedef struct DListCDT * DListADT;

DListADT createDList();

int add_first(DListADT list, void * info);

int add_last(DListADT list, void * info);

int delete_first(DListADT list);

int delete_last(DListADT list);

int delete_element(DListADT list, void * info);

void * get_first(DListADT list);

void * get_last(DListADT list);

int get_size(DListADT list);

int is_empty(DListADT list);

void free_list(DListADT list);

#endif 