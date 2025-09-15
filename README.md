# Chip-8 Emulator
A simple Chip-8 emulator written in C++.
This project is being implemented based on the [Arjun Nair](https://www.arjunnair.in/).

## Features

## Setup
0. Build tools:
    ```bash
    sudo apt install build-essential cmake build-ninja -y
    ```
1. Install dependencies:
    ```bash
    sudo apt install -y pkg-config libsdl2-dev
    ```
2. Build the project:
    ```bash
    cmake -S . -G Ninja -B build
    cmake --build build
    ```
3. Run the emulator:
    ```bash
    ./build/chip8
    ```
