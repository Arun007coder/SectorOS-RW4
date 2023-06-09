#include "idt.h"

extern void flush_idt(uint32_t);

idt_entry_t idt_entries[256];
idt_ptr_t   idt_ptr;

void init_idt()
{
    printf("[IDT] Configuring IDT pointer...\n");
    idt_ptr.limit = (sizeof(idt_entry_t) * 256) - 1;
    idt_ptr.base  = (uint32_t)&idt_entries;

    memset(&idt_entries, 0, sizeof(idt_entry_t) * 256);

    printf("[IDT] Adding entries...\n");
    set_idtGate(0, (uint32_t)exception0, 0x08, 0x8E);
    set_idtGate(1, (uint32_t)exception1, 0x08, 0x8E);
    set_idtGate(2, (uint32_t)exception2, 0x08, 0x8E);
    set_idtGate(3, (uint32_t)exception3, 0x08, 0x8E);
    set_idtGate(4, (uint32_t)exception4, 0x08, 0x8E);
    set_idtGate(5, (uint32_t)exception5, 0x08, 0x8E);
    set_idtGate(6, (uint32_t)exception6, 0x08, 0x8E);
    set_idtGate(7, (uint32_t)exception7, 0x08, 0x8E);
    set_idtGate(8, (uint32_t)exception8, 0x08, 0x8E);
    set_idtGate(9, (uint32_t)exception9, 0x08, 0x8E);
    set_idtGate(10, (uint32_t)exception10, 0x08, 0x8E);
    set_idtGate(11, (uint32_t)exception11, 0x08, 0x8E);
    set_idtGate(12, (uint32_t)exception12, 0x08, 0x8E);
    set_idtGate(13, (uint32_t)exception13, 0x08, 0x8E);
    set_idtGate(14, (uint32_t)exception14, 0x08, 0x8E);
    set_idtGate(15, (uint32_t)exception15, 0x08, 0x8E);
    set_idtGate(16, (uint32_t)exception16, 0x08, 0x8E);
    set_idtGate(17, (uint32_t)exception17, 0x08, 0x8E);
    set_idtGate(18, (uint32_t)exception18, 0x08, 0x8E);
    set_idtGate(19, (uint32_t)exception19, 0x08, 0x8E);
    set_idtGate(20, (uint32_t)exception20, 0x08, 0x8E);
    set_idtGate(21, (uint32_t)exception21, 0x08, 0x8E);
    set_idtGate(22, (uint32_t)exception22, 0x08, 0x8E);
    set_idtGate(23, (uint32_t)exception23, 0x08, 0x8E);
    set_idtGate(24, (uint32_t)exception24, 0x08, 0x8E);
    set_idtGate(25, (uint32_t)exception25, 0x08, 0x8E);
    set_idtGate(26, (uint32_t)exception26, 0x08, 0x8E);
    set_idtGate(27, (uint32_t)exception27, 0x08, 0x8E);
    set_idtGate(28, (uint32_t)exception28, 0x08, 0x8E);
    set_idtGate(29, (uint32_t)exception29, 0x08, 0x8E);
    set_idtGate(30, (uint32_t)exception30, 0x08, 0x8E);
    set_idtGate(31, (uint32_t)exception31, 0x08, 0x8E);

    set_idtGate(32, (uint32_t)isr0, 0x08, 0x8E);
    set_idtGate(33, (uint32_t)isr1, 0x08, 0x8E);
    set_idtGate(34, (uint32_t)isr2, 0x08, 0x8E);
    set_idtGate(35, (uint32_t)isr3, 0x08, 0x8E);
    set_idtGate(36, (uint32_t)isr4, 0x08, 0x8E);
    set_idtGate(37, (uint32_t)isr5, 0x08, 0x8E);
    set_idtGate(38, (uint32_t)isr6, 0x08, 0x8E);
    set_idtGate(39, (uint32_t)isr7, 0x08, 0x8E);
    set_idtGate(40, (uint32_t)isr8, 0x08, 0x8E);
    set_idtGate(41, (uint32_t)isr9, 0x08, 0x8E);
    set_idtGate(42, (uint32_t)isr10, 0x08, 0x8E);
    set_idtGate(43, (uint32_t)isr11, 0x08, 0x8E);
    set_idtGate(44, (uint32_t)isr12, 0x08, 0x8E);
    set_idtGate(45, (uint32_t)isr13, 0x08, 0x8E);
    set_idtGate(46, (uint32_t)isr14, 0x08, 0x8E);
    set_idtGate(47, (uint32_t)isr15, 0x08, 0x8E);

    set_idtGate(128, (uint32_t)exception128, 0x08, (0x8E | 0x60));

    printf("[IDT] Flushing IDT entries...\n");
    flush_idt((uint32_t)&idt_ptr);
    printf("[IDT] Initialized successfully\n");
}

