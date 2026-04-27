#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TOTAL_MEM     2560
#define OS_SIZE        400
#define MAX_BLOCKS      20
#define MAX_PROCESSES   10
#define MAX_WAITING     10

typedef struct {
    int  start;
    int  size;
    int  is_free;
    char process[16];
} Block;

typedef struct {
    char name[16];
    int  size;
    int  time;
    int  in_memory;
    int  finished;
    int  start_addr;
} Process;

Block   blocks[MAX_BLOCKS];
int     block_count = 0;
Process procs[MAX_PROCESSES];
int     proc_count  = 0;
char    waiting[MAX_WAITING][16];
int     wait_count  = 0;

void print_memory_map(void) {
    printf("\n  +--------+----------+--------+------------+\n");
    printf("  | Эхлэл  | Хэмжээ   | Төлөв  | Процесс    |\n");
    printf("  +--------+----------+--------+------------+\n");
    for (int i = 0; i < block_count; i++) {
        printf("  | %4dK  | %6dK   | %-6s | %-10s |\n",
               blocks[i].start, blocks[i].size,
               blocks[i].is_free ? "Sul" : "Duureg",
               blocks[i].is_free ? "---" : blocks[i].process);
    }
    printf("  +--------+----------+--------+------------+\n");
    printf("  [");
    for (int i = 0; i < block_count; i++) {
        if (strcmp(blocks[i].process, "OS") == 0)
            printf("OS(%dK)", blocks[i].size);
        else if (blocks[i].is_free)
            printf("Sul(%dK)", blocks[i].size);
        else
            printf("%s(%dK)", blocks[i].process, blocks[i].size);
        if (i < block_count - 1) printf("|");
    }
    printf("]\n");
    if (wait_count > 0) {
        printf("  Huleelt: ");
        for (int i = 0; i < wait_count; i++) printf("%s ", waiting[i]);
        printf("\n");
    }
    int total_free = 0;
    for (int i = 0; i < block_count; i++)
        if (blocks[i].is_free) total_free += blocks[i].size;
    printf("  Niit sul zai: %dK\n", total_free);
}

int find_process(const char *name) {
    for (int i = 0; i < proc_count; i++)
        if (strcmp(procs[i].name, name) == 0) return i;
    return -1;
}

void merge_free_blocks(void) {
    int changed = 1;
    while (changed) {
        changed = 0;
        for (int i = 0; i < block_count - 1; i++) {
            if (blocks[i].is_free && blocks[i+1].is_free) {
                blocks[i].size += blocks[i+1].size;
                for (int j = i+1; j < block_count-1; j++)
                    blocks[j] = blocks[j+1];
                block_count--;
                changed = 1;
                break;
            }
        }
    }
}

int allocate_process(const char *name, int size) {
    for (int i = 0; i < block_count; i++) {
        if (blocks[i].is_free && blocks[i].size >= size) {
            int remainder = blocks[i].size - size;
            int start     = blocks[i].start;
            blocks[i].is_free = 0;
            blocks[i].size    = size;
            strncpy(blocks[i].process, name, sizeof(blocks[i].process)-1);
            if (remainder > 0) {
                for (int j = block_count; j > i+1; j--)
                    blocks[j] = blocks[j-1];
                block_count++;
                blocks[i+1].start   = start + size;
                blocks[i+1].size    = remainder;
                blocks[i+1].is_free = 1;
                strcpy(blocks[i+1].process, "");
            }
            int pi = find_process(name);
            if (pi >= 0) { procs[pi].in_memory = 1; procs[pi].start_addr = start; }
            return 1;
        }
    }
    return 0;
}

void release_process(const char *name) {
    for (int i = 0; i < block_count; i++) {
        if (!blocks[i].is_free && strcmp(blocks[i].process, name) == 0) {
            blocks[i].is_free = 1;
            strcpy(blocks[i].process, "");
            break;
        }
    }
    merge_free_blocks();
    int pi = find_process(name);
    if (pi >= 0) { procs[pi].in_memory = 0; procs[pi].finished = 1; }
}

void try_load_waiting(void) {
    int i = 0;
    while (i < wait_count) {
        int pi = find_process(waiting[i]);
        if (pi >= 0 && allocate_process(waiting[i], procs[pi].size)) {
            printf("  >> %s huuleeltes sanah oid orloo (%dK)\n", waiting[i], procs[pi].size);
            for (int j = i; j < wait_count-1; j++) strcpy(waiting[j], waiting[j+1]);
            wait_count--;
        } else { i++; }
    }
}

void print_step(const char *title) {
    printf("\n============================================================\n");
    printf("  %s\n", title);
    printf("============================================================");
    print_memory_map();
}

