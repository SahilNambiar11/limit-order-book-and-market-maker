#ifndef SIMULATOR_HPP
#define SIMULATOR_HPP

#include "orderbook.hpp"
#include <random>

class MarketMaker;

struct SimulatorConfig {
    unsigned int randomSeed = 1;
    double marketOrderProbability = 0.20;
    int maximumGeneratedOrderQuantity = 50;
    double referencePriceVolatility = 0.02;
    double referencePriceTrend = 0.0;
    double informedOrderProbability = 0.15;
    int initialBookDepth = 5;
    int initialLevelQuantity = 100;
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
    double getReferencePrice() const;

private:
    OrderBook& book;
    MarketMaker& marketMaker;

    long long currentTime;
    int nextOrderID;
    double referencePrice;
    double marketOrderProbability;
    int maximumGeneratedOrderQuantity;
    double referencePriceVolatility;
    double referencePriceTrend;
    double informedOrderProbability;
    int initialBookDepth;
    int initialLevelQuantity;

    std::mt19937 rng;

    void seedInitialBook();
    Order generateOrder(double priceChange);
};

#endif
