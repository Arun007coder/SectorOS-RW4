#ifndef __PROCESS_H__
#define __PROCESS_H__

#include "system.h"
#include "paging.h"
#include "kheap.h"
#include "string.h"
#include "printf.h"
#include "pit.h"
#include "list.h"
#include "tree.h"
#include "math.h"
#include "elf_loader.h"
#include "isr.h"
#include "pic.h"
#include "keyboard.h"
#include "filedescriptor.h"

#define TASK_RUNNING            0
#define TASK_INTERRUPTIBLE      1
#define TASK_UNINTERRUPTIBLE    2
#define TASK_ZOMBIE             4
#define TASK_STOPPED            8
#define TASK_SWAPPING           16
#define TASK_EXCLUSIVE          32
#define TASK_CREATED            64
#define TASK_LOADING            128

#define TASK_TYPE_USER   0
#define TASK_TYPE_KERNEL 1

#define USER_ID_ROOT 0

/*
* Process manager
* Uses cooperative multitasking.
*/

typedef uint32_t pid_t;

typedef struct context
{
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t eflags;
    uint32_t cr3;
    uint32_t eip;
}__attribute__((packed)) context_t;

typedef void (*process_kbhandler_t)(char, int, int, uint8_t);

typedef struct pargs_struct
{
    char** argv;
    int argc;
}pargs_t;

typedef struct process_control_block
{
    char filename[512];
    context_t regs;
    pid_t pid;
    listnode_t * self;
    void * stack;
    uint32_t state;
    uint32_t time_slice;
    page_directory_t * page_dir;

    uint32_t type;
    uint32_t entrypoint;

    pargs_t args;
    fd_t fds[FD_MAX];
    uint32_t fd_num;

    uint32_t page_dir_addr;

    uint32_t ticks_since_boot;

    uint32_t umask;
    uint32_t uid;

    int execve_return;

    char* cwd;
    int cwdlen;

    process_kbhandler_t handler;

    // Only function symbols
    symbol_t* symtab;
    uint32_t n_syms;
    int isSYMTAB;
}pcb_t;

extern list_t* process_list;
extern pcb_t* current_process;
extern registers_t saved_context;
extern char p__cwd[512];

void init_processManager();

void change_process(pid_t pid);
void exit(uint32_t ret);

int kill(pid_t pid, uint32_t sig);

pid_t alloc_pid();
void create_process_from_routine(char* name, void* entrypoint, uint32_t type);
void create_process(char* filename);

void getcwd(char* buf, uint32_t sz);
int chdir(char* path);

// ENV does not work
void execve(char* name, char** argv, char** env);

uint32_t process_get_ticks();

pargs_t* get_args();

void list_process();
pid_t getpid();

void user_regs_switch(context_t *regs2);
void kernel_regs_switch(context_t * regs2);

void process_kbh(uint8_t scancode);
void attach_handler(process_kbhandler_t handler);

uint32_t getuid();
int setuid(uint32_t new_uid);

#endif /*__PROCESS_H__*/