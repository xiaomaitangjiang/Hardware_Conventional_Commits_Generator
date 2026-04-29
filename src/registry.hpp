#pragma once
#include <vector>
#include <optional>
#include <span>
#include <algorithm>
#include <ranges>

// OCP foundation: new types/scopes added via Register() without modifying this class.
template <typename K, typename V>
class Registry {
public:
    void Register(K key, V value) {
        items_.emplace_back(std::move(key), std::move(value));
    }

    auto Get(const K& key) const -> std::optional<V> {
        auto it = std::ranges::find_if(items_,
            [&key](const auto& pair) { return pair.first == key; });
        if (it != items_.end()) return it->second;
        return std::nullopt;
    }

    auto Items() const -> std::span<const std::pair<K, V>> {
        return items_;
    }

    auto Keys() const -> std::vector<K> {
        auto rv = items_ | std::views::keys;
        return {rv.begin(), rv.end()};
    }

    auto Size() const -> size_t { return items_.size(); }

private:
    std::vector<std::pair<K, V>> items_;
};
