//Kang Minsu 20170225(student ID) powerful(POVIS ID)

#include "cachelab.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

typedef struct {
    int valid;
    int tag;
    unsigned long used_timing;
} line;

typedef struct {
    line* lines;
} set;

typedef struct {
    int set_num;
    int line_num;
    int block_bytes;
    unsigned long instruction_count;
    set* sets;
} cache;

int s, b;
int hit_count, miss_count, eviction_count;

cache* allocate_cache(int set_num, int line_num, int block_bytes);
void deallocate_cache(cache* c);
int find_cache(cache* c, int address);

int parse_block_offset(unsigned long address);
int parse_set_index(unsigned long address);
int parse_tag(unsigned long address);

int parse_argument(int argc, char *argv[], int *vflag, int *set_bit, int *lines_per_set, int *block_bit, FILE **tracefile);
int parse_memory_trace(const char *str,char *instruction, unsigned long *address, int *size);

int match_line_address(line l, unsigned long address);
void fetch(cache* c, set* current_set, unsigned long address);
void cache_find_iteration(cache *c, char* str, int verbose_flag);

int main(int argc, char *argv[])
{
    int verbose_flag, line_num;
    int set_num,block_num;
    FILE *f;
    char buffer[100];
    cache *c;

    if(parse_argument(argc, argv, &verbose_flag, &s, &line_num, &b, &f))
    {
        set_num = 1 << s;
        block_num = 1 << b;
        //printf("v:%d, s:%d, E:%d, b:%d\n",verbose_flag, set_num, line_num, block_num);
        c = allocate_cache(set_num, line_num, block_num);
        while(fgets(buffer, 100, f))
        {
            cache_find_iteration(c, buffer, verbose_flag);
        }
        printSummary(hit_count, miss_count, eviction_count);
        deallocate_cache(c);
        return 0;
    }
    else
    {
        return -1;
    }
}

int parse_memory_trace(const char *str,char *instruction, unsigned long *address, int *size)
{
    int size_temp;
    unsigned long address_temp;
    char buff[5];
    if(str[0] == 'I')
    {
        return 0;
    }
    else
    {
        sscanf(str,"%s%lx,%d",buff,&address_temp, &size_temp);
        *instruction = buff[0];
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
            default:
                break;
            case 'h':
                printf("Usage: %s [-hv] -s <num> -E <num> -b <num> -t <file>\n", argv[0]);
                printf("Options:\n");
                printf(" -h         Print this help message.\n");
                printf(" -v         Optional verbose flag.\n");
                printf(" -s <num>   Number of set index bits.\n");
                printf(" -E <num>   Number of lines per set.\n");
                printf(" -b <num>   Number of block offset bits.\n");
                printf(" -t <file>  Trace file.\n\n");
                printf("Examples:\n");
                printf("  linux>  %s -s 4 -E 1 -b 4 -t traces/yi.trace\n", argv[0]);
                printf("  linux>  %s -v -s 8 -E 2 -b 4 -t traces/yi.trace\n", argv[0]);
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
        printf(" %s: Missing required command line argument\n", argv[0]);
        printf("Usage: %s [-hv] -s <num> -E <num> -b <num> -t <file>\n", argv[0]);
        printf("Options:\n");
        printf(" -h         Print this help message.\n");
        printf(" -v         Optional verbose flag.\n");
        printf(" -s <num>   Number of set index bits.\n");
        printf(" -E <num>   Number of lines per set.\n");
        printf(" -b <num>   Number of block offset bits.\n");
        printf(" -t <file>  Trace file.\n\n");
        printf("Examples:\n");
        printf("  linux>  %s -s 4 -E 1 -b 4 -t traces/yi.trace\n", argv[0]);
        printf("  linux>  %s -v -s 8 -E 2 -b 4 -t traces/yi.trace\n", argv[0]);
        return 0;
    }
}

int parse_block_offset(unsigned long address)
{
    return address & ((0x1 << b) - 1); 
}

