#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "breakout.h"

TEST_CASE("breakout game") {
  breakout_t breakout;

  SUBCASE("initial score is zero") { CHECK(breakout.score() == 0); }
}
