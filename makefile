all: sample2D

sample2D: Sample_GL3_2D.cpp
	g++ -g -o sample2D Sample_GL3_2D.cpp myconstants.h -lglfw -lGLEW -lGL -ldl -lao -lmpg123 -lm

clean:
	rm sample2D
