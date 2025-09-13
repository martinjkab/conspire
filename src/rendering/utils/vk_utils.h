#pragma once

#define VK_CHECK(x)       \
    do                    \
    {                     \
        VkResult err = x; \
        if (err)          \
        {                 \
            std::abort(); \
        }                 \
    } while (0)