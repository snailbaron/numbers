#pragma once

#include <utility>

template <class T> class Matrix2D
{
public:
    Matrix2D();
    Matrix2D(size_t iSize, size_t jSize);
    ~Matrix2D();

    const size_t & GetISize() const { return _iSize; }
    const size_t & GetJSize() const { return _jSize; }
    const std::pair<size_t, size_t> GetSize() const { return std::make_pair(_iSize, _jSize); }

    T * operator[](size_t idx) { return _matrix[idx]; }
    const T * operator[](size_t idx) const { return _matrix[idx]; }

private:
    size_t _iSize;
    size_t _jSize;
    T **_matrix;
};