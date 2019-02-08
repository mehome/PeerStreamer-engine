#include<data_list.h>

void push(dataNode_t ** head, dataVal_t val)
{
    dataNode_t * new_node;
    new_node = malloc(sizeof(dataNode_t));

    new_node->val = val;
    new_node->next = NULL;
    if(head)
        new_node->next = *head;
    *head = new_node;
}

dataVal_t pop_bottom(dataNode_t ** head)
{
    dataNode_t * i;
    dataNode_t * secLast = NULL;
    dataVal_t retval;
    retval.size = -1;

    dataNode_t * next_node = NULL;

    if (head == NULL || *head == NULL) {
        return retval;
    }

    i = *head;

    while ((i)->next != NULL) {  
        secLast = i;
        i = (i)->next;  
    }  

    retval.size = (i)->val.size;
    retval.data = (i)->val.data;

    if(secLast)
        secLast->next = NULL;

    free(i);

    if(secLast == NULL)
        *head = NULL;

    return retval;
}