#include<malloc.h>
#include<assert.h>
#include<string.h>
#include<time.h>
#include<endpoint.h>
#include<net_helper.h>
#include<stdio.h>
#include<fragment.h>
#include<ack_waiting.h>
#include<ord_set.h>

void test_waiting_acks()
{
    struct nodeID *n1, *n2;

    char msg[] = "ciao";

    fprintf(stderr, "create_nodes \n");
    n1 = create_node("10.0.0.1", 6000);
    n2 = create_node("10.0.0.2", 6000);

    fprintf(stderr, "create_endpoints \n");
    struct endpoint *e1, *e2;
    e1 = endpoint_create(n1, 256, 100000);
    e2 = endpoint_create(n2, 256, 100000);

    // END INIT

    fprintf(stderr, "create_fragments \n");
    struct fragment f1;
    fragment_init(&f1, n1, n2, 0, 1, 0, FRAGMENT_TYPE_RELIABLE, (uint8_t *)msg, 5, NULL);

    struct fragment f2;
    fragment_init(&f2, n1, n2, 1, 2, 0, FRAGMENT_TYPE_RELIABLE, (uint8_t *)msg, 5, NULL);

    struct fragment f3;
    fragment_init(&f3, n1, n2, 1, 2, 1, FRAGMENT_TYPE_RELIABLE, (uint8_t *)msg, 5, NULL);

    struct timeval t1;
    t1.tv_sec = 0;
	t1.tv_usec = 0;

    struct timeval t2;
    t2.tv_sec = 1;
	t2.tv_usec = 0;

    struct timeval t3;
    t3.tv_sec = 1;
	t3.tv_usec = 0;

    endpoint_push_waiting_ack(e1, &f3, t3);
    endpoint_push_waiting_ack(e1, &f1, t1);
    endpoint_push_waiting_ack(e1, &f2, t2);    

    fprintf(stderr, "test superato! \n");

    endpoint_remove_waiting_ack(e1, 0, 0);
    endpoint_remove_waiting_ack(e1, 1, 1);

    fprintf(stderr, "test superato! \n"); 

    endpoint_print_waiting_acks(e1);

    fprintf(stderr, "test superato! \n");   

    struct timeval tLong;
    tLong.tv_sec = 100;
	tLong.tv_usec = 0;

    struct ack_waiting *aw1; 
    aw1 = (struct ack_waiting *)endpoint_pop_waiting_ack(e1, tLong);
    ack_waiting_destroy(aw1);
    
    endpoint_print_waiting_acks(e1);

    fragment_deinit(&f1);
    fragment_deinit(&f2);
    fragment_deinit(&f3);
    endpoint_destroy(&e2);
    endpoint_destroy(&e1);

    nodeid_free(n1);
    nodeid_free(n2);

    fprintf(stderr, "test superato! \n");

}

int main()
{   
    test_waiting_acks();

    
    return 0;
}