// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/ros2/publishers/TransformQuaternion.h>

#include <cmath>

namespace {
constexpr float kEps = 1e-5f;
constexpr float kSqrtHalf = 0.70710678118654752440f;  // sqrt(2)/2

void ExpectNearArray3(
    const std::array<float, 3> &actual,
    const std::array<float, 3> &expected,
    float eps = kEps) {
  EXPECT_NEAR(actual[0], expected[0], eps);
  EXPECT_NEAR(actual[1], expected[1], eps);
  EXPECT_NEAR(actual[2], expected[2], eps);
}

void ExpectNearArray4(
    const std::array<float, 4> &actual,
    const std::array<float, 4> &expected,
    float eps = kEps) {
  EXPECT_NEAR(actual[0], expected[0], eps);
  EXPECT_NEAR(actual[1], expected[1], eps);
  EXPECT_NEAR(actual[2], expected[2], eps);
  EXPECT_NEAR(actual[3], expected[3], eps);
}

// Where the frame's x (0), y (1) or z (2) axis points in the parent frame:
// that column of the rotation matrix of q = (w, x, y, z).
std::array<float, 3> Axis(const std::array<float, 4> &q, int i) {
  const float w = q[0], x = q[1], y = q[2], z = q[3];
  switch (i) {
    case 0: return {1 - 2 * (y * y + z * z), 2 * (x * y + z * w), 2 * (x * z - y * w)};
    case 1: return {2 * (x * y - z * w), 1 - 2 * (x * x + z * z), 2 * (y * z + x * w)};
    default: return {2 * (x * z + y * w), 2 * (y * z - x * w), 1 - 2 * (x * x + y * y)};
  }
}

}  // namespace

TEST(TransformQuaternion, identity_rotation_is_w_one) {
  const auto tf = carla::ros2::TransformFromCarlaRotation(
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f);
  ExpectNearArray3(tf.translation, {0.0f, 0.0f, 0.0f});
  ExpectNearArray4(tf.rotation, {1.0f, 0.0f, 0.0f, 0.0f});
}

TEST(TransformQuaternion, translation_y_axis_is_mirrored) {
  const auto tf = carla::ros2::TransformFromCarlaRotation(
      3.0f, 4.0f, 5.0f,
      0.0f, 0.0f, 0.0f);
  ExpectNearArray3(tf.translation, {3.0f, -4.0f, 5.0f});
}

TEST(TransformQuaternion, pure_pitch_ninety_lands_on_minus_y_half) {
  // CARLA pitch = +90 deg flips to rx = -pi/2; with all other components
  // identity the quaternion is (cos(-pi/4), 0, sin(-pi/4), 0).
  const auto tf = carla::ros2::TransformFromCarlaRotation(
      0.0f, 0.0f, 0.0f,
      90.0f, 0.0f, 0.0f);
  ExpectNearArray4(tf.rotation, {kSqrtHalf, 0.0f, -kSqrtHalf, 0.0f});
}

TEST(TransformQuaternion, pure_yaw_ninety_lands_on_minus_z_half) {
  // CARLA yaw = +90 deg flips to ry = -pi/2; the quaternion picks up z =
  // sin(-pi/4).
  const auto tf = carla::ros2::TransformFromCarlaRotation(
      0.0f, 0.0f, 0.0f,
      0.0f, 90.0f, 0.0f);
  ExpectNearArray4(tf.rotation, {kSqrtHalf, 0.0f, 0.0f, -kSqrtHalf});
}

TEST(TransformQuaternion, pure_roll_ninety_lands_on_plus_x_half) {
  // CARLA roll = +90 deg keeps the sign: rz = +pi/2, quaternion x = sin(pi/4).
  const auto tf = carla::ros2::TransformFromCarlaRotation(
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 90.0f);
  ExpectNearArray4(tf.rotation, {kSqrtHalf, kSqrtHalf, 0.0f, 0.0f});
}

TEST(TransformQuaternion, quaternion_stays_unit_norm) {
  for (float pitch : {-180.0f, -45.0f, 0.0f, 30.0f, 90.0f, 175.0f}) {
    for (float yaw : {-90.0f, 0.0f, 60.0f}) {
      for (float roll : {-30.0f, 0.0f, 45.0f}) {
        const auto tf = carla::ros2::TransformFromCarlaRotation(
            0.0f, 0.0f, 0.0f,
            pitch, yaw, roll);
        const float norm =
            tf.rotation[0] * tf.rotation[0]
            + tf.rotation[1] * tf.rotation[1]
            + tf.rotation[2] * tf.rotation[2]
            + tf.rotation[3] * tf.rotation[3];
        EXPECT_NEAR(norm, 1.0f, 1e-4f)
            << "pitch=" << pitch << " yaw=" << yaw << " roll=" << roll;
      }
    }
  }
}

TEST(TransformQuaternion, regression_guard_against_halved_deg_to_rad) {
  // The pre-d9cfc79fb spelling used M_PI_2 / 180 in place of M_PI / 180,
  // halving every rotation. A 180-degree pitch should land at quaternion
  // w=0 (pi/2 half-angle, cos(pi/2)=0); the buggy spelling would land at
  // w=cos(pi/4)=sqrt(2)/2. Guard against a regression to that spelling.
  const auto tf = carla::ros2::TransformFromCarlaRotation(
      0.0f, 0.0f, 0.0f,
      180.0f, 0.0f, 0.0f);
  EXPECT_NEAR(tf.rotation[0], 0.0f, 1e-3f);
  EXPECT_GT(std::abs(tf.rotation[2]), 0.9f);
}

TEST(TransformQuaternion, optical_frame_axes_point_right_down_forward) {
  // REP 103 optical frame of an unrotated camera: x right, y down, z forward,
  // i.e. -y, -z and +x of the x-forward, y-left, z-up sensor frame.
  const auto q = carla::ros2::OpticalFrameRotation({1.0f, 0.0f, 0.0f, 0.0f});
  ExpectNearArray4(q, {0.5f, -0.5f, 0.5f, -0.5f});
  ExpectNearArray3(Axis(q, 0), {0.0f, -1.0f, 0.0f});
  ExpectNearArray3(Axis(q, 1), {0.0f, 0.0f, -1.0f});
  ExpectNearArray3(Axis(q, 2), {1.0f, 0.0f, 0.0f});
}

TEST(TransformQuaternion, optical_frame_of_camera_pitched_down_looks_down) {
  // A camera pitched 20 deg down in CARLA looks forward and 20 deg below the
  // horizon, with the image x axis still pointing right.
  const auto tf = carla::ros2::TransformFromCarlaRotation(
      0.0f, 0.0f, 0.0f,
      -20.0f, 0.0f, 0.0f);
  const auto q = carla::ros2::OpticalFrameRotation(tf.rotation);
  const float pitch = 20.0f * static_cast<float>(M_PI) / 180.0f;
  ExpectNearArray3(Axis(q, 2), {std::cos(pitch), 0.0f, -std::sin(pitch)});
  ExpectNearArray3(Axis(q, 0), {0.0f, -1.0f, 0.0f});
}
