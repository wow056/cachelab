//Kang Minsu 20170225(student ID) powerful(POVIS ID)

#include "cachelab.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

int parse_block_offset(int address);
int parse_tag(int address);
int parse_set_index(int address);

int parse_argument(int argc, char *argv[], int *vflag, int *set_num, int *lines_per_set, int *block_bit, FILE **tracefile);
int parse_memory_trace(const char *str, long *address, int *size);

int main(int argc, char *argv[])
{
    int verbose_flag, set_num, line_num, block_bit_num;
    int memsize;
    long address;
    FILE *f;
    char buffer[100];

    parse_argument(&argc, argv, &verbose_flag, &set_num, &line_num, &block_bit_num, &f);
    printf("v:%d, s:%d, E:%d, b:%d\n",verbose_flag, set_num, line_num, block_bit_num);
    while(fgets(buffer, 100, f))
    {
        parse_memory_trace(buffer, &address, &memsize);
        printf("%s",buffer);
        printf("address: 0x%lx, memsize: %d\n", address, memsize);
    }
    printSummary(0, 0, 0);
    return 0;
    else
    {
        return -1;
    }
}

int parse_memory_trace(const char *str,long *address, int *size)
{
    char operation;
    int size_temp;
    long address_temp;
    sscanf(str,"%*[ \n\t]%c%lx,%d",&operation,&address_temp, &size_temp);
    if(operation == 'I')
    {
        return 0;
    }
    else
    {
        *address = address_temp;
        *size = size_temp;
        return 1;
    }
}

int parse_argument(int argc, char *argv[], int *vflag, int *set_num, int *lines_per_set, int *block_bit, FILE **tracefile)
{
    int vflag_temp=0, s_temp=0, e_temp=0, b_temp=0, tflag_temp=0;
    int opt;
    FILE *file_temp = NULL;
    while ((opt = getopt(argc, argv, "hvs:E:b:t:")) != -1)
    {
        switch(opt)
        {
            case 'h':
            default:
                printf("-h: Optional help flag that prints usage info\n");
                printf("-v: Optional verbose flag that displays trace info\n");
                printf("-s <s>: Number of set index bits (S = 2s is the number of set)\n");
                printf("-E <E>: Associativity (number of lines per set)\n");
                printf("-b <b>: Number of block bits (B = 2b is the block size)\n");
                printf("-t <tracefile>: Name of the valgrind trace to replay");
                break;
            case 'v':
                vflag_temp = 1;
                break;
            case 's':
                s_temp = atoi(optarg);
                break;
            case 'E':
                e_temp = atoi(optarg);
                break;
            case 'b':
                b_temp = atoi(optarg);
                break;
            case 't':
                file_temp = fopen(optarg, "r");
                break;
        }
    }
    if(tflag_temp && s_temp && e_temp && b_temp && file_temp)
    {
        *vflag = vflag_temp;
        *set_index_bit = 1 << (s_temp-1);
        *lines_per_set = e_temp;
        *block_bit = b_temp;
        *tracefile = file_temp;
        return 1;
    }
    else
    {
        return 0;
    }
}
