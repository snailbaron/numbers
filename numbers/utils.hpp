#pragma once

template <class T> T ** Allocate2D(unsigned int iSize, unsigned int jSize);
template <class T> void Free2D(T **ptr);