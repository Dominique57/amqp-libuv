#pragma once

namespace example
{
    template<typename T>
    T add(T lhs, T rhs)
    { return lhs + rhs; }

    int add_int(int lhs, int rhs);
}
