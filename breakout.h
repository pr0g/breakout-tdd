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
  std::pair<int, int> ball_position_;
  std::pair<int, int> ball_velocity_;
  bool launched_ = false;

  void try_move_ball() {
    if (!launched_) {
      ball_position_.first = paddle_position_.first;
    }
  }

  void launch(std::pair<int, int> velocity) {
    launched_ = true;
    ball_velocity_ = velocity;
  }

public:
  [[nodiscard]] int score() const { return 0; }
  void setup(int x, int y, int width, int height) {
    board_size_ = {width, height};
    board_offset_ = {x, y};
    paddle_position_ = {width / 2, height - 1};
    paddle_size_ = {10, 1}; // default size
    ball_position_ = {paddle_position_.first, paddle_position_.second - 1};
    ball_velocity_ = {0, 0};
  }

  std::pair<int, int> board_offset() const { return board_offset_; }
  std::pair<int, int> board_size() const { return board_size_; }
  std::pair<int, int> paddle_position() const { return paddle_position_; }
  std::pair<int, int> paddle_size() const { return paddle_size_; }
  std::pair<int, int> ball_position() const { return ball_position_; }
  std::pair<int, int> ball_velocity() const { return ball_velocity_; }

  int blocks_horizontal() const { return 11; }
  int blocks_vertical() const { return 9; }
  int block_width() const { return 8; }
  int block_height() const { return 1; }
  int vertical_padding() const { return 1; }
  int horizontal_padding() const { return 2; }
  int horizontal_spacing() const { return 1; }
  int vertical_spacing() const { return 1; }

  int paddle_left_edge() const {
    return paddle_position().first - paddle_size().first / 2;
  }
  int paddle_right_edge() const {
    return paddle_position().first + (paddle_size().first / 2 - 1);
  }

  void launch_left() { launch({-1, -1}); }

  void launch_right() { launch({1, -1}); }

  void move_paddle_left(const int distance) {
    int move = std::min(paddle_left_edge() - 1, distance);
    if (paddle_left_edge() > 1) {
      paddle_position_.first -= move;
    }
    try_move_ball();
  }

  void move_paddle_right(const int distance) {
    int move = std::min(board_size_.first - paddle_right_edge() - 1, distance);
    if (paddle_right_edge() < board_size_.first) {
      paddle_position_.first += move;
    }
    try_move_ball();
  }

  void step() {
    if (launched_) {
      ball_position_.first += ball_velocity_.first;
      ball_position_.second += ball_velocity_.second;
      if (
        ball_position_.first >= board_size_.first - 1
        || ball_position_.first <= 1) {
        ball_velocity_.first *= -1;
      }
      if (ball_position_.second <= 1) {
        ball_velocity_.second *= -1;
      }
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

  void display_blocks(display_t& display) {
    const auto [board_x, board_y] = board_offset();
    for (int row = 0; row < blocks_vertical(); ++row) {
      for (int col = 0; col < blocks_horizontal(); ++col) {
        for (int block_part = 0; block_part < block_width(); ++block_part) {
          display.output(
            board_x + horizontal_padding() + block_part
              + ((block_width() + horizontal_spacing()) * col),
            board_y + vertical_padding()
              + ((block_height() + vertical_spacing()) * row));
        }
      }
    }
  }

  void display_ball(display_t& display) {
    const auto [x, y] = ball_position_;
    const auto [board_x, board_y] = board_offset();
    display.output(board_x + x, board_y + y);
  }
};
