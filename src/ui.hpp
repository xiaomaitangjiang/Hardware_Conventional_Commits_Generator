#pragma once
#include <memory>
#include <string>
#include <vector>

#include "types.hpp"

// FTXUI's Component is a shared_ptr alias, not a class — forward-declared as such
// to avoid pulling in the full FTXUI headers in this lightweight header.
namespace ftxui {
class ComponentBase;
using Component = std::shared_ptr<ComponentBase>;
class ScreenInteractive;
} // namespace ftxui

namespace hwcommit {

inline const std::vector<std::string> kFooterKeys = {
    "None",
    "BREAKING CHANGE",
    "Closes",
    "Affects",
    "Resource",
    "Timing"
};

inline constexpr int kMaxFooterSlots = 5;

struct UIState {
    // Display entries (built from registry at startup)
    std::vector<std::string> type_entries;
    std::vector<std::string> scope_entries;

    // Selection indices
    int type_index  = 0;
    int scope_index = 0;

    // Text inputs
    std::string module;
    std::string description;
    std::string body;

    // Footer slots: key_index 0 = "None" (slot disabled, excluded from output).
    struct FooterSlot {
        int key_index = 0;
        std::string value;
    };
    std::vector<FooterSlot> footer_slots;

    // Action flags
    bool should_quit     = false;
    bool show_status     = false;
    std::string status_message;
    std::string generated_message;

    UIState() : footer_slots(kMaxFooterSlots) {}
};

// Component factories
ftxui::Component MakeTypeSelector(UIState& s, const TypeRegistry& reg);
ftxui::Component MakeScopeSelector(UIState& s, const ScopeRegistry& reg);
ftxui::Component MakeModuleInput(UIState& s);
ftxui::Component MakeDescriptionInput(UIState& s);
ftxui::Component MakeBodyInput(UIState& s);
ftxui::Component MakeActionBar(UIState& s, ftxui::ScreenInteractive& screen,
                               const TypeRegistry& types,
                               const ScopeRegistry& scopes);

// Build CommitMessage from UI state
auto BuildCommitMessage(const UIState& s,
                        const TypeRegistry& types,
                        const ScopeRegistry& scopes) -> CommitMessage;

// Main entry — runs the TUI, returns the last generated message (empty if none)
auto RunTUI(TypeRegistry& types, ScopeRegistry& scopes) -> std::string;

} // namespace hwcommit
