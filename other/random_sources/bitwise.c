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

int main()
{
    bool result = checkBits(0x112233FF, 9, 10); /* Binary: 0001 0001 0010 0010 0011 0011 1111 1111 */

    printf("Result is: %s\n", result ? "true" : "false");

    return 0;
}
