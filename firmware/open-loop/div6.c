#include <stdint.h>

uint32_t foo0(uint16_t n);

__attribute__((optimize("O0")))
uint32_t foo0(uint16_t n)
{
    return n / 3;
}

uint32_t foo3(uint16_t n);

__attribute__((optimize("O3")))
uint32_t foo3(uint16_t n)
{
    return n / 3;
}

__attribute__((optimize("O3")))
uint32_t mul3(uint16_t n)
{
    return (uint32_t)n * 2731 >> 13;
}
