# juntos

Welcome to Project Juntos, a peer-to-peer game networking library built from scratch for both performance and anti-cheat.

This project is currently in active development, and I welcome any-and-all issues/PR's!

# Building with CMake

## Prerequisites

docker (used version: 4.38.0)

## Build

This project doesn't require any special command-line flags, in order to keep
things simple.

Here's the simple steps, from the root directory of the project:

```sh
git submodule update --init --recursive
```

```sh
(cd submodules/violet && sudo docker build -t violet .)
```

```sh
mkdir build && cd build
```

Then build:

```sh
cmake .. && make
```

Finally, run:

```sh
 cd submodules/violet && sudo docker run --network=host violet --credentials=USER:PASSWORD -d
./juntos
```

## Credits

Crediting projects utilised for inspiration/support
* https://github.com/mas-bandwidth/yojimbo
* https://github.com/friendlyanon/cmake-init

## Most importantly...

Why the name [Juntos](https://www.linguee.com/portuguese-english/translation/juntos.html)? I'm currently studying Portuguese (brasileiro, me desculpa), and this is a word that roughly translates to the English word [connected](https://www.oed.com/dictionary/connected_adj), representing the way this library will _connect_ clients to enable shared multiplayer experiences.