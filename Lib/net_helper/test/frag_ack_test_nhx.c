#include<malloc.h>
#include<frag_ack.h>
#include <stdio.h>
#include<net_helper.h>
#include<assert.h>

void test_create_destroy()
{

    struct nodeID *n1, *n2;
    n1 = net_helper_init("127.0.0.1", 6000, NULL);
	n2 = net_helper_init("127.0.0.1", 6001, NULL);

    struct frag_ack *fa;
    fa = frag_ack_create(n1, n2, 0, 0, NULL);

    assert( fa != NULL);

    frag_ack_destroy(&fa);

    net_helper_deinit(n1);
    net_helper_deinit(n2);

    fprintf(stderr, "test_create_destroy, OK! \n");
}

void test_frag_ack_encode_decode()
{
    struct nodeID *n1, *n2;
    size_t buff_len = 100;
    uint8_t buff[100];
    
    struct frag_ack *fa;
    struct frag_ack *fa_decoded;
    int8_t res;

    n1 = net_helper_init("127.0.0.1", 6000, NULL);
	n2 = net_helper_init("127.0.0.1", 6001, NULL);

    fa = frag_ack_create(n1, n2, 0, 0, NULL);

    assert( fa != NULL);

    res = frag_ack_encode(fa, buff, buff_len);
    
    assert(res > -1);

    fa_decoded = frag_ack_decode(n1, n2, buff, buff_len);

    assert( memcmp(fa, fa_decoded, sizeof(struct frag_ack)) );

    net_helper_deinit(n1);
    net_helper_deinit(n2);

    fprintf(stderr, "test_frag_ack_encode_decode, OK! \n");
}

int main(){

    test_create_destroy();

    test_frag_ack_encode_decode();

    return 0;
}