CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra

orderbook: main.cpp order.cpp trade.cpp order.hpp trade.hpp
	$(CXX) $(CXXFLAGS) main.cpp order.cpp trade.cpp -o orderbook

run: orderbook
	./orderbook

clean:
	rm -f orderbook
