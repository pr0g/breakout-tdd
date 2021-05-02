#pragma once

#include <utility>

struct display_t {
  virtual void output(int x, int y) = 0;

protected:
  ~display_t() = default;
};

class breakout_t {
  std::pair<int, int> board_size_;
  std::pair<int, int> board_offset_;
  std::pair<int, int> paddle_position_;
  std::pair<int, int> paddle_size_;

public:
  int score() { return 0; }
  void setup(int x, int y, int width, int height) {
    board_size_ = {width, height};
    board_offset_ = {x, y};
    paddle_position_ = {width / 2, height - 1};
    paddle_size_ = {10, 1}; // default size
  }

  std::pair<int, int> board_offset() const { return board_offset_; }
  std::pair<int, int> board_size() const { return board_size_; }
  std::pair<int, int> paddle_position() const { return paddle_position_; }
  std::pair<int, int> paddle_size() const { return paddle_size_; }

  void display_board(display_t& display) {
    const auto [board_width, board_height] = board_size_;
    const auto [board_x, board_y] = board_offset_;

    for (int x = board_x; x <= board_x + board_width; x++) {
      display.output(x, board_y);
      display.output(x, board_y + board_height);
    }
    for (int y = board_y + 1; y <= board_y + board_height - 1; y++) {
      display.output(board_x, y);
      display.output(board_x + board_width, y);
    }
  }
};
