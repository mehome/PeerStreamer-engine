#include<stdint.h>
#include<stddef.h>

typedef struct data_val
{
    int size;
    uint8_t *data;
} dataVal_t;


typedef struct data_node
{    
    dataVal_t val;
    struct data_node * next;
} dataNode_t;

void push(dataNode_t ** head, dataVal_t val);

dataVal_t pop_bottom(dataNode_t ** head);

