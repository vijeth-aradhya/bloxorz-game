all: sample2D

sample2D: Bloxorz.cpp glad.c
	g++ -o sample2D Bloxorz.cpp glad.c -lGL -lglfw -ldl -lmpg123 -lao

clean:
	rm sample2D
