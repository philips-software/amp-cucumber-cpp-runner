#include "SourceInfo.hpp"

namespace cucumber_cpp::library::engine
{

    SourceInfo::SourceInfo(std::filesystem::path path, cucumber::messages::source  source)
        : source(std::move(source))
        , path(std::move(path))
    {

    }

    const std::filesystem::path& SourceInfo::Path() const
    {
        return path;
    }

    std::string SourceInfo::ToJson() const
    {
        return source.to_json();
    }
}
