#include "matrix.hpp"
#include <cstring>

template <class T> Matrix2D<T>::Matrix2D() :
    _iSize(0),
    _jSize(0),
    _matrix(nullptr)
{
}

template <class T> Matrix2D<T>::Matrix2D(size_t iSize, size_t jSize) :
    _iSize(iSize),
    _jSize(jSize),
    _matrix(nullptr)
{
    size_t dataSize = iSize * sizeof(T *) + iSize * jSize * sizeof(T);
    char *data = (char *)malloc(dataSize);
    memset(data, 0, dataSize);
    for (unsigned int i = 0; i < iSize; i++)
        *((T **)(data)+i) = (T *)(data + iSize * sizeof(T *) + i * jSize * sizeof(T));

    _matrix = (T **)data;
}

template <class T> Matrix2D<T>::~Matrix2D()
{
    free(_matrix);
}

template class Matrix2D<int>;