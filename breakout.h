#pragma once

#include <utility>

struct display_t {
  virtual void output(int x, int y) = 0;

protected:
  ~display_t() = default;
};

class breakout_t {
  int x_;
  int y_;
  int width_;
  int height_;
  std::pair<int, int> paddle_position_;
public:
  int score() { return 0; }
  void setup(int x, int y, int width, int height) {
    x_ = x;
    y_ = y;
    width_ = width;
    height_ = height;
    paddle_position_ = {width_ / 2, height_ - 1};
  }

  std::pair<int, int> board_offset() const { return {x_, y_}; }
  std::pair<int, int> board_size() const { return {width_, height_}; }
  std::pair<int, int> paddle_position() const { return paddle_position_; }

  void display(display_t& display) {
    for (int x = x_; x <= x_ + width_; x++) {
      display.output(x, y_);
      display.output(x, y_ + height_);
    }
    for (int y = y_ + 1; y <= y_ + height_ - 1; y++) {
      display.output(x_, y);
      display.output(x_ + width_, y);
    }
  }
};
