// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/ros2/publishers/CarlaOdometryPublisher.h"

#include "carla/Logging.h"
#include "carla/ros2/publishers/PublisherImpl.h"
#include "carla/ros2/publishers/TransformQuaternion.h"
#include "carla/ros2/types/Odometry.h"
#include "carla/ros2/types/OdometryPubSubTypes.h"

#include <array>
#include <cmath>
#include <utility>

namespace carla {
namespace ros2 {

struct CarlaOdometryMsgTraits {
  using msg_type = nav_msgs::msg::Odometry;
  using msg_pubsub_type = nav_msgs::msg::OdometryPubSubType;
};

namespace {

constexpr float ToRadians(float degrees) {
  return degrees * (static_cast<float>(M_PI) / 180.0f);
}

std::array<float, 3> CarlaVectorToRosVectorRotated(
    float x, float y, float z,
    float pitch, float yaw, float roll) {
  // Mirrors carla_common.transforms.carla_vector_to_ros_vector_rotated().
  const float ros_pitch = ToRadians(-pitch);
  const float ros_yaw = ToRadians(-yaw);
  const float ros_roll = ToRadians(roll);

  const float cy = std::cos(ros_yaw);
  const float sy = std::sin(ros_yaw);
  const float cp = std::cos(ros_pitch);
  const float sp = std::sin(ros_pitch);
  const float cr = std::cos(ros_roll);
  const float sr = std::sin(ros_roll);

  const float rotated_x =
      x * (cy * cp) +
      y * (cy * sp * sr - sy * cr) +
      z * (cy * sp * cr + sy * sr);
  const float rotated_y =
      x * (sy * cp) +
      y * (sy * sp * sr + cy * cr) +
      z * (sy * sp * cr - cy * sr);
  const float rotated_z =
      x * (-sp) +
      y * (cp * sr) +
      z * (cp * cr);

  return {rotated_x, -rotated_y, rotated_z};
}

std::array<float, 3> CarlaAngularVelocityToRosVector(
    float angular_x, float angular_y, float angular_z) {
  return {ToRadians(angular_x), -ToRadians(angular_y), -ToRadians(angular_z)};
}

}  // namespace

CarlaOdometryPublisher::CarlaOdometryPublisher(
    std::string base_topic_name, std::string frame_id)
  : BasePublisher(std::move(base_topic_name), std::move(frame_id)),
    _impl(std::make_shared<PublisherImpl<CarlaOdometryMsgTraits>>()) {
  if (!_impl->Init(GetBaseTopicName())) {
    log_error("CarlaOdometryPublisher: failed to initialise writer for", GetBaseTopicName());
  }
}

CarlaOdometryPublisher::~CarlaOdometryPublisher() = default;

bool CarlaOdometryPublisher::Publish() {
  return _impl->Publish();
}

bool CarlaOdometryPublisher::Write(
    std::int32_t seconds,
    std::uint32_t nanoseconds,
    const std::string &child_frame_id,
    float tx, float ty, float tz,
    float pitch_deg, float yaw_deg, float roll_deg,
    float linear_x, float linear_y, float linear_z,
    float angular_x, float angular_y, float angular_z) {
  const auto transform = TransformFromCarlaRotation(
      tx, ty, tz,
      pitch_deg, yaw_deg, roll_deg);

  auto *message = _impl->GetMessage();
  message->header().stamp().sec(seconds);
  message->header().stamp().nanosec(nanoseconds);
  message->header().frame_id("carla_map");
  message->child_frame_id(child_frame_id.empty() ? GetFrameId() : child_frame_id);

  message->pose().pose().position().x(transform.translation[0]);
  message->pose().pose().position().y(transform.translation[1]);
  message->pose().pose().position().z(transform.translation[2]);
  message->pose().pose().orientation().w(transform.rotation[0]);
  message->pose().pose().orientation().x(transform.rotation[1]);
  message->pose().pose().orientation().y(transform.rotation[2]);
  message->pose().pose().orientation().z(transform.rotation[3]);

  const auto linear = CarlaVectorToRosVectorRotated(
      linear_x, linear_y, linear_z, pitch_deg, yaw_deg, roll_deg);
  const auto angular = CarlaAngularVelocityToRosVector(angular_x, angular_y, angular_z);
  message->twist().twist().linear().x(linear[0]);
  message->twist().twist().linear().y(linear[1]);
  message->twist().twist().linear().z(linear[2]);
  message->twist().twist().angular().x(angular[0]);
  message->twist().twist().angular().y(angular[1]);
  message->twist().twist().angular().z(angular[2]);
  return true;
}

}  // namespace ros2
}  // namespace carla