void extra_problem(void) {
    int blks[] = {100, 500, 200, 300, 600};
    int ps[]   = {212, 417, 112, 426};
    char *pn[] = {"P1","P2","P3","P4"};
    int nb=5, np=4;

    printf("\n============================================================\n");
    printf("  NEMELT BODLOGO: First Fit / Best Fit / Worst Fit\n");
    printf("  Blok: B1=100K B2=500K B3=200K B4=300K B5=600K\n");
    printf("  Process: P1=212K P2=417K P3=112K P4=426K\n");
    printf("============================================================\n");

    printf("\n-- First Fit --\n");
    int ff[5]; memcpy(ff,blks,sizeof(blks));
    for(int i=0;i<np;i++){
        int ok=0;
        for(int j=0;j<nb;j++) if(ff[j]>=ps[i]){
            printf("  %s(%dK) -> B%d(%dK) uldegdel=%dK\n",pn[i],ps[i],j+1,blks[j],ff[j]-ps[i]);
            ff[j]-=ps[i]; ok=1; break;
        }
        if(!ok) printf("  %s(%dK) -> bayrlahuugui\n",pn[i],ps[i]);
    }
    int t=0; for(int j=0;j<nb;j++) t+=ff[j];
    printf("  Niit uldegdel: %dK\n",t);

    printf("\n-- Best Fit --\n");
    int bf[5]; memcpy(bf,blks,sizeof(blks));
    for(int i=0;i<np;i++){
        int best=-1,bd=99999;
        for(int j=0;j<nb;j++) if(bf[j]>=ps[i]&&bf[j]-ps[i]<bd){bd=bf[j]-ps[i];best=j;}
        if(best>=0){
            printf("  %s(%dK) -> B%d(%dK) uldegdel=%dK\n",pn[i],ps[i],best+1,blks[best],bd);
            bf[best]-=ps[i];
        } else printf("  %s(%dK) -> bayrlahuugui\n",pn[i],ps[i]);
    }
    t=0; for(int j=0;j<nb;j++) t+=bf[j];
    printf("  Niit uldegdel: %dK\n",t);

    printf("\n-- Worst Fit --\n");
    int wf[5]; memcpy(wf,blks,sizeof(blks));
    for(int i=0;i<np;i++){
        int worst=-1,ws=-1;
        for(int j=0;j<nb;j++) if(wf[j]>=ps[i]&&wf[j]>ws){ws=wf[j];worst=j;}
        if(worst>=0){
            printf("  %s(%dK) -> B%d(%dK) uldegdel=%dK\n",pn[i],ps[i],worst+1,blks[worst],wf[worst]-ps[i]);
            wf[worst]-=ps[i];
        } else printf("  %s(%dK) -> bayrlahuugui\n",pn[i],ps[i]);
    }
    t=0; for(int j=0;j<nb;j++) t+=wf[j];
    printf("  Niit uldegdel: %dK\n",t);
}

int main(void) {
    printf("============================================================\n");
    printf("  UILDLIIN SISTEM - LABORATORI 5\n");
    printf("  Sanah oin zokhion baiguulalt ba dinamik huviarilalt\n");
    printf("  First Fit algoritm | Niit sanah oi: 2560K\n");
    printf("============================================================\n");

    block_count = 0;
    blocks[block_count].start=0; blocks[block_count].size=OS_SIZE;
    blocks[block_count].is_free=0; strcpy(blocks[block_count].process,"OS"); block_count++;
    blocks[block_count].start=OS_SIZE; blocks[block_count].size=TOTAL_MEM-OS_SIZE;
    blocks[block_count].is_free=1; strcpy(blocks[block_count].process,""); block_count++;

    char *names[]={"P1","P2","P3","P4","P5"};
    int sizes[]={600,1000,300,700,500};
    int times[]={10,5,20,8,15};
    proc_count=5;
    for(int i=0;i<proc_count;i++){
        strcpy(procs[i].name,names[i]);
        procs[i].size=sizes[i]; procs[i].time=times[i];
        procs[i].in_memory=0; procs[i].finished=0; procs[i].start_addr=-1;
    }

    for(int i=0;i<3;i++)
        if(!allocate_process(procs[i].name,procs[i].size))
            strcpy(waiting[wait_count++],procs[i].name);
    strcpy(waiting[wait_count++],"P4");
    strcpy(waiting[wait_count++],"P5");

    print_step("t=0: Ekhleliin tolov (P1,P2,P3 achaalsan - P4,P5 huleeltend)");

    printf("\n>>> P2 duusav (time=5)\n");
    release_process("P2");
    try_load_waiting();
    print_step("P2 duussanii daraa");

    print_step("P4 sanah oid orsnoii daraa");

    printf("\n>>> P1 duusav (time=10)\n");
    release_process("P1");
    try_load_waiting();
    print_step("P1 duussanii daraa");

    print_step("P5 sanah oid orsnoii daraa");

    printf("\n>>> P4 duusav\n");
    release_process("P4");
    try_load_waiting();
    print_step("P4 duussanii daraa");

    printf("\n>>> P5 duusav\n"); release_process("P5");
    printf("\n>>> P3 duusav\n"); release_process("P3");
    print_step("Etsiin tolov: Buh process duusav");

    extra_problem();

    printf("\n============================================================\n");
    printf("  DUGNELT\n");
    printf("  * First Fit: hurdan, external fragmentation uusne\n");
    printf("  * Best Fit: hamgiin baga uldegdel (P4-iig bayrluulj chadsan)\n");
    printf("  * Worst Fit: tom blokiig taraadag\n");
    printf("============================================================\n");
    return 0;
}
