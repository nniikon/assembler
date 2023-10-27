#include "../include/SPU_graph.h"


void renderRam_console(const int* ram, const size_t size)
{
    system("clear");
    assert(SPU_VRAM_HEIGHT * SPU_VRAM_WIDTH + SPU_VRAM_SHIFT <= size);
    const int* vram = ram + SPU_VRAM_SHIFT;
    for (size_t y = 0; y < SPU_VRAM_WIDTH; y++)
    {
        for (size_t x = 0; x < SPU_VRAM_HEIGHT; x++)
        {
            switch (vram[SPU_VRAM_WIDTH * y + x])
            {
            case 0:
                printf("0");
                break;
            default:
                printf("%%");
                break;
            }
        }
        fputc('\n', stdout);
    }
    
}