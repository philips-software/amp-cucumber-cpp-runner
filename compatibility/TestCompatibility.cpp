#include "BaseCompatibility.hpp"
#include <string>

#ifndef KIT_FOLDER
#error KIT_FOLDER is not defined
#endif

#ifndef KIT_NDJSON_FILE
#error KIT_NDJSON_FILE is not defined
#endif

TEST(CompatibilityTest, KIT_NAME)
{
#ifdef SKIP_TEST
    GTEST_SKIP();
#endif

    compatibility::RunDevkit({
        .paths = { KIT_FOLDER },
        .tagExpression = "",
        .retry = 0,
        .ndjsonFile = KIT_NDJSON_FILE,
        .folder = KIT_FOLDER,
        .kitString = KIT_STRING,
    });
}
