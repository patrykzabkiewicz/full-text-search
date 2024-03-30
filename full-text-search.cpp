/**
   full text search in c++20
   */

#include <string>
#include <iostream>
#include <ranges>
#include <string_view>
#include <map>
#include <set>

using namespace rng = std::ranges;
using namespace vw = std::views;

constexpr auto ngrams_of(std::string_view input) {
  return input | vw::adjacent_transform<3>(into_ngram) | vw::enumerate;
}

constexpr auto into_ngram(auto... c) {
  return ngram_t{c...}; // just a type-safe array
}

struct fulltext {
  std::multi_set<std::string> entries;
  std::map<ngram_t, std::set<index_entry>> index;

  using iterator = std::multi_set<std::string>::iterator;

  struct index_entry {
    iterator entry;
    size_t offset;
  };

  auto insert(std::string_view) -> iterator {
    const auto [entry, _] = entries.emplace(content);

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

      samples->second.erase({entry, position});
    }

    entries.erase(entry);
  }

  // will provide list of all occurances across whole dataset
  auto search(std::string_view needle) const -> std::set<index_entry> {
    auto sets = ngrams_of(needle) | vw::transform(get_position_and_set);
    return rng::fold_left_first(sets, intersect).set;
  }

  // will take a result from `enumerate` and convert it to relative position
  // from needle start and `set` containing all samples of the ngram
  auto get_position_and_set(std::tuple<size_t, ngram_t> in) {
    const auto [position, ngram] = in;
    const auto samples = index.find(ngram);

    if (samples == index.end()) {
      return position_and_set{position, {}};
    }

    return position_and_set{position, *samples};
  }

  struct position_and_set {
    size_t offset;
    value_or_reference<std::set<index_entry>> set;
  };

  auto relative(const position_and_set & samples) const noexcept {
    return samples.set | vw::transform([&samples](index_entry r) {
      // align position relatively to start of needle
      return index_entry{r.entry, r.position - samples.offset};
    });
  }

  // calculate intersection of two sets if ngrams are on the same relative position
  auto intersect(const postition_and_set & lhs, const position_and_set & rhs) {
    std::set<index_entry> result{};
    rng::set_intersection(lhs.relative(), rhs.relative(), emplace_iterator{result});
    return position_and_set{0, std::move(result)};
  }
};



int main(void) {

  auto ngrams = get_ngrams("regexp"sv) | vw::transform(get_position_and_set);

  for (const auto ngram : ngrams) std::cout << ngram << " ";
  std::cout << "\n";


  return 0;
}
