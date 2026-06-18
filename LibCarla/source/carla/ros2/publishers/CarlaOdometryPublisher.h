// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/ros2/publishers/BasePublisher.h"

#include <cstdint>
#include <memory>
#include <string>

namespace carla {
namespace ros2 {

template <typename Traits> class PublisherImpl;
struct CarlaOdometryMsgTraits;

class CarlaOdometryPublisher : public BasePublisher {
public:
  CarlaOdometryPublisher(std::string base_topic_name, std::string frame_id);
  ~CarlaOdometryPublisher() override;

  CarlaOdometryPublisher(const CarlaOdometryPublisher &) = delete;
  CarlaOdometryPublisher &operator=(const CarlaOdometryPublisher &) = delete;
  CarlaOdometryPublisher(CarlaOdometryPublisher &&) noexcept = default;
  CarlaOdometryPublisher &operator=(CarlaOdometryPublisher &&) noexcept = default;

  bool Publish() override;
  bool Write(
      std::int32_t seconds,
      std::uint32_t nanoseconds,
      const std::string &child_frame_id,
      float tx, float ty, float tz,
      float pitch_deg, float yaw_deg, float roll_deg,
      float linear_x, float linear_y, float linear_z,
      float angular_x, float angular_y, float angular_z);

private:
  std::shared_ptr<PublisherImpl<CarlaOdometryMsgTraits>> _impl;
};

}  // namespace ros2
}  // namespace carla
