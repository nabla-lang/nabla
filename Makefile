.PHONY: all
all: build

build:
	g++ -g -Wall -Wextra -Werror -Wfatal-errors src/nabla.cpp -o nabla
