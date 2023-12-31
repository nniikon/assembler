#include "../include/SPU_graph.h"


void renderRam(const int* ram, char* vramBuffer, const size_t size)
{
    //system("clear");
    assert(SPU_VRAM_HEIGHT * SPU_VRAM_WIDTH + SPU_VRAM_SHIFT <= size);
    const int* vram = ram;

    for (size_t y = 0; y < SPU_VRAM_HEIGHT; y++)
    {
        for (size_t x = 0; x < SPU_VRAM_WIDTH + 1; x++)
        {
            if (x == SPU_VRAM_WIDTH)
            {
                vramBuffer[(SPU_VRAM_WIDTH + 1) * y + x] = '\n';
                continue;
            }
            // TODO: add struct / enum.
            switch (vram[SPU_VRAM_WIDTH * y + x])
            {
                case 0:
                    vramBuffer[(SPU_VRAM_WIDTH + 1)* y + x] = '.';
                    break;
                case 100:
                    vramBuffer[(SPU_VRAM_WIDTH + 1)* y + x] = '*';
                    break;
                case 200:
                    vramBuffer[(SPU_VRAM_WIDTH + 1)* y + x] = '%';
                    break;
                default:
                    vramBuffer[(SPU_VRAM_WIDTH + 1)* y + x] = '&';
                    break;
            }
        }
    }
    fwrite(vramBuffer, (SPU_VRAM_HEIGHT) * (SPU_VRAM_WIDTH + 1), sizeof(char), stdout);
}