void idt_reinit()
{
    idt_ptr.limit = sizeof(idt_entry_t) * 256 - 1;
    idt_ptr.base  = (uint32_t)&idt_entries;

    memset(&idt_entries, 0, sizeof(idt_entry_t) * 256);

    set_idtGate(0, (uint32_t)exception0, 0x08, 0x8E);
    set_idtGate(1, (uint32_t)exception1, 0x08, 0x8E);
    set_idtGate(2, (uint32_t)exception2, 0x08, 0x8E);
    set_idtGate(3, (uint32_t)exception3, 0x08, 0x8E);
    set_idtGate(4, (uint32_t)exception4, 0x08, 0x8E);
    set_idtGate(5, (uint32_t)exception5, 0x08, 0x8E);
    set_idtGate(6, (uint32_t)exception6, 0x08, 0x8E);
    set_idtGate(7, (uint32_t)exception7, 0x08, 0x8E);
    set_idtGate(8, (uint32_t)exception8, 0x08, 0x8E);
    set_idtGate(9, (uint32_t)exception9, 0x08, 0x8E);
    set_idtGate(10, (uint32_t)exception10, 0x08, 0x8E);
    set_idtGate(11, (uint32_t)exception11, 0x08, 0x8E);
    set_idtGate(12, (uint32_t)exception12, 0x08, 0x8E);
    set_idtGate(13, (uint32_t)exception13, 0x08, 0x8E);
    set_idtGate(14, (uint32_t)exception14, 0x08, 0x8E);
    set_idtGate(15, (uint32_t)exception15, 0x08, 0x8E);
    set_idtGate(16, (uint32_t)exception16, 0x08, 0x8E);
    set_idtGate(17, (uint32_t)exception17, 0x08, 0x8E);
    set_idtGate(18, (uint32_t)exception18, 0x08, 0x8E);
    set_idtGate(19, (uint32_t)exception19, 0x08, 0x8E);
    set_idtGate(20, (uint32_t)exception20, 0x08, 0x8E);
    set_idtGate(21, (uint32_t)exception21, 0x08, 0x8E);
    set_idtGate(22, (uint32_t)exception22, 0x08, 0x8E);
    set_idtGate(23, (uint32_t)exception23, 0x08, 0x8E);
    set_idtGate(24, (uint32_t)exception24, 0x08, 0x8E);
    set_idtGate(25, (uint32_t)exception25, 0x08, 0x8E);
    set_idtGate(26, (uint32_t)exception26, 0x08, 0x8E);
    set_idtGate(27, (uint32_t)exception27, 0x08, 0x8E);
    set_idtGate(28, (uint32_t)exception28, 0x08, 0x8E);
    set_idtGate(29, (uint32_t)exception29, 0x08, 0x8E);
    set_idtGate(30, (uint32_t)exception30, 0x08, 0x8E);
    set_idtGate(31, (uint32_t)exception31, 0x08, 0x8E);

    set_idtGate(32, (uint32_t)isr0, 0x08, 0x8E);
    set_idtGate(33, (uint32_t)isr1, 0x08, 0x8E);
    set_idtGate(34, (uint32_t)isr2, 0x08, 0x8E);
    set_idtGate(35, (uint32_t)isr3, 0x08, 0x8E);
    set_idtGate(36, (uint32_t)isr4, 0x08, 0x8E);
    set_idtGate(37, (uint32_t)isr5, 0x08, 0x8E);
    set_idtGate(38, (uint32_t)isr6, 0x08, 0x8E);
    set_idtGate(39, (uint32_t)isr7, 0x08, 0x8E);
    set_idtGate(40, (uint32_t)isr8, 0x08, 0x8E);
    set_idtGate(41, (uint32_t)isr9, 0x08, 0x8E);
    set_idtGate(42, (uint32_t)isr10, 0x08, 0x8E);
    set_idtGate(43, (uint32_t)isr11, 0x08, 0x8E);
    set_idtGate(44, (uint32_t)isr12, 0x08, 0x8E);
    set_idtGate(45, (uint32_t)isr13, 0x08, 0x8E);
    set_idtGate(46, (uint32_t)isr14, 0x08, 0x8E);
    set_idtGate(47, (uint32_t)isr15, 0x08, 0x8E);

    set_idtGate(128, (uint32_t)exception128, 0x08, (0x8E | 0x60));

    flush_idt((uint32_t)&idt_ptr);
}

void set_idtGate(uint8_t num, uint32_t base, uint16_t selector, uint8_t flags)
{
    idt_entries[num].offset_low = base & 0xFFFF;
    idt_entries[num].offset_high = (base >> 16) & 0xFFFF;

    idt_entries[num].selector = selector;
    idt_entries[num].always0 = 0;

    idt_entries[num].flags = flags;
}
