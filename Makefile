CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra

orderbook: main.cpp order.cpp order.hpp
	$(CXX) $(CXXFLAGS) main.cpp order.cpp -o orderbook

run: orderbook
	./orderbook

clean:
	rm -f orderbook
