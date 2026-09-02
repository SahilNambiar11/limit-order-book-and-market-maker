#ifndef SIMULATOR_HPP
#define SIMULATOR_HPP

#include "orderbook.hpp"
#include <random>

class MarketMaker;

struct SimulatorConfig {
    unsigned int randomSeed = 1;
    double marketOrderProbability = 0.20;
    int maximumGeneratedOrderQuantity = 50;
};

class Simulator {
public:
    Simulator(
        OrderBook& book,
        MarketMaker& marketMaker,
        const SimulatorConfig& config = SimulatorConfig{}
    );

    void run(int steps);
    int getNextOrderID();

private:
    OrderBook& book;
    MarketMaker& marketMaker;

    long long currentTime;
    int nextOrderID;
    double referencePrice;
    double marketOrderProbability;
    int maximumGeneratedOrderQuantity;

    std::mt19937 rng;

    Order generateOrder();
};

#endif
