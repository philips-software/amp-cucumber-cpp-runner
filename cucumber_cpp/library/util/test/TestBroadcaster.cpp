#include "cucumber/messages/Envelope.hpp"
#include "cucumber/messages/ExternalAttachment.hpp"
#include "cucumber/messages/Meta.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include <gmock/gmock.h>
#include <memory>

namespace cucumber_cpp::library::util
{
    using testing::IsTrue;

    TEST(Broadcaster, dispatches_external_attachment_and_meta)
    {
        Broadcaster broadcaster;
        bool sawExternalAttachment = false;
        bool sawMeta = false;

        const Listener listener{ broadcaster, [&](const cucumber::messages::Envelope& envelope)
            {
                sawExternalAttachment = sawExternalAttachment || envelope.externalAttachment.has_value();
                sawMeta = sawMeta || envelope.meta.has_value();
            } };

        broadcaster.BroadcastEvent(std::make_shared<cucumber::messages::ExternalAttachment>());
        broadcaster.BroadcastEvent(std::make_shared<cucumber::messages::Meta>());

        EXPECT_THAT(sawExternalAttachment, IsTrue());
        EXPECT_THAT(sawMeta, IsTrue());
    }
}
