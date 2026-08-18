# Raspberry Pi Photo Display

A C program that cycles through a folder of images on a Raspberry Pi and 
displays them on an attached monitor. Each photo shows for a set duration, 
then the program transitions randomly to another photo in the folder 
without repeating images within the same shuffle cycle. The program can 
be stopped at any time by the user by pressing the ideal keys.

This project was built to deepen my understanding of C and its libraries — 
particularly dynamic memory management, file I/O, and low-level graphics 
rendering via SDL2.

## Features
- Recursively scans a folder for `.jpg`, `.jpeg`, and `.png` files
- Randomly shuffles the display order each cycle (Fisher-Yates shuffle)
- Displays images fullscreen with aspect ratio preserved
- GPU-accelerated rendering via SDL2 (textures loaded into VRAM)
- Automatically downscales oversized images to fit GPU texture limits
- Exit anytime with the spacebar or by closing the window

## Known limitations
- Because each cycle is shuffled independently, there's a small chance 
  the last photo of one cycle repeats as the first photo of the next.

## Requirements
- SDL2
- SDL2_image

On Raspberry Pi OS / Debian-based systems:

```bash
sudo apt update
sudo apt install libsdl2-dev libsdl2-image-dev
```

## Building

```bash
gcc main.c -o photodisplay -lSDL2 -lSDL2_image
```

## Usage

```bash
./photodisplay /path/to/your/image/folder
```

## How it works
- Images are tracked in a dynamically resized `ImageList` struct
- Filenames are shuffled between cycles so the display order changes 
  each time, without immediate repeats within a cycle
- Each image is loaded as an SDL texture, scaled to fit the screen 
  while preserving aspect ratio, and rendered for a fixed duration 
  before the next one loads
