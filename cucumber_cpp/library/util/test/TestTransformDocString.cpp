#include "cucumber/messages/PickleDocString.hpp"
#include "cucumber_cpp/library/util/DocString.hpp"
#include "cucumber_cpp/library/util/TransformDocString.hpp"
#include <gmock/gmock.h>
#include <optional>
#include <string>

namespace cucumber_cpp::library::util
{
    using testing::Eq;
    using testing::IsFalse;
    using testing::IsTrue;

    TEST(TransformDocString, returns_nullopt_for_empty_input)
    {
        EXPECT_THAT(TransformDocString(std::optional<cucumber::messages::PickleDocString>{}).has_value(), IsFalse());
        EXPECT_THAT(TransformDocString(std::optional<DocString>{}).has_value(), IsFalse());
    }

    TEST(TransformDocString, roundtrips_media_type_and_content)
    {
        cucumber::messages::PickleDocString source;
        source.mediaType = "text/plain";
        source.content = "hello";
        const std::optional<cucumber::messages::PickleDocString> pickleOpt{ source };

        const auto doc = TransformDocString(pickleOpt);

        ASSERT_THAT(doc.has_value(), IsTrue());
        ASSERT_THAT(doc->mediaType.has_value(), IsTrue());
        EXPECT_THAT(*doc->mediaType, Eq("text/plain"));
        EXPECT_THAT(doc->content, Eq("hello"));

        const std::optional<std::string> mediaType{ "text/plain" };
        const std::string content{ "hello" };
        const std::optional<DocString> docOpt{ DocString{ .mediaType = mediaType, .content = content } };

        const auto back = TransformDocString(docOpt);

        ASSERT_THAT(back.has_value(), IsTrue());
        EXPECT_THAT(back->content, Eq("hello"));
    }
}
