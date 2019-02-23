#include <malloc.h>
#include <assert.h>
#include <string.h>
#include <psinstance.h>
#include <signal.h>
#include <inttypes.h>
#include <sys/time.h>
#include<sys/types.h> 

int running = 1;

void leave(int sig) {
	running = 0;
	fprintf(stderr, "Received signal %d, exiting!\n", sig);
}

void poll(struct psinstance *ps)
{
    psinstance_poll(ps, 5000000);

    psinstance_network_periodic(ps);
}

void deinit(struct psinstance *ps1, struct psinstance *ps2)
{
    psinstance_destroy(&ps1);
    psinstance_destroy(&ps2);   
}

int main()
{
    (void) signal(SIGTERM, leave);
	(void) signal(SIGINT, leave);

    struct psinstance *ps1;
    struct psinstance *ps2;
    struct psinstance *ps3;
    uint64_t epochLastTime;

    ps1 = psinstance_create("port=6666,flow_id=10,base_out=5000,base_in=7000,dechunkiser=rtp_multi,chunkiser=rtp_multi,chunkbuffer_size=8,chunk_size=1000,max_delay_ms=25,iface=lo,source_multiplicity=1");
    ps2 = psinstance_create("port=6667,bs_port=6666,bs_addr=127.0.0.1,flow_id=20,base_out=5100,base_in=7100,dechunkiser=rtp_multi,chunkiser=rtp_multi,chunkbuffer_size=8,chunk_size=1000,max_delay_ms=25,iface=lo,source_multiplicity=1");
    ps3 = psinstance_create("port=6668,bs_port=6666,bs_addr=127.0.0.1,flow_id=30,base_out=5200,base_in=7200,dechunkiser=rtp_multi,chunkiser=rtp_multi,chunkbuffer_size=8,chunk_size=1000,max_delay_ms=25,iface=lo,source_multiplicity=1");

    int i = 0;
    while(ps1 && ps2 && ps3 && running) {
        char time_string[17];  
        char data[31];
        struct timeval now;
        uint64_t epochNow;
        uint64_t lastEpochNow;
        uint8_t res;
        
        poll(ps1);
        poll(ps2);
        poll(ps3);
        
        gettimeofday(&now, NULL);
        epochNow = (uint64_t)(now.tv_sec * 1000000ULL + now.tv_usec);

        if(epochNow - epochLastTime > 1000001) {
            epochLastTime = epochNow;
            sprintf(time_string, "%" PRIu64, epochNow);
            sprintf(data, "%d", i);

            strcat(data, ";");
            strcat(data, time_string);
            
            res = psinstance_inject_data_chunk(ps1, &data, strlen(data)+1);
            
            if (res == 0){
                fprintf(stderr, "data_msg_sent: s:%s \n", data);
                i++;
            }
        }

        uint8_t *dataRec = NULL;
        size_t dataRec_size = 0;
        int8_t resPop = 0;
        while(resPop >= 0)
        {
            resPop = psinstance_pop_data_chunk(ps2, &dataRec, &dataRec_size);
            if(resPop >= 0) {

                gettimeofday(&now, NULL);

                char nowString[21];
                sprintf(nowString, "%" PRIu64, (uint64_t) (now.tv_sec * 1000000ULL + now.tv_usec));
                fprintf(stderr, "data_msg_recived: s:%s r:%s\n", dataRec, nowString);

                free(dataRec);
            }
        }         

        if(i > 500) {
            running = 0;
        }
    }

    return 0;
}