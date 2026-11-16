# OpenADS specific changes

Based on CARLA 1.0 (Unreal Engine 5.8). Changes that upstream has adopted since are no longer listed here: the UE5 sky for generated OpenDRIVE worlds, the GNSS latitude direction, the pedestrian walk-speed limit, the linear steering input, the bicycles with their riders, Python 3.12 and the Dockerfile inside the package.

- Added independently installable CARLA server and client Helm charts under `deployment/helm`, using the shared OpenADS `openadservice` chart.
- Added separate Docker Compose service definitions for the CARLA server and client under `deployment/compose`.
- Added GitLab and GitHub CI pipelines for UE 5.8 CARLA builds on Ubuntu 24.04, including builder image creation, Unreal Engine setup, CARLA package build, release/client Docker image builds, image publishing, and cleanup.
- Integrated additional OpenADS content repository via temporary `Unreal/CarlaUnreal/content-openads` and configured additional maps and vehicles for cooking and packaging. The overlay is switched off until it is migrated to UE 5.8; every affected entry is marked `TODO(openads)`.
- Added full and slim CARLA release images; the slim variant cooks only `Town10HD_Opt` and `OpenDriveMap`, plus `aldenhoven` and `campus` once the content overlay is back, which also makes `campus` the default map again.
- Added infrastructure and road-work-zone props to both release images.
- Added a dedicated CARLA PythonAPI client image; the release image starts the server offscreen with native ROS 2 enabled.
- Enabled ROS 2 actor control for vehicles using `role_name=ego_vehicle`.
- Updated vehicle steering physics to use `Ackermann` steering geometry for Chaos vehicles; two-wheeled vehicles keep the `SingleAngle` steering of their blueprint.
- Added ROS 2 native Odometry and TF sensors publishing `nav_msgs/Odometry` and `tf2_msgs/TFMessage`.
- Added the `ignore_altitude` and `ignore_tilt` attributes to Odometry and TF sensors, and `ignore_tilt` to the IMU sensor.
- Updated ROS 2 transform publishing with actor and sensor parent relationships, dynamic transforms on `/tf`, static transforms for attached sensors on `/tf_static` (latched and repeated once per second of simulation time), and `carla_map` as the root frame.
- Added the `no_transform` attribute to all actor definitions, which makes the ROS 2 native interface skip an actor's transform while it keeps publishing the actor's data, so that another node can own its frame.
- Added the [`-carla-start-time=<unix_timestamp>`](./Docs/ros2_native.md) server option, also settable as `StartTime` under `[CARLA/Server]` in `CarlaSettings.ini`, which starts an episode's simulation clock at a given Unix timestamp instead of at zero. It seeds the episode clock, so `/clock`, ROS 2 message headers, `carla.Timestamp.elapsed_seconds`, and sensor data timestamps all share the same start time and no client needs to be told it separately.
- Run the native ROS 2 interface on Zenoh (`--rmw=zenoh`) in the Helm chart and the Compose file, with the router endpoint set through `ZENOH_CONFIG_OVERRIDE`.
