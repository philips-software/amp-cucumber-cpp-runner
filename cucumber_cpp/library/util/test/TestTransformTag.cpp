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
        std::shared_ptr<cucumber::messages::Tag> MakeTag(std::string name)
        {
            auto tag = std::make_shared<cucumber::messages::Tag>();
            tag->name = std::move(name);
            return tag;
        }
    }

    TEST(TransformTag, collects_unique_sorted_names)
    {
        const std::vector<std::shared_ptr<cucumber::messages::Tag>> tags{ MakeTag("@b"), MakeTag("@a"), MakeTag("@b") };

        const auto names = TransformTags(tags);

        EXPECT_THAT(names, ElementsAre("@a", "@b"));
    }
}
