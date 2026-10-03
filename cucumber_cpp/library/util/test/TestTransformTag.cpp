#include "cucumber/messages/Tag.hpp"
#include "cucumber_cpp/library/util/TransformTag.hpp"
#include <gmock/gmock.h>
#include <memory>
#include <string>
#include <vector>

namespace cucumber_cpp::library::util
{
    using testing::ElementsAre;

    namespace
    {
        cucumber::messages::Tag MakeTag(std::string name)
        {
            cucumber::messages::Tag tag;
            tag.name = std::move(name);
            return tag;
        }
    }

    TEST(TransformTag, collects_unique_sorted_names)
    {
        const std::vector<cucumber::messages::Tag> tags{ MakeTag("@b"), MakeTag("@a"), MakeTag("@b") };

        const auto names = TransformTags(tags);

        EXPECT_THAT(names, ElementsAre("@a", "@b"));
    }
}
