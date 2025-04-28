#include<stdio.h>
#include<string.h>
#include<stdlib.h>

typedef unsigned int page_table_entry;

char filename[20] = "trace.in";
int num_frames;
char strategy[10];
int rand_seed = 5635;
int verbose = 0;
int strat = 0;

page_table_entry *table;
int my_clock;
int fifo_num = 0;
int clock_ptr = 0;
FILE *fileptr;

struct node{
    int time;
    struct node *next;
};
struct node **page_data;
int max_time = 0;

unsigned int num_access=0, num_writes=0, num_misses=0, num_drops=0;
// page_table

// data struct to store page_num of each frame
int frame_to_page[1000];

// data struct to store time of last use of frame
int last_used_when[1000];

void func();
int is_present(page_table_entry pte);
int is_dirty(page_table_entry pte);
int is_used(page_table_entry pte);

int policy_replace();
int opt();
int fifo();
int lru();
int clock();
int random();

void print_result();
void preprocess();

int main(int argc, char *argv[]){

    strcpy(filename, argv[1]);
    char *num_frames_str = argv[2];
    char *end;
    num_frames = strtol(num_frames_str, &end, 10);
    strcpy(strategy, argv[3]);


    if(argc == 5) verbose = 1;

    if(strcmp(strategy, "OPT") == 0) strat = 0;
    else if(strcmp(strategy, "FIFO") == 0) strat = 1;
    else if(strcmp(strategy, "CLOCK") == 0) strat = 2;
    else if(strcmp(strategy, "LRU") == 0) strat = 3;
    else if(strcmp(strategy, "RANDOM") == 0) strat = 4;

    if (strat == 0) preprocess();

    srand(rand_seed);
    fifo_num = 0;
    fileptr = fopen(filename, "r");

    func();

    fclose(fileptr);

    print_result();

    return 0;
}

void func(){
    unsigned int addr;
    char op;

    page_table_entry *page_table = malloc(sizeof(page_table_entry) * 0x100000);
    memset(page_table, 0, 0x100000);
    table = page_table;

    memset(last_used_when, 0, num_frames+1);

    int num_filled_frames = 0;
    my_clock = 0;

    while (fscanf(fileptr, "%x %c", &addr, &op) != EOF){
        // do stuff

        num_access++;

        const unsigned int page_num = (addr & 0xfffff000) >> 12;
        page_table_entry pte = page_table[page_num];
        int frame_num = 0;
        
        // make sure page is in memory
        if(is_present(pte) == 1){
            // page is present in memory, do nothing

            // DO NOTHING
            frame_num = (pte & 0xfffffff0)>>4;
        }
        else{
            // page is absent
            num_misses++;

            // check if free frame is available
            if(num_filled_frames < num_frames){
                // free frame available at index num_filled_frames
                // allocate it to curr page

                // ALLOCATE FRAME TO PAGE
                frame_num = num_filled_frames;
                pte = (page_table_entry) (frame_num << 4);
                frame_to_page[frame_num] = page_num;
                num_filled_frames++;
            }
            else{
                // no frame is free
                // search for frame to free by policy
                int old_page_frame_num = policy_replace();
                int old_page_num = frame_to_page[old_page_frame_num];
                page_table_entry pte_old = page_table[old_page_num];

                // check if old page is dirty
                if(is_dirty(pte_old) == 1){
                    // write to disk
                    num_writes++;
                    if(verbose){
                        printf("Page 0x%x was read from disk, page 0x%x was written to the disk.\n", page_num, old_page_num);
                    }
                }else{
                    num_drops++;
                    if(verbose){
                        printf("Page 0x%x was read from disk, page 0x%x was dropped (it was not dirty).\n", page_num, old_page_num);
                    }
                }

                // DROP OLD PAGE AND ALLOCATE
                pte_old = (page_table_entry) (pte_old & 0xfffffffd);
                page_table[old_page_num] = pte_old;

                frame_num = old_page_frame_num;

                pte = (page_table_entry) (frame_num << 4);
                frame_to_page[frame_num] = page_num;
            }
        }


        // perform operation
        if(op == 'W') pte = (page_table_entry) (pte | 0b1); 
        pte = (page_table_entry) (pte | 0b110);
        page_table[page_num] = pte;
        last_used_when[frame_num] = ++my_clock;
    }
}

int is_present(page_table_entry pte){
    return (pte & 0x2)>>1;
}

int is_dirty(page_table_entry pte){
    return (pte & 0b1);
}

int is_used(page_table_entry pte){
    return (pte & 0b100) >> 2;
}

int policy_replace(){
    int page_num_to_replace;

    switch (strat)
    {
    case 0:
        page_num_to_replace = opt();
        break;
    case 1:
        page_num_to_replace = fifo();
        break;
    case 2:
        page_num_to_replace = clock();
        break;
    case 3:
        page_num_to_replace = lru();
        break;
    case 4:
        page_num_to_replace = random();
        break;
    default:
        break;
    }

    return page_num_to_replace;
    // return 1;
}

int random()
{
    return (rand() % num_frames);
}

int fifo()
{
    return (fifo_num++)%num_frames;
}

int lru()
{
    int return_frame_num = 0;
    int min_time = my_clock + 1;
    for(int i = 0; i < num_frames; i++){
        if( last_used_when[i] <= min_time ){
            return_frame_num = i;
            min_time = last_used_when[i];
        }
    }
    return return_frame_num;
    // return 1;
}

int clock()
{
    page_table_entry *page_table = table;
    clock_ptr %= num_frames;
    while(1){
        int page_num = frame_to_page[clock_ptr];
        page_table_entry pte = page_table[page_num];
        if(is_used(pte)){
            pte = (page_table_entry) (pte & 0xfffffffb);
            page_table[page_num] = pte;
            clock_ptr = (clock_ptr + 1) % num_frames;
        }
        else{
            return clock_ptr++;
            // return clock_ptr-1;
        }
    }

    return 1;
}

int opt()
{
    int return_frame_num = 0;
    int far_time = my_clock;
    for(int fno=0; fno<num_frames; fno++){
        int page_num = frame_to_page[fno];
        struct node *no = page_data[page_num];

        while (no->next != NULL && no->next->time <= my_clock){
            no->next = no->next->next;
        }

        if(no -> next == NULL){
            return fno;
        }
        else if((no -> next -> time) > far_time){
            far_time = no->next->time;
            return_frame_num = fno;
        }
    }
    
    return return_frame_num;
}

void print_result(){
    printf("Number of memory accessess: %d\n", num_access);
    printf("Number of misses: %d\n", num_misses);
    printf("Number of writes: %d\n", num_writes);
    printf("Number of drops: %d\n", num_drops);
}

void preprocess(){
    struct node **data = (struct node**) malloc(sizeof(struct node) * 0x100000);
    struct node **tails = (struct node**) malloc(sizeof(struct node) * 0x100000);
    for(int i = 0; i <= 0xfffff; i++){
        struct node *n = (struct node*) malloc(sizeof(struct node));
        n->time = -1;
        n->next = NULL;
        data[i] = n;
        tails[i] = data[i];
    }

    int timer = 0;
    FILE *fptr = fopen(filename, "r");

    unsigned int addr;
    char op;

    while (fscanf(fptr, "%x %c", &addr, &op) != EOF){
        int page_num = (addr & 0xfffff000)>>12;
        struct node *no = (struct node*) malloc(sizeof(struct node));
        no -> time = ++timer;
        no -> next = NULL;
        tails[page_num] -> next = no;
        tails[page_num] = tails[page_num]->next;
    }

    max_time = timer;
    fclose(fptr);
    page_data = data;
}
