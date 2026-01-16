
#include "cucumber_cpp/library/formatter/JunitXmlFormatter.hpp"
#include "cucumber_cpp/library/query/Query.hpp"
#include <cucumber/messages/envelope.hpp>
#include <pugixml.cpp>
#include <string>

namespace cucumber_cpp::library::formatter
{
    namespace
    {
        void MakeReport(query::Query& query, std::string testClassName)
        {
            
        }
    }

    void JunitXmlFormatter::OnEnvelope(const cucumber::messages::envelope& envelope)
    {
        if (envelope.test_run_finished)
        {
            pugi::xml_node xml;
        }
    }
}
