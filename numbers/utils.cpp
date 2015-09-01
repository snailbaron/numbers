#include "utils.hpp"

int FlatRand(int left, int right)
{
    return (rand() % (right - left + 1)) + left;
}