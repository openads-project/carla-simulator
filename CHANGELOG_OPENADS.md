# OpenADS specific changes

- Added GitLab and GitHub CI pipelines for UE5 CARLA builds, including builder image creation, Unreal Engine setup, CARLA package build, release/client Docker image builds, image publishing, and cleanup.
- Integrated additional OpenADS content repository via temporary `Unreal/CarlaUnreal/content-openads` and configured additional maps and vehicles for cooking and packaging.
- Updated Docker packaging and added dedicated CARLA PythonAPI client image.
- Added Python 3.12 support to the build environment and PythonAPI CMake configuration.
- Enabled ROS 2 actor control for vehicles using `role_name=ego_vehicle`.
- Updated vehicle steering physics to use linear normalized `VehicleControl.Steer` input and `Ackermann` steering geometry for Chaos vehicles.
