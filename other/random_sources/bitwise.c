#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

void setBit(uint32_t in, uint8_t pos)
{
    in |= (1U << pos);
}

void clearBit(uint32_t in, uint8_t pos)
{
    in &= ~(1U << pos);
}

bool checkBits(uint32_t in, uint8_t posSet, uint8_t posUnset)
{
    bool ret = false;

    printf("input: 0x%08X\n", in);

    bool isSet = (in & (1U << posSet)) != 0;
    bool isUnset = (in & (1U << posUnset)) == 0;

    printf("Bit at posSet (%d): %s\n", posSet, isSet ? "SET" : "UNSET");
    printf("Bit at posUnset (%d): %s\n", posUnset, isUnset ? "UNSET" : "SET");

    ret = isSet && isUnset;

    return ret;
}

uint8_t reverseBits(uint8_t dataByte)
{
    uint8_t i;
    uint8_t ret = 0;

    for (i = 0; i < 8; i++)
    {
        /* Shift result left to make room for the next bit */
        ret <<= 1U;

        /* Add the least significant bit of dataByte to result */
        ret |= (dataByte & 1U);

        /* Shift dataByte right to get the next bit in the next iteration */
        dataByte >>= 1;
    }

    return ret;
}

int czy_mozna_zapisac(void *dst, const void *src, int size)
{
    uint8_t *d = (uint8_t *)dst;
    const uint8_t *s = (const uint8_t *)src;

    bool copyingNeeded = false;

    for (int i = 0; i < size; i++)
    {
        uint8_t d_byte = d[i];
        uint8_t s_byte = s[i];

        /**
         *
         * Check if any bit in src
         * would flip from 0 bit
         * in dst to 1
         *
         */
        if ((s_byte & ~d_byte) != 0)
        {
            return -1;  /* Detected 0->1 transition between src->dst bits */
        }

        /**
         *
         * If src is not equal to
         * dst then copying is possible
         *
         */
        if (s_byte != d_byte)
        {
            copyingNeeded = true;
        }
    }

    if (!copyingNeeded)
    {
        return 1;  /* Copying not required */
    }

    return 0;  /* Copying possible */
}

int main()
{
    bool result = checkBits(0x112233FF, 9, 10); /* Binary: 0001 0001 0010 0010 0011 0011 1111 1111 */

    printf("Result is: %s\n", result ? "true" : "false");

    uint8_t org = 0x87;
    uint8_t test = reverseBits(org);

    printf("TEST [0x%x, 0x%x]\n",org ,test);

    return 0;
}
