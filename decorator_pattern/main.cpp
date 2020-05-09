#include <iostream>
#include <range/v3/view/filter.hpp>
#include <range/v3/view/single.hpp>
#include <range/v3/view/zip_with.hpp>
#include <vector>

int main() {

  auto const v1 = std::vector<int>{5, 2, 7};
  auto const v2 = ranges::view::single(9);
  auto rng =
      ranges::views::zip_with([](int a, int b) { return a & b; }, v1, v2);

  std::cout << rng << std::endl;
}
