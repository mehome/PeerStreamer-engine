#include <malloc.h>
#include <assert.h>
#include <string.h>
#include <psinstance.h>
#include <signal.h>
#include <inttypes.h>
#include <sys/time.h>

int running = 1;

void leave(int sig) {
	running = 0;
	fprintf(stderr, "Received signal %d, exiting!\n", sig);
}

void poll(struct psinstance *ps1, struct psinstance *ps2)
{
    psinstance_poll(ps1, 5000000);
    psinstance_poll(ps2, 5000000);

    psinstance_network_periodic(ps1);
    psinstance_network_periodic(ps2);
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
    uint64_t epochLastTime;

    ps1 = psinstance_create("port=6666,flow_id=10,base_out=5000,base_in=7000,dechunkiser=rtp_multi,chunkiser=rtp_multi,chunkbuffer_size=8,chunk_size=1000,max_delay_ms=25,iface=lo,source_multiplicity=1");
    ps2 = psinstance_create("port=6667,bs_port=6666,bs_addr=127.0.0.1,flow_id=20,base_out=5100,base_in=7100,dechunkiser=rtp_multi,chunkiser=rtp_multi,chunkbuffer_size=8,chunk_size=1000,max_delay_ms=25,iface=lo,source_multiplicity=1");

    int i = 0;
    while(ps1 && ps2 && running) {
        char time_string[17];  
        char data[31];
        struct timeval now;
        uint64_t epochNow;
        uint8_t res;

        poll(ps1, ps2);
        
        gettimeofday(&now, NULL);
        epochNow = (uint64_t)(now.tv_sec * 1000000ULL + now.tv_usec);

        if(epochNow - epochLastTime > 1000001) {
            epochLastTime = epochNow;
            sprintf(time_string, "%" PRIu64, epochNow);
            sprintf(data, "%d", i);

            strcat(data, ";");
            strcat(data, time_string);
            
            res = psinstance_inject_data_chunk(ps1, &data, strlen(data)+1);
            if (res == 0)
                i++;
        }

       if(i > 150) {
           running = 0;
       }
    }
    


    return 0;
}