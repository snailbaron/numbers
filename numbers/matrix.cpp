#include "matrix.hpp"
#include <cstring>

template <class T> Matrix2D<T>::Matrix2D(size_t iSize, size_t jSize) :
    _iSize(iSize),
    _jSize(jSize),
    _matrix(nullptr)
{
    if (_iSize * _jSize)
        _matrix = _Allocate2D(_iSize, _jSize);
}

template <class T> Matrix2D<T>::Matrix2D(const Matrix2D &m) :
    _iSize(m._iSize),
    _jSize(m._jSize),
    _matrix(nullptr)
{
    if (_iSize * _jSize)
    {
        _matrix = _Allocate2D(_iSize, _jSize);
        for (size_t i = 0; i < _iSize; i++)
            for (size_t j = 0; j < _jSize; j++)
                _matrix[i][j] = m._matrix[i][j];
    }
}

template <class T> Matrix2D<T>::~Matrix2D()
{
    free(_matrix);
}

template <class T> Matrix2D<T> & Matrix2D<T>::operator=(const Matrix2D<T> &m)
{
    if (&m != this)
    {
        if (_iSize != m._iSize || _jSize != m._jSize)
        {
            free(_matrix);
            _iSize = m._iSize;
            _jSize = m._jSize;
            _matrix = _Allocate2D(_iSize, _jSize);
        }

        for (size_t i = 0; i < _iSize; i++)
            for (size_t j = 0; j < _jSize; j++)
                _matrix[i][j] = m._matrix[i][j];
    }

    return *this;
}

template <class T> T * Matrix2D<T>::operator[](size_t idx)
{
    return _matrix[idx];
}

template <class T> T ** Matrix2D<T>::_Allocate2D(size_t iSize, size_t jSize)
{
    size_t dataSize = _BufferSize(iSize, jSize);
    char *data = (char *)malloc(dataSize);
    memset(data, 0, dataSize);
    for (unsigned int i = 0; i < iSize; i++)
        *((T **)(data)+i) = (T *)(data + iSize * sizeof(T *) + i * jSize * sizeof(T));

    return (T **)data;
}

template <class T> size_t Matrix2D<T>::_BufferSize(size_t iSize, size_t jSize)
{
    return iSize * sizeof(T *) + iSize * jSize * sizeof(T);
}

template class Matrix2D<int>;