FLAGS = -Wall -Wpedantic -O3 -funroll-loops -march=native -std=gnu23

all: build perft chess_tui chess_gui
clean:
	rm build/*

build: 
	mkdir -p build

build/chess.o: src/chess.c
	$(CC) -c $(FLAGS) -o build/chess.o src/chess.c

build/fen.o: src/fen.c
	$(CC) -c $(FLAGS) -o build/fen.o src/fen.c

build/perft.o: src/perft.c
	$(CC) -c $(FLAGS) -o build/perft.o src/perft.c

build/eval.o: src/eval.c
	$(CC) -c $(FLAGS) -o build/eval.o src/eval.c

build/game.o: src/game.c
	$(CC) -c $(FLAGS) -o build/game.o src/game.c

build/tui.o: src/tui.c
	$(CC) -c $(FLAGS) -o build/tui.o src/tui.c

build/gui.o: src/gui.c
	$(CC) -c $(FLAGS) -o build/gui.o src/gui.c

perft: build/chess.o build/fen.o build/perft.o
	$(CC) -o perft build/chess.o build/fen.o build/perft.o

chess_tui: build/chess.o build/eval.o build/game.o build/tui.o
	$(CC) -o chess_tui build/chess.o build/eval.o build/game.o build/tui.o

chess_gui: build/chess.o build/eval.o build/game.o build/gui.o
	$(CC) -o chess_gui -lSDL3 build/chess.o build/eval.o build/game.o build/gui.o

no_gui: build perft chess_tui

