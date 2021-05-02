#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "breakout.h"

#include <vector>
#include <numeric>

TEST_CASE("breakout game") {
  breakout_t breakout;

  int test_x = 10;
  int test_y = 5;
  int test_width = 100;
  int test_height = 80;
  breakout.setup(test_x, test_y, test_width, test_height);

  SUBCASE("initial score is zero") { CHECK(breakout.score() == 0); }
  SUBCASE("board play area") {
    const auto [x, y] = breakout.board_offset();
    CHECK(x == 10);
    CHECK(y == 5);

    const auto [width, height] = breakout.board_size();
    CHECK(width == 100);
    CHECK(height == 80);
  }

  SUBCASE("board outline displayed") {
    struct display_test_t : public display_t {
      std::vector<std::pair<int, int>> positions_;
      void output(int x, int y) override {
        positions_.push_back({x, y});
      }
    };

    display_test_t display_test;
    breakout.display(display_test);

    // 100x2 + 78x2 = 556
    CHECK(display_test.positions_.size() == 360);

    int x_max = 0;
    int x_min = INT_MAX;
    int y_max = 0;
    int y_min = INT_MAX;
    for (const auto& position : display_test.positions_) {
      x_max = std::max(position.first, x_max);
      x_min = std::min(position.first, x_min);
      y_max = std::max(position.second, y_max);
      y_min = std::min(position.second, y_min);
    }

    CHECK(x_min == 10);
    CHECK(x_max == 110);
    CHECK(y_min == 5);
    CHECK(y_max == 85);
  }
}
