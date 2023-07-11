#include "system.h"
#include "modules.h"

#include "paging.h"
#include "kheap.h"
#include "string.h"
#include "vfs.h"
#include "devfs.h"

#include "spinlock.h"

// A prototype sound subsystem for SectorOS

typedef struct sound_knob_struct
{
    char name[16];
    uint32_t id;
}sound_knob_t;

typedef int (*sound_knob_read)(uint32_t knob_id, uint32_t* val);
typedef int (*sound_knob_write)(uint32_t knob_id, uint32_t val);

typedef int (*sound_set_sampling_rate)(uint32_t id, uint32_t rate);
typedef int (*sound_get_sampling_rate)(uint32_t id, uint32_t* val);

#define SOUND_TYPE_8  0x00
#define SOUND_TYPE_16 0x01

typedef struct sound_driver_info_struct
{
    char name[32];
    uint32_t id;

    uint32_t sampling_rate;
    uint32_t type;
    uint32_t flags;
    uint32_t knobs;
    uint32_t queued_packets;
}sound_driver_info_t;

#define DSP_IOCTL_GET_DRIVER_IDS 0x1231
#define DSP_IOCTL_SET_DRIVER 0x1232
#define DSP_IOCTL_GET_SAMPLING_RATE 0x1233
#define DSP_IOCTL_SET_SAMPLING_RATE 0x1234
#define DSP_IOCTL_GET_CURRENT_DRIVER_ID 0x1235

typedef struct sound_packet_struct
{
    uint32_t size;
    void* buffer;
    listnode_t* self;
}sound_packet_t;

typedef struct sound_driver_struct
{
    char name[32];
    listnode_t* self;
    
    void* conf;

    uint32_t type;

    uint32_t max_packet_size;

    uint32_t id;
    uint32_t flags;

    sound_knob_read read_knob;
    sound_knob_write write_knob;

    sound_get_sampling_rate get_rate;
    sound_set_sampling_rate set_rate;

    list_t* knobs;

    list_t* packets;
}sound_driver_t;

static list_t* drivers = NULL;

static int init(int argc, char** argv)
{
    drivers = list_create();

    return 0;
}

static uint32_t dsp_write(FILE* f, uint32_t offset, uint32_t size, char* buffer)
{
    if(size == 0)
        return 0;

    sound_driver_t* driver = f->device;

    uint32_t count = 0;
    count = size / driver->max_packet_size;

    for(uint32_t i = 0; i < count; i++)
    {
        sound_packet_t* packet = ZALLOC_TYPES(sound_packet_t);
        packet->size = driver->max_packet_size;
        packet->buffer = zalloc(driver->max_packet_size);
        memcpy(packet->buffer, buffer+(driver->max_packet_size*i), driver->max_packet_size);
        packet->self = list_insert_back2(driver->packets, packet);
    }

    sound_packet_t* packet = ZALLOC_TYPES(sound_packet_t);
    packet->size = size % (driver->max_packet_size);
    packet->buffer = zalloc(packet->size);
    memcpy(packet->buffer, buffer+(driver->max_packet_size*count), packet->size);
    packet->self = list_insert_back2(driver->packets, packet);

    return count;
}

int dsp_ioctl(FILE* f, int request, void *data)
{
    switch(request)
    {
    case DSP_IOCTL_GET_CURRENT_DRIVER_ID:
    {
        uint32_t* id = data;
        sound_driver_t* drv = f->device;
        if(validate(drv) != 1)
            return -1;
        if(validate(data) != 1)
            return -1;
        *id = drv->id;
        return 0;
    }break;
    case DSP_IOCTL_SET_DRIVER:
    {
        uint32_t* id = data;
        if(validate(id) != 1)
            return -1;
        if(validate(drivers) != 1)
            return -1;

        sound_driver_t* drv = NULL;
        foreach(l, drivers)
        {
            sound_driver_t* d = l->val;
            if(d->id == *id)
            {
                drv = d;
            }
        }

        if(validate(drv) != 1)
            return -1;

        f->device = drv;

        return 0;
    }break;
    case DSP_IOCTL_GET_SAMPLING_RATE:
    {
        uint32_t* rate = data;
        if(validate(data) != 1)
            return -1;

        sound_driver_t* drv = f->device;

        if(validate(drv) != 1)
            return -1;

        if(!drv->get_rate)
            return -1;

        return drv->get_rate(drv->id, rate);
    }break;
    case DSP_IOCTL_SET_SAMPLING_RATE:
    {
        uint32_t* rate = data;
        if(validate(data) != 1)
            return -1;

        sound_driver_t* drv = f->device;

        if(validate(drv) != 1)
            return -1;

        if(!drv->set_rate)
            return -1;

        return drv->set_rate(drv->id, *rate);
    }break;
    case DSP_IOCTL_GET_DRIVER_IDS:
    {
        uint32_t* ids = data;
        if(validate(data) != 1)
            return -1;
        
        if(validate(data + (list_size(drivers)*sizeof(uint32_t))) != 1)
            return -1;

        int j = 0;
        foreach(l, drivers)
        {
            sound_driver_t* drv = l->val;
            ids[j++] = drv->id;
        }

        return 0;
    }break;
    default:
    {
        return -1;
    }break;
    };
}

static int fini()
{
    return 0;
}

MODULE_DEF(sound, init, fini);