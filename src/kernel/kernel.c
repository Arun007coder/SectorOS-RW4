#include "system.h"
#include "serial.h"
#include "vga_text.h"
#include "printf.h"
#include "gdt.h"
#include "idt.h"
#include "pit.h"
#include "paging.h"
#include "fat32.h"
#include "kheap.h"
#include "pmm.h"
#include "cpuinfo.h"
#include "keyboard.h"
#include "sorfs.h"
#include "shell.h"
#include "atapio.h"
#include "ramdisk.h"
#include "kernelfs.h"
#include "pci.h"
#include "vfs.h"
#include "devfs.h"
#include "mount.h"
#include "tss.h"
#include "usermode.h"
#include "syscall.h"
#include "process.h"
#include "stdout.h"
#include "sse.h"
#include "multiboot_vesa.h"
#include "ifb.h"
#include "compositor.h"
#include "timer.h"
#include "tmpfs.h"
#include "portdev.h"
#include "nulldev.h"
#include "mount.h"
#include "logdisk.h"

#include "rtl8139.h"
#include "pcnet.h"
#include "arp.h"
#include "ipv4.h"
#include "udp.h"
#include "tcp.h"
#include "dhcp.h"

#include "modules.h"
#include "multiboot2_tags.h"

#include "mouse.h"


#define MOTD_NUM 3

void print_motd(int idx)
{
    switch(idx)
    {
    case 1:
        printf("   _____           _              ____   _____       _______          ___  _   \n  / ____|         | |            / __ \\ / ____|     |  __ \\ \\        / / || |  \n | (___   ___  ___| |_ ___  _ __| |  | | (___ ______| |__) \\ \\  /\\  / /| || |_ \n  \\___ \\ / _ \\/ __| __/ _ \\| '__| |  | |\\___ \\______|  _  / \\ \\/  \\/ / |__   _|\n  ____) |  __/ (__| || (_) | |  | |__| |____) |     | | \\ \\  \\  /\\  /     | |  \n |_____/ \\___|\\___|\\__\\___/|_|   \\____/|_____/      |_|  \\_\\  \\/  \\/      |_|  \n                                                                               \n                                                                               \n");
        break;
    case 2:
        printf("   _____           __             ____ _____       ____ _       ____ __\n  / ___/___  _____/ /_____  _____/ __ / ___/      / __ | |     / / // /\n  \\__ \\/ _ \\/ ___/ __/ __ \\/ ___/ / / \\__ \\______/ /_/ | | /| / / // /_\n ___/ /  __/ /__/ /_/ /_/ / /  / /_/ ___/ /_____/ _, _/| |/ |/ /__  __/\n/____/\\___/\\___/\\__/\\____/_/   \\____/____/     /_/ |_| |__/|__/  /_/   \n                                                                       \n");
        break;
    case 3:
        printf("   _____           __             ____ _____       ____ _       ____ __\n  / ___/___  _____/ /_____  _____/ __ / ___/      / __ | |     / / // /\n  \\__ \\/ _ \\/ ___/ __/ __ \\/ ___/ / / \\__ \\______/ /_/ | | /| / / // /_\n ___/ /  __/ /__/ /_/ /_/ / /  / /_/ ___/ /_____/ _, _/| |/ |/ /__  __/\n/____/\\___/\\___/\\__/\\____/_/   \\____/____/     /_/ |_| |__/|__/  /_/   \n                                                                       \n");
        break;
    default:
        printf("SectorOS-RW4\n");
        break;
    };
}

void usp2()
{
    SYSCALL_CLEAR
    SYSCALL_PUTC('W');
    SYSCALL_EXIT(0)
}

void usp()
{
    SYSCALL_CLEAR
    SYSCALL_PUTC('H');
    while(1);
}


void rand_motd()
{
    int r = rand_range(0, MOTD_NUM);
    print_motd(r);
}

char* mount_filesystems = "nodev	tmpfs\n        sorfs\n";
char* compiler = KERNEL_COMPILER;

list_t* mboot_cmd;

