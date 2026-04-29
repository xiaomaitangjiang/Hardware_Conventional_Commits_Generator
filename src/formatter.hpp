#pragma once
#include <string>
#include <sstream>
#include <algorithm>
#include "types.hpp"

namespace hwcommit {

inline auto FormatCommitMessage(const CommitMessage& msg) -> std::string {
    std::ostringstream oss;

    // Header: <type>(<scope>)<module>: <description>
    oss << msg.type << '(' << msg.scope << ')';
    if (!msg.module.empty())
        oss << '<' << msg.module << '>';
    oss << ": " << msg.description;

    // Body
    if (!msg.body.empty()) {
        // Trailing whitespace would produce a blank gap before the footer section.
        auto body = msg.body;
        auto end = body.find_last_not_of(" \t\n\r");
        if (end != std::string::npos)
            body.resize(end + 1);
        if (!body.empty())
            oss << "\n\n" << body;
    }

    // Footers
    for (const auto& [key, value] : msg.footers) {
        if (!value.empty())
            oss << "\n" << key << ": " << value;
    }

    return oss.str();
}

} // namespace hwcommit
