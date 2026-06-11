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

#include <utility>

namespace carla {
namespace ros2 {

struct CarlaOdometryMsgTraits {
  using msg_type = nav_msgs::msg::Odometry;
  using msg_pubsub_type = nav_msgs::msg::OdometryPubSubType;
};

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
  constexpr float to_meters = 1e-2f;
  const auto transform = TransformFromCarlaRotation(
      tx * to_meters, ty * to_meters, tz * to_meters,
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

  message->twist().twist().linear().x(linear_x);
  message->twist().twist().linear().y(-linear_y);
  message->twist().twist().linear().z(linear_z);
  message->twist().twist().angular().x(angular_x);
  message->twist().twist().angular().y(-angular_y);
  message->twist().twist().angular().z(angular_z);
  return true;
}

}  // namespace ros2
}  // namespace carla
