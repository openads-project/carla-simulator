# OpenADS specific changes

- Added GitLab and GitHub CI pipelines for UE5 CARLA builds, including builder image creation, Unreal Engine setup, CARLA package build, release/client Docker image builds, image publishing, and cleanup.
- Integrated additional OpenADS content repository via temporary `Unreal/CarlaUnreal/content-openads` and configured additional maps and vehicles for cooking and packaging.
- Updated Docker packaging and added dedicated CARLA PythonAPI client image.
- Added Python 3.12 support to the build environment and PythonAPI CMake configuration.
- Enabled ROS 2 actor control for vehicles using `role_name=ego_vehicle`.
- Updated vehicle steering physics to use linear normalized `VehicleControl.Steer` input and `Ackermann` steering geometry for Chaos vehicles.
- Added ROS 2 native Odometry and TF sensors publishing `nav_msgs/Odometry` and `tf2_msgs/TFMessage`.
- Added the `ignore_altitude` and `ignore_tilt` attributes to Odometry and TF sensors, and `ignore_tilt` to the IMU sensor.
- Updated ROS 2 transform publishing with actor and sensor parent relationships, dynamic transforms on `/tf`, static transforms for attached sensors on `/tf_static`, and `carla_map` as the root frame.
- Corrected the GNSS latitude direction to preserve CARLA's geographic coordinate convention.
