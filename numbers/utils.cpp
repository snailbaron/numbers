#include "utils.hpp"
#include <cstdlib>

template <class T> T ** Allocate2D(unsigned int iSize, unsigned int jSize)
{
    char *data = (char *) malloc(iSize * sizeof(T *) + iSize * jSize * sizeof(T));

    T **ptr;
    for (unsigned int i = 0; i < iSize; i++)
        ptr[i] = (T *) (data + iSize * sizeof(T *) + i * jSize * sizeof(T));

    return ptr;
}

template <class T> void Free2D(T **ptr)
{
    free(ptr);
}