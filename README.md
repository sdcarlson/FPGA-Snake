# FPGA Snake

The classic Snake game running on a MicroBlaze soft processor on a Digilent Basys 3 FPGA, played with a Pmod joystick on a Pmod RGB OLED display.

**Tech stack:** C, Xilinx MicroBlaze, Xilinx Vivado / Vitis, Digilent Basys 3 (Artix-7 FPGA), Pmod OLEDrgb, Pmod JSTK2, AXI GPIO, SPI

<img width="684" alt="Snake game running on the Basys 3 with the Pmod OLEDrgb display" src="https://github.com/sdcarlson/FPGA-Snake/assets/66243744/8dd790de-8424-4a21-82eb-710f25f7ebb0">

## Features

- Snake steered with the Pmod JSTK2 joystick and drawn on a 12x8 cell grid on the Pmod OLEDrgb display
- Fruit respawns at a random cell that is not occupied by the snake's tail
- Snake grows by one segment per fruit; the game ends when the head collides with the tail
- Board edges wrap around to the opposite side
- Game speeds up with every fruit eaten
- Two-digit score shown live on the Basys 3 seven-segment display via AXI GPIO

## How it works

`snake.c` is a bare-metal application for a MicroBlaze processor instantiated in the FPGA fabric. It uses the Digilent Pmod drivers (`PmodJSTK2`, `PmodOLEDrgb`) over AXI SPI/GPIO and the Xilinx `XGpio` driver for the seven-segment display.

- Game state (head, tail segments, fruit, direction, score) lives in a `SnakeGame` struct.
- The main loop multiplexes the seven-segment digits every iteration and runs a game tick every `limit` iterations. Each tick reads the joystick, updates direction (reversing into the tail is ignored), advances the snake, and redraws the OLED using custom font glyphs.
- Eating a fruit lowers `limit`, which shortens the time between ticks and speeds up the game.

## Getting started

This repository contains the application source only. To run it you need a Vivado hardware design with:

- A MicroBlaze processor
- Digilent `PmodOLEDrgb` and `PmodJSTK2` IP cores (from the Digilent vivado-library), connected to Pmod ports on the Basys 3
- A dual-channel AXI GPIO for the seven-segment anodes and cathodes

Then:

1. Generate the bitstream in Vivado and export the hardware platform (`.xsa`).
2. In Vitis, create a platform from the `.xsa` and a new standalone C application.
3. Add `snake.c` to the application sources and build. `snake.c` includes a `bitmap.h` header that is not in this repository; remove the include if your build does not provide one (no symbols from it are used).
4. Program the FPGA and run the application on the Basys 3.

## Project context

Team project, uploaded to GitHub in January 2024. On top of the base game, I added the feature that speeds the snake up as the score increases.