int parse_set_index(unsigned long address)
{
    address >>= b;
    return address & ((0x1 << s) - 1);
}

int parse_tag(unsigned long address)
{
    unsigned long temp = address; 
    return (int)((unsigned long)temp >> (s + b));
}

cache* allocate_cache(int set_num, int line_num, int block_bytes)
{
    cache *temp; 
    int i, j;

    temp = (cache*)malloc(sizeof(cache));
    temp->set_num = set_num;
    temp->line_num = line_num;
    temp->block_bytes = block_bytes;
    temp->sets = (set*)malloc(sizeof(set) * temp->set_num);
    temp->instruction_count = 0;
    for(i = 0; i < set_num; i++)
    {
        temp->sets[i].lines = (line*)malloc(sizeof(line) * temp->line_num); 
        for(j=0; j<temp->line_num; j++)
        {
            temp->sets[i].lines[j].valid = 0;
            temp->sets[i].lines[j].tag = 0;
            temp->sets[i].lines[j].used_timing = 0;
        }
    }
    return temp;
}

void deallocate_cache(cache* c)
{
    int i;
    for(i=0;i<c->set_num; i++){
        free(c->sets[i].lines);
    }
    free(c);
}

int match_line_address(line l, unsigned long address)
{
    return (parse_tag(address) == l.tag) && l.valid; 
}

void cache_find_iteration(cache *c, char* str, int verbose_flag)
{
    int i;
    set* current_set;
    unsigned long min_timing_val;
    int oldest_index;
    int is_miss= 0, is_hit = 0, is_eviction;
    unsigned long address;
    char instruction;
    int size;

    //find match

    if(parse_memory_trace(str,&instruction, &address, &size))
    {
        current_set = &(c->sets[parse_set_index(address)]);
        for(i=0; i<c->line_num;i++)
        {
            if(match_line_address(current_set->lines[i], address))
            {
                //hit
                current_set->lines[i].used_timing = c->instruction_count;
                is_hit = 1;
                break;
            }
        }

        //if no match
        if(!is_hit)
        {
            //find empty line
            for(i = 0;i<c->line_num; i++)
            {
                if(!current_set->lines[i].valid)
                {
                    current_set->lines[i].tag = parse_tag(address);
                    current_set->lines[i].used_timing = c->instruction_count;
                    current_set->lines[i].valid = 1;
                    is_miss = 1;
                    break;
                }
            }
        }

        //if line is full
        if(!(is_miss | is_hit))
        {
            //find LRU
            min_timing_val = c->instruction_count;
            for(i=0; i<c->line_num;i++)
            {
                if(current_set->lines[i].used_timing < min_timing_val)
                {
                    min_timing_val = current_set->lines[i].used_timing;
                    oldest_index = i;
                }
            }
            current_set->lines[oldest_index].tag = parse_tag(address);
            current_set->lines[oldest_index].used_timing = c->instruction_count;
            is_eviction = 1;
        }

        if(verbose_flag)
        {
            printf("%c %lx, %d ", instruction, address, size);
        }
        //increse count and print
        if(is_hit)
        {
            if(instruction == 'M')
            {
                if(verbose_flag)
                    printf("hit hit\n");
                hit_count += 2;
            }
            else
            {
                if(verbose_flag)
                    printf("hit\n");
                hit_count +=1;
            }
        }
        if(is_miss)
        {
            if(instruction == 'M')
            {
                if(verbose_flag)
                    printf("miss hit\n");
                miss_count+=1;
                hit_count+=1;
            }
            else
            {
                if(verbose_flag)
                    printf("miss\n");
                miss_count+=1;
            }
        }
        if(is_eviction)
        {
            if(instruction == 'M')
            {
                if(verbose_flag)
                    printf("miss eviction hit\n");
                miss_count += 1;
                eviction_count += 1;
                hit_count += 1;
            }
            else
            {
                if(verbose_flag)
                    printf("miss eviction\n");
                miss_count += 1;
                eviction_count+= 1;
            }
        }
        c->instruction_count++;
    }
}

