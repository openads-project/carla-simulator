# OpenADS specific changes

Based on CARLA 1.0.0 (upstream tag `1.0`, Unreal Engine 5.8). Changes that upstream has adopted since are no longer listed here: the UE5 sky for generated OpenDRIVE worlds, the GNSS latitude direction, the pedestrian walk-speed limit, the linear steering input, the bicycles with their riders, Python 3.12 and the Dockerfile inside the package.

- Added independently installable CARLA server and client Helm charts under `deployment/helm`, using the shared OpenADS `openadservice` chart.
- Added separate Docker Compose service definitions for the CARLA server and client under `deployment/compose`.
- Added GitLab and GitHub CI pipelines for UE 5.8 CARLA builds on Ubuntu 24.04, including builder image creation, Unreal Engine setup, CARLA package build, release/client Docker image builds, image publishing, and cleanup.
- Integrated the OpenADS content repository (branch `ue58`) as an overlay on the official content: CI clones it to the temporary `Unreal/CarlaUnreal/content-openads` and `Util/SetupUtils/MergeContentOverlay.py` merges it in. It adds the maps `aldenhoven`, `campus` (the default map) and `test-track` and the vehicles `vehicle.ika.karl`, `vehicle.ika.shuttle` and `vehicle.ika.wheelloader`. Paths listed in `Util/SetupUtils/ContentOverlayExclude.txt` are left out together with the catalog entries that name them (none at the moment). The overlay's bicycle files are CARLA 1.0's bicycles with the OpenADS rider pose and drivetrain and the `Roadbike` path-case fix. The full package also cooks CARLA 1.0's `Town01_Opt` to `Town07_Opt`, which upstream ships in the separate `AdditionalMaps` pack (do not install that pack on top: two copies of a town collide).
- Added full and slim CARLA release images; the slim variant cooks only `Town10HD_Opt`, `OpenDriveMap`, `aldenhoven` and `campus`.
- Added infrastructure and road-work-zone props to both release images.
- Added a dedicated CARLA PythonAPI client image; the release image starts the server offscreen with native ROS 2 enabled.
- Enabled ROS 2 actor control for vehicles using `role_name=ego_vehicle`.
- Updated vehicle steering physics to use `Ackermann` steering geometry for Chaos vehicles; two-wheeled vehicles keep the `SingleAngle` steering of their blueprint.
- Fixed the rider of two-wheeled vehicles trailing the vehicle while it moves: the rider's animation now evaluates after physics toa lign with the rider with the vehicle.
- Added rider hand and foot settings for two-wheeled vehicles: `RiderWristOffset` moves the hand IK target off the handlebar socket, `RiderLeftHandRotation` and `RiderRightHandRotation` turn the hands around the grips; `RiderFootOffset` moves the foot IK target off the pedal axle and `RiderLeftFootRotation`/`RiderRightFootRotation` set the foot angle, so the ball of the foot rests on the pedal.
- Added ROS 2 native Odometry and TF sensors publishing `nav_msgs/Odometry` and `tf2_msgs/TFMessage`.
- Added the `ignore_altitude` and `ignore_tilt` attributes to Odometry and TF sensors, and `ignore_tilt` to the IMU sensor.
- Updated ROS 2 transform publishing with actor and sensor parent relationships, dynamic transforms on `/tf`, static transforms for attached sensors on `/tf_static` (latched and repeated once per second of simulation time), and `carla_map` as the root frame.
- Added the `no_transform` attribute to all actor definitions, which makes the ROS 2 native interface skip an actor's transform while it keeps publishing the actor's data, so that another node can own its frame.
- Added the [`-carla-start-time=<unix_timestamp>`](./Docs/ros2_native.md) server option, also settable as `StartTime` under `[CARLA/Server]` in `CarlaSettings.ini`, which starts an episode's simulation clock at a given Unix timestamp instead of at zero. It seeds the episode clock, so `/clock`, ROS 2 message headers, `carla.Timestamp.elapsed_seconds`, and sensor data timestamps all share the same start time and no client needs to be told it separately.
- Run the native ROS 2 interface on Zenoh (`--rmw=zenoh`) in the Helm chart and the Compose file, with the router endpoint set through `ZENOH_CONFIG_OVERRIDE`.
