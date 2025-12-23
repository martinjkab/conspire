#pragma once
#include <ostream>

class ComponentBase
{
public:
    virtual ~ComponentBase() = default;

    friend std::ostream &operator<<(std::ostream &os, const ComponentBase &comp)
    {
        (void)comp;
        os << "Component";
        return os;
    }
};