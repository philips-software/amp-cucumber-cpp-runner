#include "cucumber_cpp/library/cucumber_expression/Ast.hpp"
#include "cucumber_cpp/library/cucumber_expression/Errors.hpp"
#include "cucumber_cpp/library/cucumber_expression/ExpressionParser.hpp"
#include <cstddef>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <ostream>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

namespace cucumber_cpp::library::cucumber_expression
{
    void PrintTo(const Node& node, std::ostream* os)
    {
        std::visit([&node, &os](const auto& arg)
            {
                if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, std::string>)
                {
                    *os << "{type:" << static_cast<std::size_t>(node.type) << " start:" << node.start << " end:" << node.end << " text: " << arg << "}";
                }
                else if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, std::vector<Node>>)
                {
                    *os << "{type:" << static_cast<std::size_t>(node.type) << " start:" << node.start << " end:" << node.end << " children: [";
                    for (const auto& child : arg)
                    {
                        PrintTo(child, os);
                        *os << ", ";
                    }
                    *os << "]}";
                }
                else
                {
                    *os << "{type:" << static_cast<std::size_t>(node.type) << " start:" << node.start << " end:" << node.end << "}";
                }
            },
            node.children);
    }

    struct TestExpressionParser : testing::Test
    {
        ExpressionParser parser;
    };

    TEST_F(TestExpressionParser, Construct)
    {
        EXPECT_THAT(parser.Parse(""), testing::Eq(Node{
                                          NodeType::expression,
                                          0,
                                          0,
                                          std::vector<Node>{},
                                      }));
    }

    TEST_F(TestExpressionParser, Text)
    {
        parser.Parse("This is a GIVEN step");
    }

    TEST_F(TestExpressionParser, Phrase)
    {
        EXPECT_THAT(parser.Parse("three blind mice"), testing::Eq(Node{
                                                          NodeType::expression,
                                                          0,
                                                          16,
                                                          std::vector<Node>{
                                                              Node{
                                                                  NodeType::text,
                                                                  0,
                                                                  5,
                                                                  "three",
                                                              },
                                                              Node{
                                                                  NodeType::text,
                                                                  5,
                                                                  6,
                                                                  " ",
                                                              },
                                                              Node{
                                                                  NodeType::text,
                                                                  6,
                                                                  11,
                                                                  "blind",
                                                              },
                                                              Node{
                                                                  NodeType::text,
                                                                  11,
                                                                  12,
                                                                  " ",
                                                              },
                                                              Node{
                                                                  NodeType::text,
                                                                  12,
                                                                  16,
                                                                  "mice",
                                                              },
                                                          },
                                                      }));
    }

    TEST_F(TestExpressionParser, Parameter)
    {
        EXPECT_THAT(parser.Parse("{string}"), testing::Eq(Node{
                                                  NodeType::expression,
                                                  0,
                                                  8,
                                                  std::vector<Node>{
                                                      Node{
                                                          NodeType::parameter,
                                                          0,
                                                          8,
                                                          std::vector<Node>{
                                                              Node{
                                                                  NodeType::text,
                                                                  1,
                                                                  7,
                                                                  "string",
                                                              },
                                                          },
                                                      },
                                                  },
                                              }));
    }

    TEST_F(TestExpressionParser, Alternation)
    {
        EXPECT_THAT(parser.Parse("mice/rats"), testing::Eq(Node{
                                                   NodeType::expression,
                                                   0,
                                                   9,
                                                   std::vector{
                                                       Node{
                                                           NodeType::alternation,
                                                           0,
                                                           9,
                                                           std::vector{
                                                               Node{
                                                                   NodeType::alternative,
                                                                   0,
                                                                   4,
                                                                   std::vector{
                                                                       Node{
                                                                           NodeType::text,
                                                                           0,
                                                                           4,
                                                                           "mice",
                                                                       },
                                                                   },
                                                               },
                                                               Node{
                                                                   NodeType::alternative,
                                                                   5,
                                                                   9,
                                                                   std::vector<Node>{
                                                                       Node{
                                                                           NodeType::text,
                                                                           5,
                                                                           9,
                                                                           "rats",
                                                                       },
                                                                   },
                                                               },
                                                           },
                                                       },
                                                   },
                                               }));
    }

    TEST_F(TestExpressionParser, AlternationPhrase)
    {
        EXPECT_THAT(parser.Parse("three hungry/blind mice"), testing::Eq(Node{
                                                                 NodeType::expression,
                                                                 0,
                                                                 23,
                                                                 std::vector{
                                                                     { NodeType::text, 0, 5, "three" },
                                                                     { NodeType::text, 5, 6, " " },
                                                                     Node{
                                                                         NodeType::alternation,
                                                                         6,
                                                                         18,
                                                                         std::vector{
                                                                             Node{
                                                                                 NodeType::alternative,
                                                                                 6,
                                                                                 12,
                                                                                 std::vector{
                                                                                     Node{ NodeType::text, 6, 12, "hungry" },
                                                                                 },
                                                                             },
                                                                             Node{
                                                                                 NodeType::alternative,
                                                                                 13,
                                                                                 18,
                                                                                 std::vector<Node>{
                                                                                     Node{ NodeType::text, 13, 18, "blind" },
                                                                                 },
                                                                             },
                                                                         },
                                                                     },
                                                                     { NodeType::text, 18, 19, " " },
                                                                     { NodeType::text, 19, 23, "mice" },
                                                                 },
                                                             }));
    }

    TEST_F(TestExpressionParser, Optional)
    {
        EXPECT_THAT(parser.Parse("(blind)"), testing::Eq(Node{
                                                 NodeType::expression,
                                                 0,
                                                 7,
                                                 std::vector<Node>{
                                                     Node{
                                                         NodeType::optional,
                                                         0,
                                                         7,
                                                         std::vector<Node>{
                                                             Node{
                                                                 NodeType::text,
                                                                 1,
                                                                 6,
                                                                 "blind",
                                                             },
                                                         },
                                                     },
                                                 },
                                             }));
    }

    TEST_F(TestExpressionParser, OptionalPhrase)
    {
        EXPECT_THAT(parser.Parse("three (blind) mice"), testing::Eq(Node{
                                                            NodeType::expression,
                                                            0,
                                                            18,
                                                            std::vector<Node>{
                                                                Node{ NodeType::text, 0, 5, "three" },
                                                                Node{ NodeType::text, 5, 6, " " },
                                                                Node{
                                                                    NodeType::optional,
                                                                    6,
                                                                    13,
                                                                    std::vector<Node>{
                                                                        Node{ NodeType::text, 7, 12, "blind" },
                                                                    },
                                                                },
                                                                Node{ NodeType::text, 13, 14, " " },
                                                                Node{ NodeType::text, 14, 18, "mice" },
                                                            },
                                                        }));
    }

    TEST_F(TestExpressionParser, AlternationFollowedByOptional)
    {
        auto expected = Node{
            NodeType::expression,
            0,
            23,
            std::vector<Node>{
                Node{ NodeType::text, 0, 5, "three" },
                Node{ NodeType::text, 5, 6, " " },
                Node{
                    NodeType::alternation,
                    6,
                    23,
                    std::vector<Node>{
                        Node{
                            NodeType::alternative,
                            6,
                            16,
                            std::vector<Node>{
                                Node{ NodeType::text, 6, 16, "blind rat" },
                            },
                        },
                        Node{
                            NodeType::alternative,
                            17,
                            23,
                            std::vector<Node>{
                                Node{ NodeType::text, 17, 20, "cat" },
                                Node{
                                    NodeType::optional,
                                    20,
                                    23,
                                    std::vector<Node>{
                                        Node{ NodeType::text, 21, 22, "s" },
                                    },
                                },
                            },
                        },
                    },
                },
            },
        };
        EXPECT_THAT(parser.Parse("three blind\\ rat/cat(s)"), testing::Eq(expected));
    }

    TEST_F(TestExpressionParser, AlternationWithParameter)
    {
        auto expected = Node{ NodeType::expression, 0, 29, std::vector<Node>{
                                                               Node{ NodeType::text, 0, 1, "I" },
                                                               Node{ NodeType::text, 1, 2, " " },
                                                               Node{ NodeType::text, 2, 8, "select" },
                                                               Node{ NodeType::text, 8, 9, " " },
                                                               Node{ NodeType::text, 9, 12, "the" },
                                                               Node{ NodeType::text, 12, 13, " " },
                                                               Node{ NodeType::parameter, 13, 18, std::vector<Node>{
                                                                                                      Node{ NodeType::text, 14, 17, "int" },
                                                                                                  } },
                                                               Node{ NodeType::alternation, 18, 29, std::vector<Node>{
                                                                                                        Node{
                                                                                                            NodeType::alternative,
                                                                                                            18,
                                                                                                            20,
                                                                                                            std::vector<Node>{
                                                                                                                Node{ NodeType::text, 18, 20, "st" },
                                                                                                            },
                                                                                                        },
                                                                                                        Node{
                                                                                                            NodeType::alternative,
                                                                                                            21,
                                                                                                            23,
                                                                                                            std::vector<Node>{
                                                                                                                Node{ NodeType::text, 21, 23, "nd" },
                                                                                                            },
                                                                                                        },
                                                                                                        Node{
                                                                                                            NodeType::alternative,
                                                                                                            24,
                                                                                                            26,
                                                                                                            std::vector<Node>{
                                                                                                                Node{ NodeType::text, 24, 26, "rd" },
                                                                                                            },
                                                                                                        },
                                                                                                        Node{
                                                                                                            NodeType::alternative,
                                                                                                            27,
                                                                                                            29,
                                                                                                            std::vector<Node>{
                                                                                                                Node{ NodeType::text, 27, 29, "th" },
                                                                                                            },
                                                                                                        },
                                                                                                    } },
                                                           } };
        EXPECT_THAT(parser.Parse("I select the {int}st/nd/rd/th"), testing::Eq(expected));
    }

    TEST_F(TestExpressionParser, OptionalContainingNestedOptional)
    {
        auto expected = Node{ NodeType::expression, 0, 25, std::vector{
                                                               Node{ NodeType::text, 0, 5, "three" },
                                                               Node{ NodeType::text, 5, 6, " " },
                                                               Node{ NodeType::optional, 6, 20, std::vector{
                                                                                                    Node{ NodeType::optional, 7, 13, std::vector{
                                                                                                                                         Node{ NodeType::text, 8, 12, "very" },
                                                                                                                                     } },
                                                                                                    Node{ NodeType::text, 13, 14, " " },
                                                                                                    Node{ NodeType::text, 14, 19, "blind" },
                                                                                                } },
                                                               Node{ NodeType::text, 20, 21, " " },
                                                               Node{ NodeType::text, 21, 25, "mice" },
                                                           } };

        EXPECT_THAT(parser.Parse("three ((very) blind) mice"), testing::Eq(expected));
    }

    TEST_F(TestExpressionParser, UnfinishedParameter)
    {
        EXPECT_THROW(parser.Parse("{string"), MissingEndToken);
    }

    TEST_F(TestExpressionParser, AlternationWithUnusedStartOptional)
    {
        EXPECT_THROW(parser.Parse(R"(three blind\ mice/rats()"), MissingEndToken);
    }

    TEST_F(TestExpressionParser, AlternationInOptional)
    {
        EXPECT_THROW(parser.Parse(R"(three blind (mice/rats)))"), AlternationNotAllowedInOptional);
    }
}
