#include "doubleLinkedList.h"
#include "video.h"
#include "memoryManager.h"

DListADT create_list(){
    DListADT list = (DListADT) memory_alloc(sizeof(struct DListCDT));
    if(list == NULL)
        return NULL;
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
    return list;
}

static TNode new_node(void * info){
    TNode node = (TNode) memory_alloc(sizeof(Node));
    if(node == NULL)
        return NULL;
    node->info = info;
    node->next = NULL;
    node->prev = NULL;
    return node;
}

int add_first(DListADT list, void * info){
    TNode node = new_node(info);
    if(node == NULL)
        return -1;
    if(list->head == NULL){ // lista vacia
        list->head = node;
        list->tail = node;
    } else {
        node->next = list->head;
        list->head->prev = node;
        list->head = node;
    }
    list->size++;
    return 0;
}

int add_last(DListADT list, void * info){
    TNode node = new_node(info);
    if(node == NULL)
        return -1;
    if(list->head == NULL){ // lista vacia
        list->head = node;
        list->tail = node;
    } else {
        node->prev = list->tail;
        list->tail->next = node;
        list->tail = node;
    }
    list->size++;
    return 0;
}

int delete_first(DListADT list){    // elimina el primer elemento
    if(list->head == NULL) // lista vacia
        return -1;
    TNode to_delete = list->head;
    if(list->head == list->tail){ // un solo elemento
        list->head = NULL;
        list->tail = NULL;
    } else {
        list->head = list->head->next;
        list->head->prev = NULL;
    }
    memory_free(to_delete);
    list->size--;
    return 0;
}

int delete_last(DListADT list){     // elimina el ultimo elemento
    if(list->head == NULL) // lista vacia
        return -1;
    TNode to_delete = list->tail;
    if(list->head == list->tail){ // un solo elemento
        list->head = NULL;
        list->tail = NULL;
    } else {
        list->tail = list->tail->prev;
        list->tail->next = NULL;
    }
    memory_free(to_delete);
    list->size--;
    return 0;
}

int delete_element(DListADT list, void * info){
    if(list->head == NULL) // lista vacia
        return -1;
    TNode current = list->head;
    while(current != NULL){
        if(current->info == info){
            if(current == list->head){ // primer elemento
                return delete_first(list);
            } else if(current == list->tail){ // ultimo elemento
                return delete_last(list);
            } else { // elemento en el medio
                current->prev->next = current->next;
                current->next->prev = current->prev;
                memory_free(current);
                list->size--;
                return 0;
            }
        }
        current = current->next;
    }
    return -1; // no se encontro el elemento
}

void * get_first(DListADT list){
    if(list->head == NULL) // lista vacia
        return NULL;
    return list->head->info;
}

void * get_last(DListADT list){
    if(list->head == NULL) // lista vacia
        return NULL;
    return list->tail->info;
}

int get_size(DListADT list){
    return list->size;
}

int is_empty(DListADT list){
    return list->size == 0;
}

void free_list(DListADT list){
    if(list == NULL)
        return;
    TNode current = list->head;
    while(current != NULL){
        TNode to_delete = current;
        current = current->next;
        memory_free(to_delete);
    }
    memory_free(list);
}