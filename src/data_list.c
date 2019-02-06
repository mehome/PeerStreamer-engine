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
    dataNode_t * secLast = NULL;
    dataVal_t retval;
    retval.size = -1;

    dataNode_t * next_node = NULL;

    if (head == NULL || *head == NULL) {
        return retval;
    }

    while ((*head)->next != NULL) {  
        secLast = *head;
        *head = (*head)->next;  
    }  

    retval.size = (*head)->val.size;
    retval.data = (*head)->val.data;
    if(secLast)
        secLast->next = NULL;
    free(*head);
    *head = secLast;

    return retval;
}