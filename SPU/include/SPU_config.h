// VRAM frame.
const size_t SPU_VRAM_WIDTH  = 20;
const size_t SPU_VRAM_HEIGHT = 10;

// Unrendered RAM capacity. 
const size_t SPU_VRAM_SHIFT = 100;

const size_t SPU_RAM_CAPACITY = SPU_VRAM_SHIFT + SPU_VRAM_HEIGHT * SPU_VRAM_WIDTH;