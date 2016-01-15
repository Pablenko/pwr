all: zespolqrs

zespolqrs:
	g++ zespolqrs.cpp -o zespolqrs --std=c++11 -lwfdb
	./zespolqrs

clean:
	rm zespolqrs
	rm 100s.qrs
