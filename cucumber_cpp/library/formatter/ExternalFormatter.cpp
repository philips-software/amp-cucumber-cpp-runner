#include "cucumber_cpp/library/formatter/ExternalFormatter.hpp"
#include "cucumber/messages/Envelope.hpp"
#include "cucumber/pretty-formatter/Formatter.hpp"
#include "cucumber/pretty-formatter/PrettyPrinter.hpp"
#include "cucumber/pretty-formatter/ProgressBarPrinter.hpp"
#include "cucumber/pretty-formatter/ProgressPrinter.hpp"
#include "cucumber/pretty-formatter/SummaryPrinter.hpp"
#include "cucumber/pretty-formatter/Theme.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include "nlohmann/json.hpp"
#include <cstddef>
#include <memory>
#include <ostream>
#include <set>
#include <string>
#include <utility>

namespace cucumber_cpp::library::formatter
{
    namespace
    {
        constexpr std::size_t defaultMaxWidth{ 80 };

        std::shared_ptr<cucumber::pretty_formatter::Theme> MakeTheme(const nlohmann::json& options)
        {
            const auto theme = options.value("theme", std::string{ "cucumber" });

            if (theme == "plain")
                return cucumber::pretty_formatter::Theme::Plain();
            if (theme == "none")
                return cucumber::pretty_formatter::Theme::None();
            return cucumber::pretty_formatter::Theme::Cucumber();
        }

        // Owns the Tty so its lifetime spans the ProgressBarPrinter.
        struct ProgressBarFormatter
            : cucumber::pretty_formatter::Formatter
        {
            ProgressBarFormatter(std::ostream& output, std::shared_ptr<cucumber::pretty_formatter::Theme> theme, std::set<cucumber::pretty_formatter::ProgressBarPrinter::Options> options)
                : tty{ output }
                , printer{ tty, std::move(theme), defaultMaxWidth, [](std::string uri)
                    {
                        return uri;
                    },
                    std::move(options) }
            {}

            void Update(const cucumber::messages::Envelope& envelope) override
            {
                printer.Update(envelope);
            }

        private:
            cucumber::pretty_formatter::ProgressBarPrinter::TtyOstream tty;
            cucumber::pretty_formatter::ProgressBarPrinter printer;
        };
    }

    ExternalFormatter::ExternalFormatter(util::Broadcaster& broadcaster, std::unique_ptr<cucumber::pretty_formatter::Formatter> formatter)
        : util::Listener{ broadcaster, [this](const cucumber::messages::Envelope& envelope)
            {
                this->formatter->Update(envelope);
            } }
        , formatter{ std::move(formatter) }
    {}

    std::unique_ptr<cucumber::pretty_formatter::Formatter> MakePrettyPrinter(const nlohmann::json& options, std::ostream& output)
    {
        using Options = cucumber::pretty_formatter::PrettyPrinter::Options;

        cucumber::pretty_formatter::PrettyPrinter::Factory factory;
        factory.Theme(MakeTheme(options))
            .Options(Options::includeAttachments, options.value("include_attachments", true))
            .Options(Options::includeFeatureLine, options.value("include_feature_line", true))
            .Options(Options::includeRuleLine, options.value("include_rule_line", true))
            .Options(Options::useStatusIcon, options.value("use_status_icon", true));

        return factory.Build(output);
    }

    std::unique_ptr<cucumber::pretty_formatter::Formatter> MakeSummaryPrinter(const nlohmann::json& options, std::ostream& output)
    {
        cucumber::pretty_formatter::SummaryPrinter::Factory factory;
        factory.Theme(MakeTheme(options))
            .Options(cucumber::pretty_formatter::SummaryPrinter::Options::includeAttachments, options.value("include_attachments", true));

        return factory.Build(output);
    }

    std::unique_ptr<cucumber::pretty_formatter::Formatter> MakeProgressPrinter(const nlohmann::json& options, std::ostream& output)
    {
        return std::make_unique<cucumber::pretty_formatter::ProgressPrinter>(output, MakeTheme(options), defaultMaxWidth);
    }

    std::unique_ptr<cucumber::pretty_formatter::Formatter> MakeProgressBarPrinter(const nlohmann::json& options, std::ostream& output)
    {
        std::set<cucumber::pretty_formatter::ProgressBarPrinter::Options> progressBarOptions;
        if (options.value("include_attachments", true))
            progressBarOptions.insert(cucumber::pretty_formatter::ProgressBarPrinter::Options::includeAttachments);

        return std::make_unique<ProgressBarFormatter>(output, MakeTheme(options), std::move(progressBarOptions));
    }
}
