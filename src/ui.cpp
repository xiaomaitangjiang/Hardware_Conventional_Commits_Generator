#include "ui.hpp"
#include "formatter.hpp"

#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>

#include <cstdio>
#include <cassert>
using namespace ftxui;

namespace hwcommit {

auto BuildCommitMessage(const UIState& s,
                        const TypeRegistry& types,
                        const ScopeRegistry& scopes) -> CommitMessage {
    CommitBuilder builder;
    auto type_keys  = types.Keys();
    auto scope_keys = scopes.Keys();

    if (s.type_index >= 0 && s.type_index < (int)type_keys.size())
        builder.SetType(type_keys[s.type_index]);
    if (s.scope_index >= 0 && s.scope_index < (int)scope_keys.size())
        builder.SetScope(scope_keys[s.scope_index]);

    if (!s.module.empty())
        builder.SetModule(s.module);
    if (!s.description.empty())
        builder.SetDescription(s.description);
    if (!s.body.empty())
        builder.SetBody(s.body);

    for (const auto& slot : s.footer_slots) {
        if (slot.key_index > 0 && slot.key_index < (int)kFooterKeys.size()
            && !slot.value.empty()) {
            builder.AddFooter(kFooterKeys[slot.key_index], slot.value);
        }
    }

    return builder.Build();
}

// ---- Component Factories ----

Component MakeTypeSelector(UIState& s, const TypeRegistry& /*reg*/) {
    return Toggle(&s.type_entries, &s.type_index);
}

Component MakeScopeSelector(UIState& s, const ScopeRegistry& /*reg*/) {
    return Toggle(&s.scope_entries, &s.scope_index);
}

Component MakeModuleInput(UIState& s) {
    return Input(&s.module, "module name (without brackets)");
}

Component MakeDescriptionInput(UIState& s) {
    return Input(&s.description, "enter commit description");
}

Component MakeBodyInput(UIState& s) {
    InputOption option;
    option.multiline = true;
    return Input(&s.body, "enter body (optional, Ctrl+J for newline)", std::move(option));
}

// Each slot is a Container(Horizontal) wrapped in a Renderer that lays out
// the toggle and input vertically so the key toggle has room to breathe.
Components MakeFooterInput(UIState& s) {
    static std::vector<std::string> footer_keys = {
        "None", "BREAKING CHANGE", "Closes", "Affects", "Resource", "Timing"
    };
    // Create raw interactive components once, shared across slots
    std::vector<Component> key_toggles;
    std::vector<Component> value_inputs;
    for (int i = 0; i < kMaxFooterSlots; ++i) {
        key_toggles.push_back(
            Toggle(&footer_keys, &s.footer_slots[i].key_index));
        value_inputs.push_back(
            Input(&s.footer_slots[i].value, "enter value"));
    }

    Components children;
    for (int i = 0; i < kMaxFooterSlots; ++i) {
        // Event routing: both components receive events
        auto event_container = Container::Horizontal({
            key_toggles[i], value_inputs[i]});
        // Capture Component (shared_ptr) by value so the lambda owns a
        // live reference after key_toggles / value_inputs go out of scope.
        auto kt    = key_toggles[i];
        auto vi    = value_inputs[i];
        children.push_back(Renderer(event_container, [&s, kt, vi,i] {
            bool disabled = s.footer_slots[i].key_index == 0;
            auto key_row = kt->Render();
            auto val_row = hbox({
                text("  Value: "),
                vi->Render() | flex,
            });
            if (disabled) {
                key_row = key_row | dim;
                val_row = val_row | dim;
            }
            return vbox({key_row, val_row});
        }));
    }
    return children;
}

// Clipboard helper — best-effort, silently ignored if clip.exe unavailable.
static void CopyToClipboard(const std::string& text) {
#ifdef _WIN32
    FILE* clip = _popen("clip", "w");
    if (clip != nullptr) {
        std::ignore = fwrite(text.c_str(), 1, text.size(), clip);
        _pclose(clip);
    }
#endif
}

Component MakeActionBar(UIState& s, ScreenInteractive& screen,
                        const TypeRegistry& types,
                        const ScopeRegistry& scopes) {
    auto generate_btn = Button("Generate & Copy", [&] {
        s.generated_message = FormatCommitMessage(
            BuildCommitMessage(s, types, scopes));
        CopyToClipboard(s.generated_message);
        s.show_status    = true;
        s.status_message = "Copied to clipboard!";
    });

    auto reset_btn = Button("Reset", [&] {
        s.type_index  = 0;
        s.scope_index = 0;
        s.module.clear();
        s.description.clear();
        s.body.clear();
        for (auto& slot : s.footer_slots) {
            slot.key_index = 0;
            slot.value.clear();
        }
        s.show_status     = false;
        s.generated_message.clear();
    });

    auto quit_btn = Button("Quit", [&] {
        s.should_quit = true;
        screen.Exit();
    });

    return Container::Horizontal({generate_btn, reset_btn, quit_btn});
}

// ---- Main TUI Entry ----

auto RunTUI(TypeRegistry& types, ScopeRegistry& scopes) -> std::string {
    UIState state;

    // Populate display entries from registries
    for (const auto& [id, type] : types.Items())
        state.type_entries.push_back(type.name);
    for (const auto& [id, scope] : scopes.Items())
        state.scope_entries.push_back(scope.name);

    auto screen = ScreenInteractive::Fullscreen();

    // Create top-level components
    auto type_selector  = MakeTypeSelector(state, types);
    auto scope_selector = MakeScopeSelector(state, scopes);
    auto module_input   = MakeModuleInput(state);
    auto desc_input     = MakeDescriptionInput(state);
    auto body_input     = MakeBodyInput(state);
    auto footer_inputs  = MakeFooterInput(state);
    auto action_bar     = MakeActionBar(state, screen, types, scopes);

    // Compose event-routing container
    std::vector<Component> children = {
        type_selector,
        scope_selector,
        module_input,
        desc_input,
        body_input,
    };
    children.insert(children.end(), footer_inputs.begin(), footer_inputs.end());
    children.push_back(action_bar);
    auto main_container = Container::Vertical(std::move(children));

    // Renderer decouples layout (Element tree) from event routing (Container).
    // Each frame, the UI is rebuilt from UIState — the single source of truth.
    auto main_renderer = Renderer(main_container, [&] {
        auto msg     = BuildCommitMessage(state, types, scopes);
        auto preview = FormatCommitMessage(msg);

        auto desc_len = state.description.size();
        auto over = desc_len > 72;
        auto counter_color = over ? Color::Red : Color::White;
        auto counter = text(" " + std::to_string(desc_len) + "/72")
                     | color(counter_color);

        Elements rows;

        // Title
        rows.push_back(text(" Hardware Conventional Commits Generator ")
                       | bold | center);
        rows.push_back(separator());

        // Type / Scope
        rows.push_back(hbox({text(" Type:        "),
                             type_selector->Render() | flex}));
        rows.push_back(hbox({text(" Scope:       "),
                             scope_selector->Render() | flex}));

        // Module
        rows.push_back(hbox({text(" Module:      "),
                             module_input->Render() | flex}));

        // Description
        rows.push_back(hbox({text(" Description: "),
                             desc_input->Render() | flex, counter}));
        if (!state.description.empty() && over) {
            rows.push_back(text("  ! Description exceeds 72 characters")
                           | color(Color::Red) | bold);
        }

        rows.push_back(separator());

        // Body
        rows.push_back(text(" Body (optional):") | bold);
        rows.push_back(body_input->Render()
                       | border | size(HEIGHT, GREATER_THAN, 3));

        rows.push_back(separator());

        // Footers — each slot renders itself (key row + value row)
        rows.push_back(text(" Footers:") | bold);
        for (auto& fc : footer_inputs) {
            rows.push_back(fc->Render());
        }

        rows.push_back(separator());

        // Preview
        rows.push_back(text(" Preview:") | bold | color(Color::Cyan));
        rows.push_back(paragraph(preview)
                       | border | size(HEIGHT, GREATER_THAN, 4));

        // Status banner
        if (state.show_status) {
            rows.push_back(separator());
            rows.push_back(text("  " + state.status_message + "  ")
                           | color(Color::Green) | bold | center);
        }

        rows.push_back(separator());

        // Bottom bar: keyboard hints + buttons
        rows.push_back(hbox({
            text(" Tab: nav  |  Enter: toggle  |  ")
                | dim | center | flex,
            action_bar->Render(),
        }));

        return vbox(std::move(rows)) | border | size(WIDTH, GREATER_THAN, 78);
    });

    screen.Loop(main_renderer);

    return state.generated_message;
}

} // namespace hwcommit
