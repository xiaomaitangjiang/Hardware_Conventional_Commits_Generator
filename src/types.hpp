#pragma once
#include <string>
#include <vector>
#include <utility>
#include "registry.hpp"

namespace hwcommit {

struct CommitType {
    std::string id;
    std::string name;
    std::string description;
};

struct CommitScope {
    std::string id;
    std::string name;
    std::string description;
};

struct CommitMessage {
    std::string type;
    std::string scope;
    std::string module;
    std::string description;
    std::string body;
    std::vector<std::pair<std::string, std::string>> footers;
};

using TypeRegistry  = Registry<std::string, CommitType>;
using ScopeRegistry = Registry<std::string, CommitScope>;

class CommitBuilder {
public:
    auto SetType(std::string id) -> CommitBuilder& {
        msg_.type = std::move(id);
        return *this;
    }
    auto SetScope(std::string id) -> CommitBuilder& {
        msg_.scope = std::move(id);
        return *this;
    }
    auto SetModule(std::string mod) -> CommitBuilder& {
        msg_.module = std::move(mod);
        return *this;
    }
    auto SetDescription(std::string desc) -> CommitBuilder& {
        msg_.description = std::move(desc);
        return *this;
    }
    auto SetBody(std::string body) -> CommitBuilder& {
        msg_.body = std::move(body);
        return *this;
    }
    auto AddFooter(std::string key, std::string value) -> CommitBuilder& {
        msg_.footers.emplace_back(std::move(key), std::move(value));
        return *this;
    }
    auto ClearFooters() -> CommitBuilder& {
        msg_.footers.clear();
        return *this;
    }
    auto Build() const -> CommitMessage { return msg_; }

private:
    CommitMessage msg_;
};

} // namespace hwcommit
