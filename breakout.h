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

  int paddle_left_edge() const {
    return paddle_position().first - paddle_size().first / 2;
  }
  int paddle_right_edge() const {
    return paddle_position().first + (paddle_size().first / 2 - 1);
  }

  void move_paddle_left(const int distance) {
    if (paddle_left_edge() - distance > 0) {
      paddle_position_.first -= distance;
    }
  }

  void move_paddle_right(const int distance) {
    if (paddle_right_edge() + distance < board_size_.first) {
      paddle_position_.first += distance;
    }
  }

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

  void display_paddle(display_t& display) {
    const auto [board_x, board_y] = board_offset_;
    const auto [paddle_x, paddle_y] = paddle_position();
    const auto [paddle_width, paddle_height] = paddle_size();

    for (int i = 0; i < paddle_width; ++i) {
      display.output(board_x + paddle_left_edge() + i, board_y + paddle_y);
    }
  }
};
