#include "../include/SPU_graph.h"


void renderRam_console(const int* ram, const size_t size)
{
    //system("clear");
    assert(SPU_VRAM_HEIGHT * SPU_VRAM_WIDTH + SPU_VRAM_SHIFT <= size);
    const int* vram = ram;

    char buffer[(SPU_VRAM_HEIGHT) * (SPU_VRAM_WIDTH + 1)] = {};

    for (size_t y = 0; y < SPU_VRAM_HEIGHT; y++)
    {
        for (size_t x = 0; x < SPU_VRAM_WIDTH + 1; x++)
        {
            if (x == SPU_VRAM_WIDTH)
            {
                buffer[(SPU_VRAM_WIDTH + 1) * y + x] = '\n';
                continue;
            }

            switch (vram[SPU_VRAM_WIDTH * y + x])
            {
            case 0:
                buffer[(SPU_VRAM_WIDTH + 1)* y + x] = '0';
                break;
            default:
                buffer[(SPU_VRAM_WIDTH + 1)* y + x] = '%';
                break;
            }
        }
    }
    fwrite(buffer, (SPU_VRAM_HEIGHT) * (SPU_VRAM_WIDTH + 1), sizeof(char), stdout);
}