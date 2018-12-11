#include "stdlib.h"

#ifndef _LinkedList
#define _LinkedList

#define _LinkedListSuccess 0
#define _LinkedListFail -1
#define _LinkedListNotFound -2
#define _LinkedListExisting -3

typedef struct _node{
    int line;
    char* label;
    struct _node *next;
    struct _node *prev;
}_node;

_node head, tail;

int _nodecount;

int ClearList();

_node* CreateNode(int line, char* label);

int InsertNode(_node *node, int overwrite);

int RemoveNodeByLine(int line);

int RemoveNodeByLabel(char* label);

int RemoveNodeByIndex(int index);

int RemoveNode(_node *node);

int Count();

_node* FindNodeByLine(int line);

_node* FindNodeByLabel(char* label);

int GetLineOfLabel(char* label);

void PrintList();

#endif