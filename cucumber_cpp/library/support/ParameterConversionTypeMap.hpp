#ifndef SUPPORT_PARAMETER_CONVERSION_TYPE_MAP_HPP
#define SUPPORT_PARAMETER_CONVERSION_TYPE_MAP_HPP

#include "cucumber/messages/group.hpp"
#include <functional>
#include <map>
#include <string>

namespace cucumber_cpp::library::support
{
    // template<class T>
    // using TypeMap = std::map<std::string, std::function<T(const cucumber::messages::group&)>>;

    // template<class T>
    // struct ConverterTypeMap
    // {
    //     static std::map<std::string, std::function<T(const cucumber::messages::group&)>>& Instance();
    // };

    // template<class T>
    // std::map<std::string, std::function<T(const cucumber::messages::group&)>>& ConverterTypeMap<T>::Instance()
    // {
    //     static std::map<std::string, std::function<T(const cucumber::messages::group&)>> typeMap;
    //     return typeMap;
    // }
}

#endif
