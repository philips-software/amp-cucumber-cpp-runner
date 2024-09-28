#ifndef FIXTURES_FIXTURE_HPP
#define FIXTURES_FIXTURE_HPP

#include <string_view>

struct NordicBleFixture
{};

struct StBleFixture
{
    StBleFixture(std::string_view coms)
        : coms(coms)
    {}

    std::string_view coms;
};

#endif
