#include "BaseCompatibility.hpp"
#include <string>

#ifndef KIT_FOLDER
#error KIT_FOLDER is not defined
#define KIT_FOLDER ""
#endif

#ifndef KIT_NDJSON_FILE
#error KIT_NDJSON_FILE is not defined
#define KIT_NDJSON_FILE ""
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
