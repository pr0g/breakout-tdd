#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "breakout.h"

TEST_CASE("breakout game") {
  breakout_t breakout;

  SUBCASE("initial score is zero") { CHECK(breakout.score() == 0); }
  SUBCASE("board play area") {
    int test_x = 10;
    int test_y = 5;
    int test_width = 100;
    int test_height = 80;
    breakout.setup(test_x, test_y, test_width, test_height);

    const auto [x, y] = breakout.board_offset();
    CHECK(x == 10);
    CHECK(y == 5);

    const auto [width, height] = breakout.board_size();
    CHECK(width == 100);
    CHECK(height == 80);
  }
}
