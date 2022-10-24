#include <stdarg.h>
#include <os_internal.h>
#include "controller.h"
#include "piint.h"
#include "../libc/xstdio.h"

#define gISVDbgPrnAdrs ((ISVDbg*)0xB3FF0000)
#define IS64_MAGIC ((u32)(('I' << 24) | ('S' << 16) | ('6' << 8) | ('4' << 0)))

typedef struct {
    /* 0x00 */ u32 magic; // IS64
    /* 0x04 */ u32 get;
    /* 0x08 */ u8 unk_08[0x14-0x08];
    /* 0x14 */ u32 put;
    /* 0x18 */ u8 unk_18[0x20-0x18];
    /* 0x20 */ u8 data[0x10000-0x20];
} ISVDbg;

s32 osPiWriteIo(u32 devAddr, u32 data)
{
    register s32 stat;

    __osPiGetAccess();
    WAIT_ON_IOBUSY(stat);
    IO_WRITE((u32)osRomBase | devAddr, data);
    __osPiRelAccess();

    return 0;
}

char* is_proutSyncPrintf(char* arg, const char* str, size_t count) {
    u32 data;
    s32 pos;
    s32 start;
    s32 end;

    osPiReadIo((u32)&gISVDbgPrnAdrs->get, &data);
    pos = data;
    osPiReadIo((u32)&gISVDbgPrnAdrs->put, &data);
    start = data;
    end = start + count;
    if (end >= 0xFFE0) {
        end -= 0xFFE0;
        if (pos < end || start < pos) {
            return (void*)1;
        }
    } else {
        if (start < pos && pos < end) {
            return (void*)1;
        }
    }
    while (count) {
        u32 addr = (u32)&gISVDbgPrnAdrs->data + (start & 0xFFFFFFC);
        s32 shift = ((3 - (start & 3)) * 8);

        if (*str) {
            osPiReadIo(addr, &data);
            osPiWriteIo(addr, (*str << shift) | (data & ~(0xFF << shift)));

            start++;
            if (start >= 0xFFE0) {
                start -= 0xFFE0;
            }
        }
        count--;
        str++;
    }
    osPiWriteIo((u32)&gISVDbgPrnAdrs->put, start);

    return (void*)1;
}

u8 is64active = 0;

void pdxOsSyncPrintf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    if (is64active) {
        _Printf(is_proutSyncPrintf, NULL, fmt, args);
    }

    va_end(args);
}

void hudmsgCreate(char *text, s32 type);

void pdxPrintf(const char* fmt, ...) {
    va_list args;
    char tmpbuffer[2048];
    va_start(args, fmt);

    pdxOsSyncPrintf(fmt, args);

    sprintf(tmpbuffer, fmt, args);
    hudmsgCreate(tmpbuffer, 1);

    va_end(args);
}

void isPrintfInit(void) {
    osPiWriteIo((u32)&gISVDbgPrnAdrs->put, 0);
    osPiWriteIo((u32)&gISVDbgPrnAdrs->get, 0);
    osPiWriteIo((u32)&gISVDbgPrnAdrs->magic, IS64_MAGIC);
    if (gISVDbgPrnAdrs->magic == IS64_MAGIC) {
        is64active = 1;
        pdxOsSyncPrintf("IS-Viewer initialized.\n");
    } else {
        is64active = 0;
    }
}
