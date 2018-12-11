#include "LinkedList.h"
#include <string.h>
#include "limits.h"
#include "stdlib.h"
#include "stdio.h"

_node head = {
    .line = -1,
    .label = "",
    .next = &tail,
    .prev = NULL,
};

_node tail = {
    .line = INT_MAX,
    .label = "",
    .next = NULL,
    .prev = &head,
};

int _nodecount = 0;

int ClearList(){
    _node *curr = &head;
    curr = curr->next;
    while(curr != &tail){
        curr = curr->next;
        free(curr->prev->label);
        free(curr->prev);
        _nodecount--;
    }
    head.next = &tail;
    tail.prev = &head;
    return _LinkedListSuccess;
}

_node* CreateNode(int line, char* label){
    _node *newNode = (_node *) malloc(sizeof(_node));

    if(newNode == NULL)
        return NULL;

    newNode->next = NULL;
    newNode->prev = NULL;

    newNode->line = line;
    newNode->label = strdup(label);
    if(newNode->label == NULL){
        free(newNode);
        return NULL;
    }

    return newNode;
}

int InsertNode(_node *node, int overwrite){
    _node *curr, prev, next;
    curr = &head;
    while(curr->line < node->line){
        curr = curr->next;
    }
    if(curr == NULL){
        return _LinkedListFail;
    }
    if(curr->line == node->line && overwrite == 0){
        return _LinkedListExisting;
    }
    curr->prev->next = node;
    node->prev = curr->prev;
    node->next = curr;
    curr->prev = node;
    _nodecount++;
    return _LinkedListSuccess;
}

int RemoveNodeByLine(int line){
    _node *curr = &head;
    while(curr->line < line){
        curr = curr->next;
    }
    if(curr->line != line){
        return _LinkedListNotFound;
    }
    if(curr == &head || curr == &tail){
        return _LinkedListFail;
    }
    return RemoveNode(curr);
}

int RemoveNodeByLabel(char* label){
    _node *curr = &head;
    while(strcmp(curr->label, label) != 0){
        curr = curr->next;
        if(curr == &tail){
            return _LinkedListNotFound;
        }
    }
    return RemoveNode(curr);
}

int RemoveNodeByIndex(int index){
    if(index < 0 || index > _nodecount){
        return _LinkedListFail;
    }
    _node *curr = &head;
    curr = curr->next;
    if(curr == &tail){
        return _LinkedListFail;
    }
    for(int i = 0; i < index; i++){
        curr = curr->next;
    }
    return RemoveNode(curr);
}

int RemoveNode(_node *node){
    node->prev->next = node->next;
    node->next->prev = node->prev;
    free(node->label);
    free(node);
    _nodecount--;
    return _LinkedListSuccess;
}

void PrintList(){
    _node *curr = &head;
    curr = curr->next;
    while(curr != &tail){
        printf("%d -> %s\n", curr->line, curr->label);
        curr = curr->next;
    }
}

_node* FindNodeByLine(int line){
    _node *curr = &head;
    while(curr->line < line){
        curr = curr->next;
    }
    if(curr->line != line){
        return NULL;
    }
    if(curr == &head || curr == &tail){
        return curr;
    }
}

_node* FindNodeByLabel(char* label){
    _node *curr = &head;
    while(strcmp(curr->label, label) != 0){
        curr = curr->next;
        if(curr == &tail){
            return NULL;
        }
    }
    return curr;
}

int GetLineOfLabel(char *label){
    _node *node = FindNodeByLabel(label);
    if(node == NULL){
        return -1;
    }
    return node->line;
}

int Count(){
    return _nodecount;
}