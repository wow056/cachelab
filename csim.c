//Kang Minsu 20170225(student ID) powerful(POVIS ID)

#include "cachelab.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

typedef struct {
    int valid;
    int tag;
} line;

typedef struct {
    line* lines;
} set;

typedef struct {
    int set_num;
    int line_num;
    int block_bytes;
    set* sets;
} cache;

int parse_block_offset(long address, int b);
int parse_set_index(long address, int s, int b);
int parse_tag(long address, int s, int b);

int parse_argument(int argc, char *argv[], int *vflag, int *set_bit, int *lines_per_set, int *block_bit, FILE **tracefile);
int parse_memory_trace(const char *str, long *address, int *size);

int main(int argc, char *argv[])
{
    int verbose_flag, set_bit, line_num, block_bit_num;
    int set_num,block_num;
    int memsize;
    long address;
    FILE *f;
    char buffer[100];
    int block_offset, set_index, tag;

    if(parse_argument(argc, argv, &verbose_flag, &set_bit, &line_num, &block_bit_num, &f))
    {
        set_num = 1 << set_bit;
        block_num = 1 << block_bit_num;
        printf("v:%d, s:%d, E:%d, b:%d\n",verbose_flag, set_num, line_num, block_num);
        while(fgets(buffer, 100, f))
        {
            if(parse_memory_trace(buffer, &address, &memsize))
            {
                printf("%s",buffer);
                printf("address: 0x%lx, memsize: %d\n", address, memsize);
                tag = parse_tag(address, set_bit, block_bit_num);
                set_index = parse_set_index(address, set_bit, block_bit_num);
                block_offset = parse_block_offset(address, set_bit);
                printf("tag: 0x%x, set_index: 0x%x, block_offset: 0x%x\n\n", tag, set_index, block_offset);
            }
        }
        printSummary(0, 0, 0);
        return 0;
    }
    else
    {
        return -1;
    }
}

int parse_memory_trace(const char *str,long *address, int *size)
{
    int size_temp;
    long address_temp;
    if(str[0] == 'I')
    {
        return 0;
    }
    else
    {
        sscanf(str,"%*s%lx,%d",&address_temp, &size_temp);
        *address = address_temp;
        *size = size_temp;
        return 1;
    }
}

int parse_argument(int argc, char *argv[], int *vflag, int *set_bit, int *lines_per_set, int *block_bit, FILE **tracefile)
{
    int vflag_temp=0, s_temp=0, e_temp=0, b_temp=0;
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
    if(s_temp && e_temp && b_temp && file_temp)
    {
        *vflag = vflag_temp;
        *set_bit = s_temp;
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

int parse_block_offset(long address, int b)
{
    return address & ((0x1 << b) - 1); 
}

int parse_set_index(long address, int s, int b)
{
    address >>= b;
    return address & ((0x1 << s) - 1);
}
int parse_tag(long address, int s, int b)
{
    unsigned long temp = address; 
    return (int)((unsigned long)temp >> (s + b));
}


