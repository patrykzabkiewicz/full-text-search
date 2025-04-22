#include <array>
#include <iostream>
#include <ranges>

int main() {
    constexpr static std::array data{1, 2, 3, 4, 5, 6};
    constexpr int window{3};

    auto Fun = [](auto... ints) { return (... + ints); };

    constexpr auto view = data | std::views::adjacent_transform<window>(Fun);

    for (int x : view)
        std::cout << x << ' ';
    std::cout << '\n';
}
