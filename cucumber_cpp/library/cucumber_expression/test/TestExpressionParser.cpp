#include "cucumber_cpp/library/cucumber_expression/Ast.hpp"
#include "cucumber_cpp/library/cucumber_expression/ExpressionParser.hpp"
#include "yaml-cpp/node/node.h"
#include "yaml-cpp/node/parse.h"
#include "yaml-cpp/yaml.h"
#include <cstddef>
#include <filesystem>
#include <format>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <iostream>
#include <map>
#include <ostream>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace cucumber_cpp::library::cucumber_expression
{
    namespace
    {
        const std::map<std::string_view, NodeType> tokenTypeMap = {
            { "TEXT_NODE", NodeType::text },
            { "OPTIONAL_NODE", NodeType::optional },
            { "ALTERNATION_NODE", NodeType::alternation },
            { "ALTERNATIVE_NODE", NodeType::alternative },
            { "PARAMETER_NODE", NodeType::parameter },
            { "EXPRESSION_NODE", NodeType::expression },
        };

        Node CreateNode(const YAML::Node& yaml)
        {
            if (yaml["nodes"])
            {
                Node node{
                    tokenTypeMap.at(yaml["type"].as<std::string>()),
                    yaml["start"].as<std::size_t>(),
                    yaml["end"].as<std::size_t>(),
                    std::vector<Node>{},
                };

                for (const auto& child : yaml["nodes"])
                    node.Children().push_back(CreateNode(child));

                return node;
            }
            else
                return {
                    tokenTypeMap.at(yaml["type"].as<std::string>()),
                    yaml["start"].as<std::size_t>(),
                    yaml["end"].as<std::size_t>(),
                    yaml["token"].as<std::string>(),
                };
        }

        Node CreateNodes(const YAML::Node& yaml)
        {
            return CreateNode(yaml);
        }

        std::vector<std::pair<std::string, YAML::Node>> GetTestData(const std::filesystem::path& path)
        {
            std::vector<std::pair<std::string, YAML::Node>> testdata;

            for (const auto& file : std::filesystem::directory_iterator(path))
                if (file.is_regular_file() && file.path().extension() == ".yaml")
                    testdata.emplace_back(file.path().string(), YAML::LoadFile(file.path().string()));

            return testdata;
        }

        void PrintTo(const Node& node, std::ostream* os)
        {
            std::visit([&node, &os](const auto& arg)
                {
                    if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, std::string>)
                    {
                        *os << "{type:" << static_cast<std::size_t>(node.Type()) << " start:" << node.Start() << " end:" << node.End() << " text: " << arg << "}";
                    }
                    else if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, std::vector<Node>>)
                    {
                        *os << "{type:" << static_cast<std::size_t>(node.Type()) << " start:" << node.Start() << " end:" << node.End() << " children: [";
                        for (const auto& child : arg)
                        {
                            PrintTo(child, os);
                            *os << ", ";
                        }
                        *os << "]}";
                    }
                    else
                    {
                        *os << "{type:" << static_cast<std::size_t>(node.Type()) << " start:" << node.Start() << " end:" << node.End() << "}";
                    }
                },
                node.GetLeafNodes());
        }
    }

    TEST(TestExpressionParser, TestFromFiles)
    {
        std::filesystem::path testdataPath = "testdata/cucumber-expression/parser";

        for (const auto& [file, testdata] : GetTestData(testdataPath))
        {
            if (testdata["exception"])
                ASSERT_ANY_THROW(ExpressionParser{}.Parse(testdata["expression"].as<std::string>()))
                    << std::format("Test failed for file: {}", file);
            else
            {
                const auto actual = ExpressionParser{}.Parse(testdata["expression"].as<std::string>());
                const auto expected = CreateNode(testdata["expected_ast"]);
                ASSERT_THAT(actual, testing::Eq(expected))
                    << std::format("Test failed for file: {}", file);
            }
        }
    }
}
