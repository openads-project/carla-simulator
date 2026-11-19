# CARLA deployments

This directory contains reusable Docker Compose and Helm deployments for the
CARLA server and client. The files follow the multi-deployment layout and
publishing conventions of `openads-dev-environment`, but are maintained
manually because this repository does not provide ROS package launch metadata.

The independent charts in `helm/carla-server` and `helm/carla-client` use the shared `openadservice` base chart. The server runs in offscreen mode with GPU support and native ROS 2 enabled. ROS 2 runs on Zenoh (`--rmw=zenoh`), so a Zenoh router (`rmw_zenohd`, for example the `zenoh-router` chart) has to be reachable: the server connects to `tcp/zenoh-router:7447`, set through `openadservice.rmw.zenoh.configOverrides` in the Helm chart and through `ZENOH_CONFIG_OVERRIDE` in the Compose file. The client provides an interactive GPU/X11 environment and waits for the server's RPC port before starting. The cluster needs NVIDIA GPU nodes with the `nvidia` RuntimeClass.
