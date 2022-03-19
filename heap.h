#ifndef _HEAP_
#define _HEAP_

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>

typedef struct Node_s
{
    int x, y;
    double g, f, h;
    struct Node_s* parent;
} Node_s;

unsigned char equal_nodes(Node_s A, Node_s B);

unsigned char cmpAlg(Node_s A, Node_s B);

typedef struct Heap_node_s
{
    struct Heap_node_s* next;
    Node_s* node;
    struct Heap_node_s* prev;
} Heap_node_s;

typedef struct Node_heap_s
{
    Heap_node_s* first;
    Heap_node_s* last;
} Node_heap_s;

void push(Node_heap_s* heap, Node_s* node)
{
    if(heap->first == NULL)
    {
        heap->first = (Heap_node_s*)malloc(sizeof(Heap_node_s));
        heap->first->next = NULL;
        heap->first->prev = NULL;
        heap->first->node = node;
        heap->last = heap->first;
        return;
    }

    Heap_node_s* newNode = (Heap_node_s*)malloc(sizeof(Heap_node_s));
    Heap_node_s* currentNode = heap->first;
    newNode->next = NULL;
    newNode->prev = NULL;
    newNode->node = node;

    if(cmpAlg(*newNode->node, *heap->first->node)){
        newNode->prev = heap->first;
        heap->first->next = newNode;
        heap->first = newNode;
        return;
    }

    if(!cmpAlg(*newNode->node, *heap->last->node))
    {
        newNode->next = heap->last;
        heap->last->prev = newNode;
        heap->last = newNode;
        return;
    }

    while(currentNode->prev != NULL)
    {
        if(cmpAlg(*newNode->node, *currentNode->node))
        {
            newNode->prev = currentNode;
            newNode->next = currentNode->next;
            currentNode->next = newNode;
            currentNode->next->prev = newNode;
            break;
        }

        currentNode = currentNode->prev;
    }
}

void update(Node_heap_s* heap, Node_s* node)
{
    Heap_node_s* current = heap->first;
    while(!equal_nodes(*node, *current->node) || current->prev != NULL)
        current = current->prev;

    if(current->prev != NULL)
    {
        current->next->prev = current->prev;
        current->prev->next = current->next;
        current->next = NULL;
        current->prev = NULL;
        free(current);

        push(heap, node);
    }
}

Node_s* pop(Node_heap_s* heap)
{
    Node_s* node = NULL;
    if(heap->first == NULL)
        return node;
    node = heap->first->node;
    heap->first->node = NULL;

    if(heap->first == heap->last)
    {
        free(heap->first);
        heap->first = NULL;
        heap->last = NULL;
        return node;
    }

    heap->first = heap->first->prev;
    free(heap->first->next);
    heap->first->next = NULL;
    return node;
}

void free_heap(Node_heap_s* heap)
{
    Heap_node_s* current = heap->first;
    if(heap->first != NULL)
    {
        heap->first = NULL;
    heap->last = NULL;

    while(current->prev != NULL){
        Heap_node_s* plch = current;
        Node_s* current_node = plch->node;
        current->node = NULL;
        current->next = NULL;
        current = current->prev;
        plch->prev = NULL;
        while(current_node->parent != NULL)
        {
            Node_s* plch_node = current_node;
            //printf("T(%d, %d)\n", plch_node->x, plch_node->y);
            current_node = plch_node->parent;
            plch_node->parent = NULL;
            free(plch_node);
        }
        free(plch);
    }
    }
}

unsigned char is_empty(Node_heap_s* heap)
{
    return heap->first == NULL;
}

unsigned char cmpAlg(Node_s A, Node_s B)
{
    return A.f < B.f;
}

unsigned char equal_nodes(Node_s A, Node_s B)
{
    return (A.x == B.x && A.y == B.y);
}

#endif