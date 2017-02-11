all: sample2D

sample2D: Bloxorz.cpp glad.c
	g++ -o sample2D Bloxorz.cpp glad.c -lGL -lglfw -ldl

clean:
	rm sample2D
