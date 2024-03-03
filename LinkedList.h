#pragma once

/*DATA TIME SEG ---------------------------------------*/
typedef struct DataTimeSeg {
    int songID;
    int timeInterval;
} DataTimeSeg;

typedef struct node {
    DataTimeSeg value;
    struct node* next;
}node;

typedef struct ll {
    uint32_t size;
    node* head;
}ll, * list;

list llCreate();
void llAdd(list l, DataTimeSeg value);
void llPrint(list l);
void llDelete(list* l);