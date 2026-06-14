# CHIP-8 Arcade: PONG Edition

A high-performance, modular CHIP-8 hardware emulator built from scratch using **C++17** and **SDL2**. This project simulates vintage 1970s hardware components completely in software, featuring a customized retro-green CRT phosphor aesthetic and optimized cycle timing regulation configured specifically for an arcade-accurate *Pong* experience.

# Key Features

* **Modular OOP Architecture:** Cleanly separates the virtual CPU state machine logic (`chip8.cpp`) from the platform-rendering and real-time game loops (`main.cpp`).
* **Dual-Clock Timing Loop:** Decouples the core instruction execution cycle (regulated at a responsive **800Hz** for fast-paced Pong gameplay) from the internal hardware delay/sound timers running at a precise **60Hz**.
* **True Retro Visuals:** Utilizes SDL2 texture streaming and linear hardware filtering to map the original low-res $64 \times 32$ virtual framebuffer into a vibrant, sharp $4:3$ aspect ratio window.
* **Built for Head-to-Head Multiplayer:** Hardwired, intuitive control mappings replacing the standard confusing hexadecimal keypad layout:
    * **Player 1 (Left Paddle):** `W` (Up) / `S` (Down)
    * **Player 2 (Right Paddle):** `Up Arrow` (Up) / `Down Arrow` (Down)
  
# Tech Stack & Prerequisites

* **Language:** C++17
* **Graphics & Input API:** SDL2 (Simple DirectMedia Layer)
* **Environment & Toolchain:** Windows 10/11, MSYS2 (MinGW w64 / UCRT64 toolchain), and Visual Studio Code.


# Project Structure

chip8-emulator/
├── .vscode/
│   ├── c_cpp_properties.json  # VS Code IntelliSense & library path config
│   └── tasks.json             # Automated g++ compilation flags script
├── chip8.hpp                  # CPU State Machine Header & Blueprint
├── chip8.cpp                  # Fetch-Decode-Execute Logic & Sprite Drawing
├── main.cpp                   # SDL2 Window Management & High-Res Game Loop
└── pong.ch8                   # Target Game ROM

# How It Works (Under the Hood)
The Fetch-Decode-Execute CycleThe core emulator continuously reads memory, fetches two consecutive bytes at the current Program Counter (pc), and decodes them into executable 16-bit instructions (opcodes) using bitwise masks and bit-shifting:$$\text{opcode} = (\text{memory}[\text{pc}] \ll 8) \mid \text{memory}[\text{pc} + 1]$$2. XOR Sprite CollisionGraphics are drawn pixel-by-pixel by processing raw bit-arrays from memory. When a sprite pixel overlaps an existing active pixel on the virtual screen matrix, the emulator uses a bitwise XOR (^=) operation to flip the pixel off and sets the hardware flag register V[0xF] = 1. This low-level hardware design is exactly how the game program detects ball-to-paddle collisions.
