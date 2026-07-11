# Juntos

Welcome to Project Juntos, a peer-to-peer UDP networking library built from scratch for performance.

This project is currently in active development and I welcome any issues/PR's!

# Building with CMake

## Prerequisites

* C++20 (I'm using clang++-16).
* Python 3 (locally developed with 3.13.2).

## How to Integrate Juntos into your Project

Feel free to add my repo as a submodule! If you're looking for an example, my distributed network simulator project, [Orla](github.com/nmstory/orla) is built using Juntos.

## Building Juntos as a Standalone Project with Examples

The included [`chat`](examples/chat/main.cpp) example is a minimal peer-to-peer chat that exercises the full Juntos flow end to end:

1. Each peer registers with the STUN server (`stun.py`) and asks for the current guest list.
2. Peers discover one another and UDP hole-punch a direct connection — once that's done, no traffic flows through the server.
3. Anything you type is sent straight to every connected peer, and incoming messages are printed as `Peer: <message>`.

When a peer connects you'll see `Peer discovered: <ip>:<port>` on the client and a `Client joined` line on the STUN server. The steps below build the example and run two peers so you can chat between them.

### Linux
Here's the simple steps, from the root directory of the project:

```sh
mkdir build && cd build
```

Then build:
```sh
cmake -D JUNTOS_BUILD_EXAMPLES=ON .. && make
```

Finally, run the STUN server and then a chat peer (optionally pass the STUN
host and port; they default to `127.0.0.1:12345`):
```sh
python3 ../stun.py &
./examples/chat/juntos_chat {DESIRED_PORT_NUMBER} [STUN_HOST] [STUN_PORT]
```

Start a second peer in another terminal (on a different port) to chat between
them — type a message in one and it appears in the other:
```sh
./examples/chat/juntos_chat {ANOTHER_PORT_NUMBER}
```

To go beyond localhost, host `stun.py` somewhere reachable, pass its address as `STUN_HOST`/`STUN_PORT`, and make sure that port is open for **UDP** — as cloud firewalls can default to TCP.

### Windows
I'm currently using Microsoft's Visual Studio 2022, which has support to compile and launch CMake projects.

[See here](https://learn.microsoft.com/en-us/cpp/build/cmake-projects-in-visual-studio?view=msvc-170) for a tutorial on how to configure this!

## Credits

Crediting projects utilised for inspiration/support
* https://github.com/mas-bandwidth/yojimbo

## Most importantly...

Why the name [Juntos](https://www.linguee.com/portuguese-english/translation/juntos.html)? I'm currently studying Portuguese (brasileiro, me desculpa), and this is a word that roughly translates to the English word [together](https://www.oed.com/search/dictionary/?scope=Entries&q=together), representing the way this library will _connect_ clients to enable shared multiplayer experiences.