int no_process = 0;

void sample_callback(uint8_t* addr, uint8_t* data, uint32_t len)
{
    ldprintf("kernel", LOG_DEBUG, "DATA FROM %d.%d.%d.%d:\n\t%s\n", addr[0], addr[1], addr[2], addr[3], data);
}

int tcp_sample(TCPSocket_t* self, uint8_t* data, uint16_t len)
{
    uint8_t addr[8];
    iptoa(self->remIP, addr);
    ldprintf("kernel", LOG_DEBUG, "DATA FROM %d.%d.%d.%d(0x%x):\n\t%s\n", addr[0], addr[1], addr[2], addr[3], self->remIP, data);
    
    if (len > 9 && data[0] == 'G' && data[1] == 'E' && data[2] == 'T' && data[3] == ' ' && data[4] == '/' && data[5] == ' ' && data[6] == 'H' && data[7] == 'T' && data[8] == 'T' && data[9] == 'P')
    {
        ldprintf("KERNEL", LOG_DEBUG, "GOT CONNECTION OVER HTTP");
        tcpsocket_send(self, (uint8_t *)"HTTP/1.1 200 OK\r\nServer: SectorOS\r\nContent-Type: text/html\r\n\r\n<html><head><title>SectorOS</title></head><body><b>This is a test webpage which is hosted on SectorOS</b> https://github.com/Arun007coder/SectorOS</body></html>\r\n", 224);
        tcp_disconnect(self);
    }
    return 1;
}

const char* mmap_get_type_str(uint32_t type)
{
    switch(type)
    {
    case MULTIBOOT_MEMORY_ACPI_RECLAIMABLE:
        return "ACPI reclaimable memory (3)";
        break;
    case MULTIBOOT_MEMORY_AVAILABLE:
        return "Free memory (1)";
        break;
    case MULTIBOOT_MEMORY_RESERVED:
        return "Reserved memory (2)";
        break;
    case MULTIBOOT_MEMORY_NVS:
        return "ACPI non volatile memory (4)";
        break;
    case MULTIBOOT_MEMORY_BADRAM:
        return "Bad memory region (5)";
        break;
    default:
        return "Reserved memory (2)";
        break;
    };
}

char cmdline[2048];
char bootloader_name[1024];

uint32_t mem_upper = 0;
uint32_t mem_lower = 0;

char* version_file = KERNEL_NAME" "KERNEL_VERSION" "KERNEL_VERSION_CODENAME" (Enabled options: "KERNEL_ENABLED_OPTIONS") "KERNEL_BUILD_DATE" "KERNEL_BUILD_TIME;

uint8_t* mtags = NULL;

