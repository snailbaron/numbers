#pragma once

#include <Windows.h>

template <class T> T ** Allocate2D(unsigned int iSize, unsigned int jSize)
{
    char *data = (char *)malloc(iSize * sizeof(T *) + iSize * jSize * sizeof(T));
    ZeroMemory(data, iSize * sizeof(T *) + iSize * jSize * sizeof(T));
    for (unsigned int i = 0; i < iSize; i++)
        *((T **)(data) + i) = (T *)(data + iSize * sizeof(T *) + i * jSize * sizeof(T));

    return (T **) data;
}

template <class T> void Free2D(T **ptr)
{
    free(ptr);
}

// Release COM object
template <class T> void ReleaseCOM(T **ptr)
{
    if (*ptr != NULL)
    {
        (*ptr)->Release();
        (*ptr) = NULL;
    }
}