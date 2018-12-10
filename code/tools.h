#pragma once

void CatStrings(size_t sourceACount, char *sourceA,
                size_t sourceBCount, char *sourceB,
                size_t destCount, char *dest)
{

    for (i32 index = 0; index < sourceACount; ++index)
    {
        *dest++ = *sourceA++;
    }

    for (i32 index = 0; index < sourceBCount; ++index)
    {
        *dest++ = *sourceB++;
    }

    *dest++ = 0;
}

int StringLength(char *string)
{
    i32 count = 0;
    while (*string++)
    {
        ++count;
    }
    return count;
}
