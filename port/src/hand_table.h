/* hand_table.h - routine address -> hand-written C (tools/hand_table.py). */
#ifndef HAND_TABLE_H
#define HAND_TABLE_H
#include <stdint.h>

typedef struct {
    uint32_t pc;            /* 24-bit routine address */
    uint8_t m, x;           /* register widths the variant was written for */
    void (*fn)(void);
} HandEntry;

extern const HandEntry g_hand_table[];
extern const int g_hand_table_count;

#endif