void kernelmain(const void* info, uint32_t multiboot_magic)
{
    init_serial(COM1, 1);
    if(multiboot_magic != MULTIBOOT2_BOOTLOADER_MAGIC)
    {
        serial_puts("Invalid magic\n");
        return;
    }

    init_text();
    text_chcolor(VGA_WHITE, VGA_LIGHT_BLUE);
    text_clear();

    init_gdt();
    init_idt();

    int logidx;

    init_pmm(1024*MB);
    init_paging();

    struct multiboot_tag* tag = info;
    uint32_t tsz = *((uint32_t*)info);
    alloc_region(kernel_page_dir, info, info+tsz, 1, 1, 0);
    for(tag = (struct multiboot_tag *) ((size_t) (info + 8)); tag->type != MULTIBOOT_TAG_TYPE_END; tag = (struct multiboot_tag *) ((multiboot_uint8_t *) tag + ((tag->size + 7) & ~7)))
    {
        switch(tag->type)
        {
        case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO:
        {
            struct multiboot_tag_basic_meminfo* meminfo = tag;
            mem_lower = meminfo->mem_lower;
            mem_upper = meminfo->mem_upper;
            init_pmm(1024*mem_upper);
        }break;
        };
        break;
    }

    init_kheap(KHEAP_START, KHEAP_START + KHEAP_INITIAL_SIZE, KHEAP_MAX_ADDRESS);
    ASSERT(load_multiboot2_tags(info) == 0);

    init_logdisk(4*MB, LOG_VERBOSE);

    ldprintf("kernel", LOG_INFO, "Running SectorOS-RW4 kernel %s", KERNEL_VERSION);
    ldprintf("multiboot", LOG_INFO, "cmdline: %s", cmdline);
    ldprintf("gdt", LOG_INFO, "Added %d entries to GDT", GDT_MAX_ENTRIES);
    ldprintf("idt", LOG_INFO, "IDT has been initialized");
    ldprintf("pmm", LOG_INFO, "Memory found in system: %dKB", ((mem_upper)));
    ldprintf("pmm", LOG_INFO, "Physical memory manager has been initialized");
    ldprintf("paging", LOG_INFO, "Memory page manager has been initialized");
    ldprintf("kernel_heap", LOG_INFO, "Heap start: 0x%x, Heap end: 0x%x, Heap max address: 0x%x, Heap initial size: %dB", KHEAP_START, (KHEAP_START+KHEAP_INITIAL_SIZE), KHEAP_MAX_ADDRESS, KHEAP_INITIAL_SIZE);
    ldprintf("kernel_heap", LOG_INFO, "Kernel heap manager has been initialized");

    init_pic();

    init_vfs();
    init_devfs();
    init_kernelfs();

    init_mount();
    syscall_mount(NULL, "/proc", "kernelfs", 0, NULL);
    logdisk_mount();
    init_sorfs();
    init_tmpfs();
    init_ext2();

    init_timer_interface();
	init_pit();

    enable_sse();
    enable_fast_memcpy();

    tag = info;
    mtags = zalloc(tsz);
    memcpy(mtags, info, tsz);
    for(tag = (struct multiboot_tag *) ((size_t) (mtags+8)); tag->type != MULTIBOOT_TAG_TYPE_END; tag = (struct multiboot_tag *) ((multiboot_uint8_t *) tag + ((tag->size + 7) & ~7)))
    {
        switch(tag->type)
        {
        case MULTIBOOT_TAG_TYPE_CMDLINE:
        {
            struct multiboot_tag_string* str = tag;
            strncpy(cmdline, str->string, 2048);
        }break;
        case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO:
        {
            struct multiboot_tag_basic_meminfo* meminfo = tag;
            mem_lower = meminfo->mem_lower;
            mem_upper = meminfo->mem_upper;
        }break;
        case MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME:
        {
            struct multiboot_tag_string* bootloader = tag;
            strncpy(bootloader_name, bootloader->string, 1024);
        }break;
        case MULTIBOOT_TAG_TYPE_ELF_SECTIONS:
        {
            struct multiboot_tag_elf_sections* elfs = tag;
            alloc_region(kernel_page_dir, tag, tag+tag->size, 1, 1, 1);
            load_kernel_symbols(elfs);
        }break;
        case MULTIBOOT_TAG_TYPE_VBE:
        {
            if(isVesaInit())
                break;
            struct multiboot_tag_vbe* be = tag;
            init_ifb();
            init_vesa(be);

            init_compositor();

            init_seterm();
        }break;
        };
    }

    mboot_cmd = str_split(((char*)cmdline), " ", NULL);

    if((logidx = list_contain_str(mboot_cmd, "--loglevel")) != 0)
    {
        if(list_size(mboot_cmd) < (logidx+1))
        {
            ldprintf("kernel", LOG_WARN, "Argument %s does not have a value, therefore this command will be ignored.", list_get_node_by_index(mboot_cmd, logidx)->val);
        }
        else
        {
            listnode_t* l = list_get_node_by_index(mboot_cmd, logidx+1);
            uint32_t logstate = atoi(l->val);
            logdisk_change_policy(logstate);
        }
    }
    kernelfs_add_variable("/proc", "compiler", compiler, strlen(compiler));
    kernelfs_add_variable("/proc", "cmdline", cmdline, strlen(((char*)cmdline)));
    kernelfs_add_variable("/proc", "loader", bootloader_name, strlen(((char*)bootloader_name)));
    kernelfs_add_variable("/proc", "version", version_file, strlen(((char*)version_file)));

    syscall_mount(NULL, "/tmp", "tmpfs", 0, NULL);

    init_portDev();
    init_nulldev();

    init_syscall();

    int q = 0;
    if((q = list_contain_str(mboot_cmd, "--no_process")) != -1)
    {
        no_process = 1;
    }

    if(no_process == 0)
        init_tss(0x05, 0x10, 0);

    if(no_process == 0)
        init_processManager();

    init_keyboard();

    init_pci();

    init_ata_pio();
    init_stdout();

    uint32_t esp;
    ASM_FUNC("mov %%esp, %0" : "=r"(esp));
    if(no_process == 0)
        tss_set_kernel_stack(0x10, esp);

    ldprintf("multiboot", LOG_DEBUG, "\t-Bootloader: %s\n\t-CmdLine: %s\n\t-Available memory: %dMB\n", bootloader_name, cmdline, (mem_upper / 1024));

    ldprintf("KERNEL", LOG_INFO, "Kernel has successfully initialized");

    init_rand();

    //print_cpu_info();

    ldprintf("Rand", LOG_DEBUG, "%d %d %d %d %d %d %d %d %d %d\n", rand_range(0, 256), rand_range(0, 256), rand_range(0, 256), rand_range(0, 256), rand_range(0, 256), rand_range(0, 256), rand_range(0, 256), rand_range(0, 256), rand_range(0, 256), rand_range(0, 256));

    text_clear();

    rand_motd();
    printf("Version: %s\n", KERNEL_VERSION);
    printf("\n");

    int j = 0;

    if((j = list_contain_str(mboot_cmd, "--root")) != -1)
    {
        if(j > list_size(mboot_cmd))
        {
            ldprintf("Kernel", LOG_ERR, "Device path is not in arguments.");
        }
        else
        {
            char* root = (list_get_node_by_index(mboot_cmd, j+1)->val);
            mount(root, "/");
        }
    }

    enable_interrupts();

    printtime();

    compositor_load_wallpaper("/wallpaper.bmp", 2);
    init_vbox();
    init_shell();

    print_mountList();

    if(list_contain_str(mboot_cmd, "--network_enable_loopback") != -1)
    {
        init_networkInterfaceManager();
        init_loopback();
        init_rtl8139();
        //init_pcnet(); //! Does not work

        switch_interface(0);

        read_mac_addr();

        init_arp();
        init_udp();
        init_dhcp();

        init_tcp();

        TCPSocket_t* s = tcp_listen(9090);
        tcp_bind(s, tcp_sample);

        uint8_t ip[4];
        iptoa(0x100007f, ip);

        TCPSocket_t* sock = tcp_connect(ip, 9090);
        tcpsocket_send(sock, "GET / HTTP", 11);
    }

    if(list_contain_str(mboot_cmd, "--network_enable") != -1)
    {
        init_networkInterfaceManager();
        init_loopback();
        init_rtl8139();
        init_pcnet(); //! Does not work

        switch_interface(1);

        read_mac_addr();

        init_arp();
        init_udp();
        init_dhcp();

        init_tcp();

        dhcp_discover();
        while(isIPReady() == 0);

        TCPSocket_t* s = tcp_listen(9090);
        tcp_bind(s, tcp_sample);
    }

    #if __ENABLE_DEBUG_SYMBOL_LOADING__
    compositor_message_show("Warning:\nENABLE_DEBUG_SYMBOL_LOADING is enabled. This can impact performance when loading executables.");
    #endif

    printf("\nSectorOS shell v2.0.0\nRun help to get the list of commands.\nkshell #> ");

    init_acpi();

    if(list_contain_str(mboot_cmd, "--nomouse") == -1)
    {
        init_mouse();
        register_mouse_handler(compositor_on_mouse_move);
    }

    // init_fat32("/dev/apio1");

    while(1);
}