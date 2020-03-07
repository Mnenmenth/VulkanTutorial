/**
  * Created by Earl Kennedy
  * https://github.com/Mnenmenth
  */

#ifndef VULKANTUTORIAL_TYPES_H
#define VULKANTUTORIAL_TYPES_H

#include <cstdint>
#include <cstddef>

namespace type
{
    using uint16 = std::uint16_t;
    constexpr uint16 uint16_max = UINT16_MAX;
    using uint32 = std::uint32_t;
    constexpr uint32 uint32_max = UINT32_MAX;
    using uint64 = std::uint64_t;
    constexpr uint64 uint64_max = UINT64_MAX;
    using size = std::size_t;

    using cstr = const char*;
}

#endif //VULKANTUTORIAL_TYPES_H
