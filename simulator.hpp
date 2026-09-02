#ifndef SIMULATOR_HPP
#define SIMULATOR_HPP

#include "orderbook.hpp"
#include <random>

class MarketMaker;

class Simulator {
public:
    Simulator(
        OrderBook& book,
        MarketMaker& marketMaker,
        unsigned int seed
    );

    void run(int steps);
    int getNextOrderID();

private:
    OrderBook& book;
    MarketMaker& marketMaker;

    long long currentTime;
    int nextOrderID;
    double referencePrice;

    std::mt19937 rng;

    Order generateOrder();
};

#endif
