CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra

orderbook: main.cpp order.cpp trade.cpp orderbook.cpp simulator.cpp marketmaker.cpp order.hpp trade.hpp orderbook.hpp simulator.hpp marketmaker.hpp
	$(CXX) $(CXXFLAGS) main.cpp order.cpp trade.cpp orderbook.cpp simulator.cpp marketmaker.cpp -o orderbook

run: orderbook
	./orderbook

clean:
	rm -f orderbook
