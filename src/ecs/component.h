#pragma once
#include <ostream>

class Component
{
public:
    virtual ~Component() = default;

    friend std::ostream &operator<<(std::ostream &os, const Component &comp)
    {
        (void)comp;
        os << "Component";
        return os;
    }
};