# CHIP-8 Emulator

Simple CHIP-8 interpreter written in **C++** using **Qt**.

## Features

- CHIP-8 instruction set
- 64×32 monochrome display
- CHIP-8 hexadecimal keypad mapped to keyboard
- Delay and sound timers
- `.ch8` ROM loading through a file dialog
- sound does not work yet

## Controls

```text
CHIP-8     Keyboard

1 2 3 C    1 2 3 4
4 5 6 D    Q W E R
7 8 9 E    A S D F
A 0 B F    Z X C V
```

## Building

Open the project in **Qt Creator**, configure a Qt kit, build and run.

## ROMs

Use the **File → Load** option to select a `.ch8` ROM.