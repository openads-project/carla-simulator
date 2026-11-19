// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/ros2/publishers/CarlaTransformPublisher.h"

#include "carla/Logging.h"
#include "carla/ros2/middleware/QosProfile.h"
#include "carla/ros2/publishers/PublisherImpl.h"
#include "carla/ros2/publishers/TransformQuaternion.h"
#include "carla/ros2/types/msg/TFMessage.h"
#include "carla/ros2/types/msg/TransformStamped.h"

#include <cmath>

namespace carla {
namespace ros2 {

struct CarlaTransformMsgTraits {
  using msg_type = msg::TFMessage;
};

namespace {

constexpr float kEpsilon = 1e-4f;

// A static transform is repeated once per second of simulation time. Latching
// alone does not reach every late joiner: the Zenoh middleware advertises
// TRANSIENT_LOCAL but does not replay the last sample.
constexpr double kStaticRepublishPeriod = 1.0;

bool IsSameTransform(
    const CarlaTransformPublisher::CachedTransform &cached,
    float tx, float ty, float tz,
    float pitch, float yaw, float roll) noexcept {
  return std::abs(cached.tx - tx) < kEpsilon
      && std::abs(cached.ty - ty) < kEpsilon
      && std::abs(cached.tz - tz) < kEpsilon
      && std::abs(cached.pitch - pitch) < kEpsilon
      && std::abs(cached.yaw - yaw) < kEpsilon
      && std::abs(cached.roll - roll) < kEpsilon;
}

}  // namespace

CarlaTransformPublisher::CarlaTransformPublisher(bool is_static)
  : BasePublisher(is_static ? "rt/tf_static" : "rt/tf"),
    _impl(std::make_shared<PublisherImpl<CarlaTransformMsgTraits>>()),
    _is_static(is_static) {
  // /tf_static is latched (RELIABLE / TRANSIENT_LOCAL / KEEP_LAST 1), the QoS
  // tf2 static listeners subscribe with.
  const bool initialised = _is_static
      ? _impl->Init(GetBaseTopicName(), QosProfile::ReliableTransientLocal())
      : _impl->Init(GetBaseTopicName());
  if (!initialised) {
    log_error(
        "CarlaTransformPublisher: failed to initialise writer for ",
        GetBaseTopicName());
  }
}

CarlaTransformPublisher::~CarlaTransformPublisher() = default;

bool CarlaTransformPublisher::Publish() {
  if (_is_static && !_static_publish_due) {
    return true;
  }
  const bool published = _impl->Publish();
  if (published && _is_static) {
    _has_published = true;
    _static_publish_due = false;
    _last_static_publish = _pending_static_time;
  }
  return published;
}

bool CarlaTransformPublisher::Write(
    std::int32_t seconds,
    std::uint32_t nanoseconds,
    const std::string &parent_frame_id,
    const std::string &child_frame_id,
    float tx, float ty, float tz,
    float pitch_deg, float yaw_deg, float roll_deg) {
  std::array<float, 3> translation;
  std::array<float, 4> rotation;
  bool changed = false;

  auto it = _last_transforms.find(child_frame_id);
  if (it != _last_transforms.end()
      && IsSameTransform(it->second, tx, ty, tz, pitch_deg, yaw_deg, roll_deg)) {
    translation = it->second.translation;
    rotation = it->second.rotation;
  } else {
    changed = true;
    // Quaternion conversion lives in TransformQuaternion.h (header-only
    // seam, pinned by test_transform_quaternion.cpp).
    const auto computed =
        TransformFromCarlaRotation(tx, ty, tz, pitch_deg, yaw_deg, roll_deg);
    translation = computed.translation;
    rotation = computed.rotation;
    _last_transforms.insert_or_assign(
        child_frame_id,
        CachedTransform{
            tx, ty, tz, pitch_deg, yaw_deg, roll_deg, translation, rotation});
  }

  if (_is_static) {
    // Publish the first sample, every change (an attached sensor that was
    // moved), and otherwise one repeat per period. The clock restarts when a
    // new episode begins, hence the check for time running backwards.
    const double now = static_cast<double>(seconds) + 1e-9 * static_cast<double>(nanoseconds);
    if (!_has_published || changed
        || now < _last_static_publish
        || now - _last_static_publish >= kStaticRepublishPeriod) {
      _static_publish_due = true;
      _pending_static_time = now;
    }
  }

  // Static transforms hold for all times, which tf2 expresses as a zero stamp.
  msg::TransformStamped ts{};
  ts.header.stamp.sec = _is_static ? 0 : seconds;
  ts.header.stamp.nanosec = _is_static ? 0u : nanoseconds;
  ts.header.frame_id = parent_frame_id;
  ts.child_frame_id = child_frame_id;

  ts.transform.translation.x = translation[0];
  ts.transform.translation.y = translation[1];
  ts.transform.translation.z = translation[2];
  ts.transform.rotation.w = rotation[0];
  ts.transform.rotation.x = rotation[1];
  ts.transform.rotation.y = rotation[2];
  ts.transform.rotation.z = rotation[3];

  _impl->GetMessage()->transforms = {ts};
  return true;
}

}  // namespace ros2
}  // namespace carla
