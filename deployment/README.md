# CARLA Helm deployment

The chart in `helm/` uses the shared `openadservice` base chart to deploy CARLA with a ClusterIP Service. CARLA runs in offscreen mode with GPU support and native ROS 2 enabled. The cluster needs an NVIDIA GPU node with the `nvidia` RuntimeClass.

## Install

Run from the repository root:

```sh
helm dependency build deployment/helm
helm upgrade --install carla-simulator deployment/helm \
  --namespace simulation --create-namespace
```

Clients in the same namespace connect to `carla-server:2000` by default.

## Configuration

Defaults live in [`helm/values.yaml`](helm/values.yaml); override them with `-f` or through Helmfile. Use `openadservice.image` for the container image and `openadservice.args` for startup options.

`openadservice.env.CARLA_RPC_PORT` sets the RPC port. Keep `openadservice.expose` in sync for RPC, streaming (RPC + 1), and secondary (RPC + 2).

Native ROS 2 uses Fast DDS on domain 0; it requires DDS connectivity or a middleware bridge to communicate with a Zenoh-based stack.
