/**
 *  full text search in c++23
 */
#include <algorithm>
#include <array>
#include <cassert>
#include <iostream>
#include <map>
#include <ranges>
#include <set>
#include <string>
#include <string_view>
#include <utility>

using namespace std::string_view_literals;
using iterator = std::multiset<std::string>::iterator;

namespace rng = std::ranges;
namespace vw = std::views;

using ngram_t = std::array<char, 3>;
constexpr int window{3};

auto into_ngram = [](auto... c) {
    return ngram_t{c...}; // just a type-safe array
};

constexpr auto ngrams_of(std::string_view input) { return input | vw::adjacent_transform<window>(into_ngram) | vw::enumerate; }

struct index_entry {
    iterator entry;
    unsigned long position;

    friend bool operator<(const index_entry &l, const index_entry &r) { return l.position < r.position; }
};

struct position_and_set {
    size_t offset;
    std::set<index_entry> set;

    auto relative(const position_and_set &samples) const noexcept {
        return samples.set | vw::transform([&samples](index_entry r) {
                   // align position relatively to start of needle
                   return index_entry{r.entry, r.position - samples.offset};
               });
    }
};

struct full_text {
    std::multiset<std::string> entries;
    std::map<ngram_t, std::set<index_entry>> index;

    auto insert(std::string_view content) -> iterator {
        const auto entry = entries.emplace(content);

        for (const auto [position, ngram] : ngrams_of(content)) {
            const auto [samples, _] = index.emplace(ngram, std::set<index_entry>{});
            samples->second.emplace(entry, position);
        }

        return entry;
    }

    void remove(iterator entry) noexcept {
        for (const auto [position, ngram] : ngrams_of(*entry)) {
            const auto samples = index.find(ngram);
            assert(samples != index.end());

            samples->second.erase({entry, static_cast<unsigned long>(position)});
        }

        entries.erase(entry);
    }

    // will provide list of all occurances across whole dataset
    auto search(std::string_view needle) const -> std::set<index_entry> {

        // calculate intersection of two sets if ngrams are on the same relative
        // position
        auto intersect = [](const position_and_set &lhs, const position_and_set &rhs) {
            std::set<index_entry> result{};
            rng::set_intersection(lhs.relative(lhs), rhs.relative(rhs), std::back_inserter(result));
            return position_and_set{0, std::move(result)};
        };

        // will take a result from `enumerate` and convert it to relative position
        // from needle start and `set` containing all samples of the ngram
        auto get_position_and_set = [this](std::tuple<size_t, ngram_t> in) {
            const auto [position, ngram] = in;
            const auto samples = index.find(ngram);

            if (samples == index.end()) {
                return position_and_set{position, {}};
            }

            return position_and_set{position, samples->second};
        };

        auto sets = ngrams_of(needle) | vw::transform(get_position_and_set);
        return rng::fold_left_first(sets, intersect)->set;
    }
};

int main(void) {

    full_text ft;
    auto ngrams = ft.search("doooopa"sv);

    for (const auto [entry, position] : ngrams)
        std::cout << *entry << ": " << position << "; ";
    std::cout << "\n";
}
