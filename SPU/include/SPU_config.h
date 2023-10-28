// VRAM frame.
const size_t SPU_VRAM_WIDTH  = 50;
const size_t SPU_VRAM_HEIGHT = 25;

// Unrendered RAM capacity. 
const size_t SPU_VRAM_SHIFT = 10;

const size_t SPU_RAM_CAPACITY = SPU_VRAM_SHIFT + SPU_VRAM_HEIGHT * SPU_VRAM_WIDTH;