# FlexProxy

A crude proxy server(FlexServer) and client(FlexClient) used by myself.

The project name contains a `Flex`? Well, I initially want this project to support **many** protocols to transport **many** different standard protocols.

But... the fact is I'm lazy, only a fixed `tcp-kcp-tcp` proxy pattern is implemented, and it's hard-coded, which is just enough for me by now. The project may be refactored, and more proxy/proxied protocols may be add in the future (but not guarenteed...).

# What does FlexProxy do?

Assuming you are using a tcp based server and client, as show below:

```mermaid
graph LR;
    TcpClient-->TcpServer;
    TcpServer-->TcpClient;
```

The `FlexProxy`'s job is to help the client to communicate with the server using `FlexProxy`'s internal protocol:

```mermaid
graph LR;
    TcpClient-->FlexClient;
    FlexClient-->FlexServer;
    FlexServer-->TcpServer;
    
    FlexClient-->TcpClient;
    FlexServer-->FlexClient;
    TcpServer-->FlexServer;

```

# How to use it?

1. Deploy the `FlexServer` on your server side, run it with your own configuration. `FlexServer` will read `FlexClient`'s data and forward them to you actual tcp server, and back.

```bash
$ ./FlexServer [your_config_file.json]
```

2. Run the `FlexClient` on your client side with you own configuration. `FlexClient` will read your actual client's data and forward then to `FlexServer`, and back.
3. Make your tcp connection connect to `FlexClient` instead of the original tcp server.

```
$ ./FlexClient [your_config_file.json]
```

# Dependencies

- asio (bundled)
- jsonmapper (bundled)
- JsonMapper (bundled)
- kcp (bundled)
- spdlog (bundled)
- a C++ compiler with C++17 support

# Build

```
xmake f --jsonmapper-use-bundled-rapidjson=yes
xmake
```