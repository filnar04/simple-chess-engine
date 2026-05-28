# A very simple chess engine.

(this project is still work in progress)


## Requirements
To build
- C compiler supporting C23 with GNU extensions
- make
- SDL3 (only for GUI version)  

for TUI version: terminal supporting RGB color 

## How to build
- Clone this repository
- run ```make``` or ```make no_gui``` to build without GUI

## How to use
### GUI version 
- run ```./chess_gui```
- click a piece to select it, then click one of highlighted squares to move there
### TUI version
- run ```./chess_tui```
- you can add ```--nerd``` flag to use symbols NerdFont symbols for pieces instead of default unicode or ```--ascii``` to use letters
- same as GUI version, but instead of clicking type the coordinates e. g. ```e2```
