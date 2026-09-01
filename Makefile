CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra

orderbook: main.cpp order.cpp trade.cpp orderbook.cpp order.hpp trade.hpp orderbook.hpp
	$(CXX) $(CXXFLAGS) main.cpp order.cpp trade.cpp orderbook.cpp -o orderbook

run: orderbook
	./orderbook

clean:
	rm -f orderbook
