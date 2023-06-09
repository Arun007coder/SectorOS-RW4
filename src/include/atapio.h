#ifndef __ATA_PIO_H__
#define __ATA_PIO_H__


#include "system.h"
#include "string.h"
#include "ports.h"
#include "printf.h"
#include "kheap.h"
#include "devfs.h"
#include "vfs.h"
#include "isr.h"

#define IDENT_DEVICETYPE   0
#define IDENT_CYLINDERS    2
#define IDENT_HEADS        6
#define IDENT_SECTORS      12
#define IDENT_SERIAL       20
#define IDENT_MODEL        54
#define IDENT_CAPABILITIES 98
#define IDENT_FIELDVALID   106
#define IDENT_MAX_LBA      120
#define IDENT_COMMANDSETS  164
#define IDENT_MAX_LBA_EXT  200

typedef struct ata_pio
{
    bool master;

    uint16_t basePort;

    uint16_t dataPort;
    uint16_t errorPort;
    uint16_t sectorCountPort;
    uint16_t lbaLowPort;
    uint16_t lbaMidPort;
    uint16_t lbaHiPort;
    uint16_t devicePort;
    uint16_t commandPort;
    uint16_t controlPort;

    uint32_t bytesPerSector;

    char model[41];
    uint8_t Reserved; // If ATAPI, 0 (empty) otherwise 1 (disk)
    uint8_t channel;
    uint16_t type;
    uint16_t signature;
    uint16_t capabilities;
    uint32_t commandSets;
    uint32_t size; // in LBA or CHS

}ata_pio_t;

void init_ata_pio();

uint32_t ata_pio_read(vfs_node* node, uint32_t offset, uint32_t size, char* buffer);
uint32_t ata_pio_write(vfs_node* node, uint32_t offset, uint32_t size, char* buffer);
void ata_pio_open(vfs_node* node, uint32_t flags);
void ata_pio_close(vfs_node* node);

FILE* ata_pio_GetVFSNode();

char* ata_pio_read_sector(ata_pio_t* ap, uint32_t lba);
void ata_pio_write_sector(ata_pio_t* ap, uint32_t lba, uint8_t* buffer);

void ata_pio_Read28(ata_pio_t* ap, uint32_t lba, uint8_t* buffer, int count);
void ata_pio_Write28(ata_pio_t* ap, uint32_t lba, uint8_t* buffer, int count);

void ata_pio_identify();

#endif /*__ATA_PIO_H__*/