all: sample2D

sample2D: Sample_GL3_2D.cpp
	g++ -g -o sample2D Sample_GL3_2D.cpp -lglfw -lGLEW -lGL -ldl -lmpg123 -lao --std=c++11

clean:
	rm sample2D
