#ifndef UTIL_IMMOVEABLE_HPP
#define UTIL_IMMOVEABLE_HPP

namespace cucumber_cpp::library::util
{
    struct Immoveable
    {
    protected:
        Immoveable() = default;
        ~Immoveable() = default;

    public:
        Immoveable(const Immoveable&) = delete;
        Immoveable& operator=(const Immoveable&) = delete;
    };
}

#endif
