#ifndef D5276DD0_2B76_4493_83B4_DC4A0849A343
#define D5276DD0_2B76_4493_83B4_DC4A0849A343

#include "nlohmann/json_fwd.hpp"

namespace cucumber_cpp::report
{
    struct Report
    {
        virtual ~Report() = default;

        virtual void GenerateReport(const nlohmann::json& json) = 0;
    };
}

#endif /* D5276DD0_2B76_4493_83B4_DC4A0849A343 */
