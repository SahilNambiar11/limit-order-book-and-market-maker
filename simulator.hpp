#ifndef SIMULATOR_HPP
#define SIMULATOR_HPP

#include "orderbook.hpp"
#include <random>
#include <vector>

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
    int externalOrderMaximumLifetimeSteps = 250;
    double externalOrderMaximumDistance = 0.25;
    bool externalLiquidityReplenishmentEnabled = true;
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
    int getExternalOrdersCanceled() const;
    double getAverageExternalOrderLifetime() const;
    int getExternalOrdersReplenished() const;

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
    int externalOrderMaximumLifetimeSteps;
    double externalOrderMaximumDistance;
    bool externalLiquidityReplenishmentEnabled;

    struct ExternalOrderState {
        int orderID;
        Side side;
        double price;
        long long submissionTime;
    };

    std::vector<ExternalOrderState> externalOrders;
    int externalOrdersCanceled;
    long long externalOrderLifetimeTotal;
    long long externalOrderLifetimeSamples;
    int externalOrdersReplenished;

    std::mt19937 rng;

    void seedInitialBook();
    void maintainExternalLiquidity();
    void replenishExternalLiquidity(bool countAsReplenishment);
    void addExternalLimitOrder(
        Side side,
        double price,
        int quantity,
        bool countAsReplenishment
    );
    void recordRemainingExternalOrderLifetimes();
    Order generateOrder(double priceChange);
};

#endif
