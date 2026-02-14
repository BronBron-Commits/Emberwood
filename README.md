# Emberwood

**Emberwood** is a 2D, top-down, 16-bit style adventure game for PC, inspired by classic titles like *The Legend of Zelda: A Link to the Past*. Built from scratch in C++ with SDL2, Emberwood features tile-based maps, sprite animations, and an entity system for players, NPCs, and objects.

## Features (Planned)

* Tile-based world with outdoor and dungeon areas
* Sprite-based player and NPCs
* Basic combat and item collection
* Event triggers and interactive objects (doors, chests)
* 16-bit style graphics and audio

## Project Structure

```
Emberwood/
├── src/                 # Source code
│   ├── main.cpp         # Entry point
│   ├── engine/          # Rendering and engine utilities
│   └── entities/        # Game entities (player, tree, enemies)
├── assets/              # Sprites, tiles, and audio
├── include/             # External headers (SDL2, etc.)
├── build/               # Compiled binaries and objects
├── CMakeLists.txt       # Build configuration
└── README.md            # This file
```

## Dependencies

* [SDL2](https://www.libsdl.org/) (Graphics, input, audio)
* C++17 compatible compiler (GCC, Clang, or MSVC)
* CMake (optional, for building)

## Building

1. Clone the repository:

```bash
git clone <repo_url> Emberwood
cd Emberwood
```

2. Create a build directory and build with CMake:

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

3. Run the game executable:

```bash
./Emberwood
```

## Contributing

Contributions are welcome! Suggested areas:

* Adding new entities (enemies, NPCs, items)
* Expanding tilemaps and levels
* Implementing audio and visual effects
* Optimizing rendering and performance

## License

This project is MIT licensed.
