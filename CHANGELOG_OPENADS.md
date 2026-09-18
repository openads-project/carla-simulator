# OpenADS specific changes

- Added independently installable CARLA server and client Helm charts under `deployment/helm`, using the shared OpenADS `openadservice` chart.
- Added separate Docker Compose service definitions for the CARLA server and client under `deployment/compose`.
- Added GitLab and GitHub CI pipelines for UE5 CARLA builds, including builder image creation, Unreal Engine setup, CARLA package build, release/client Docker image builds, image publishing, and cleanup.
- Integrated additional OpenADS content repository via temporary `Unreal/CarlaUnreal/content-openads` and configured additional maps and vehicles for cooking and packaging.
- Added full and slim CARLA release images; the slim variant cooks only `Town10HD_Opt`, `aldenhoven`, and `campus`, while both variants use `campus` as the default map.
- Added infrastructure and road-work-zone props to both release images.
- Updated Docker packaging and added dedicated CARLA PythonAPI client image.
- Added Python 3.12 support to the build environment and PythonAPI CMake configuration.
- Enabled ROS 2 actor control for vehicles using `role_name=ego_vehicle`.
- Updated vehicle steering physics to use linear normalized `VehicleControl.Steer` input and `Ackermann` steering geometry for Chaos vehicles.
- Added ROS 2 native Odometry and TF sensors publishing `nav_msgs/Odometry` and `tf2_msgs/TFMessage`.
- Added the `ignore_altitude` and `ignore_tilt` attributes to Odometry and TF sensors, and `ignore_tilt` to the IMU sensor.
- Updated ROS 2 transform publishing with actor and sensor parent relationships, dynamic transforms on `/tf`, static transforms for attached sensors on `/tf_static`, and `carla_map` as the root frame.
- Added the `no_transform` attribute to all actor definitions, which makes the ROS 2 native interface skip an actor's transform while it keeps publishing the actor's data, so that another node can own its frame.
- Added the [`-carla-start-time=<unix_timestamp>`](./Docs/ros2_native.md) server option, also settable as `StartTime` under `[CARLA/Server]` in `CarlaSettings.ini`, which starts an episode's simulation clock at a given Unix timestamp instead of at zero. It seeds the episode clock, so `/clock`, ROS 2 message headers, `carla.Timestamp.elapsed_seconds`, and sensor data timestamps all share the same start time and no client needs to be told it separately.
- Corrected the GNSS latitude direction to preserve CARLA's geographic coordinate convention.
- Increased the maximum pedestrian velocity by applying CARLA's 4096 cm/s walk-speed limit to the UE5 character movement component.
