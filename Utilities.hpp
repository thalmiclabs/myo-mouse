#pragma once

namespace utils {

template<typename T>
T clamp(T value, T min, T max)
{
    return value < min ? min : value > max ? max : value;
}

template<typename T>
T extractFractional(T value)
{
    return value - std::trunc(value);
}
} // namespace utils
