# CARLA deployments

The independent charts in `helm/server` and `helm/client` use the shared `openadservice` base chart. The server runs in offscreen mode with GPU support and native ROS 2 enabled. The client provides an interactive GPU/X11 environment and waits for the server's RPC port before starting. The cluster needs NVIDIA GPU nodes with the `nvidia` RuntimeClass.

## Helm

Run from the repository root:

```sh
helm dependency build deployment/helm/server
helm upgrade --install carla-server deployment/helm/server \
  --namespace simulation --create-namespace

helm dependency build deployment/helm/client
helm upgrade --install carla-client deployment/helm/client \
  --namespace simulation --create-namespace
```

The client expects the server at `carla-server:2000` in the same namespace. It remains running with `sleep infinity` so commands can be started with `kubectl exec`.

## Configuration

Defaults live in [`helm/server/values.yaml`](helm/server/values.yaml) and [`helm/client/values.yaml`](helm/client/values.yaml); override them with `-f` or through Helmfile. Use `openadservice.image` for the container image and `openadservice.args` for startup options.

`openadservice.env.CARLA_RPC_PORT` sets the RPC port. Keep `openadservice.expose` in sync for RPC, streaming (RPC + 1), and secondary (RPC + 2).

Native ROS 2 uses Fast DDS on domain 0; it requires DDS connectivity or a middleware bridge to communicate with a Zenoh-based stack.

The client defaults to display `:0` and mounts `/tmp/.X11-unix`. Set `openadservice.x11Display` and `openadservice.x11XauthorityFile` for the target GPU node's X11 session.

## Docker Compose

The reusable server and client service definitions live in `compose/docker-compose.server.yml` and `compose/docker-compose.client.yml`. They extend the shared OpenADS GPU/X11 service template and can be combined by a parent Compose project.
