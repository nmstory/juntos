# juntos

Welcome to Project Juntos, a peer-to-peer UDP game networking library built from scratch for both performance and anti-cheat.

This project is currently in active development, and I welcome any-and-all issues/PR's!

# Building with CMake

## Prerequisites

* C++20 (I use the clang++-16 compiler).
* Python 3 (locally developed with 3.13.2).

## Build

### Linux
Here's the simple steps, from the root directory of the project:

```sh
mkdir build && cd build
```

Then build:
```sh
cmake .. && make
```

Finally, run:
```sh
python3 ../stun.py &
./juntos {DESIRED_PORT_NUMBER}
```

### Windows
I'm currently using Microsoft's Visual Studio 2022, which has support to compile and launch CMake projects.

[See here](https://learn.microsoft.com/en-us/cpp/build/cmake-projects-in-visual-studio?view=msvc-170) for a tutorial on how to configure this!

## Credits

Crediting projects utilised for inspiration/support
* https://github.com/mas-bandwidth/yojimbo
* https://github.com/friendlyanon/cmake-init

## Most importantly...

Why the name [Juntos](https://www.linguee.com/portuguese-english/translation/juntos.html)? I'm currently studying Portuguese (brasileiro, me desculpa), and this is a word that roughly translates to the English word [together](https://www.oed.com/search/dictionary/?scope=Entries&q=together), representing the way this library will _connect_ clients to enable shared multiplayer experiences.
