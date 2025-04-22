#include <array>
#include <cassert>
#include <iostream>
#include <ranges>
#include <set>
#include <string>
#include <string_view>

using namespace std::string_view_literals;
using iterator = std::multiset<std::string>::iterator;

namespace vw = std::views;

using ngram_t = std::array<char, 3>;
constexpr int window{3};

auto into_ngram = [](auto... c) {
    return ngram_t{c...}; // just a type-safe array
};

constexpr auto ngrams_of(std::string_view input) { return input | vw::adjacent_transform<window>(into_ngram) | vw::enumerate; }

int main() {
    for (auto [x, y] : ngrams_of("expression"sv))
        std::cout << x << ": " << std::string(y.begin(), y.end()) << ' ';

    std::cout << '\n';
}