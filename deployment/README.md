# CARLA deployments

This directory contains reusable Docker Compose and Helm deployments for the
CARLA server and client. The files follow the multi-deployment layout and
publishing conventions of `openads-dev-environment`, but are maintained
manually because this repository does not provide ROS package launch metadata.

The independent charts in `helm/server` and `helm/client` use the shared `openadservice` base chart. The server runs in offscreen mode with GPU support and native ROS 2 enabled. The client provides an interactive GPU/X11 environment and waits for the server's RPC port before starting. The cluster needs NVIDIA GPU nodes with the `nvidia` RuntimeClass.
