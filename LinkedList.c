#include "stdafx.h"
#include "linkedList.h"

list llCreate() {
    list l = (list)malloc(sizeof(ll));
    l->size = 0;
    l->head = NULL;
    return l;
}

void llAdd(list l, DataTimeSeg value) {
    node* item = (node*)malloc(sizeof(node));
    if (!item) {
        return;
    }
    item->value = value;
    node* temp = l->head;
    if (!l->head) {
        l->head = item;
    }
    else {
        while (temp->next) {
            temp = temp->next;
        }
        temp->next = item;
    }
    item->next = NULL;
    l->size++;
}

void llPrint(list l) {
    if (!l) return;
    node* temp = l->head;
    while (temp) {
        printf(" id = %ld time = %d \n", temp->value.songID, temp->value.timeInterval);
        temp = temp->next;
    }
    puts(«»);
}

void llDelete(list* l) {
    node* temp = (*l)->head;
    node* next = NULL;
    while (temp) {
        next = temp->next;
        free(temp);
        temp = next;
    }
    (*l)->size = 0;
    free(*l);
    (*l) = NULL;
}