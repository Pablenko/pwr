all: zespolqrs

zespolqrs:
	g++ zespolqrs.cpp -o zespolqrs --std=c++11 -lwfdb

clean:
	rm zespolqrs
