#pragma once

#include <utility>

template <class T> class Matrix2D
{
public:
    Matrix2D(size_t iSize = 0, size_t jSize = 0);
    Matrix2D(const Matrix2D &m);
    ~Matrix2D();

    const size_t & GetISize() const { return _iSize; }
    const size_t & GetJSize() const { return _jSize; }
    const std::pair<size_t, size_t> GetSize() const { return std::make_pair(_iSize, _jSize); }

    Matrix2D<T> & operator=(const Matrix2D<T> &m);
    T * operator[](size_t idx);
    const T * operator[](size_t idx) const { return _matrix[idx]; }

private:
    static T ** _Allocate2D(size_t iSize, size_t jSize);
    static size_t _BufferSize(size_t iSize, size_t jSize);

private:
    size_t _iSize;
    size_t _jSize;
    T **_matrix;
};