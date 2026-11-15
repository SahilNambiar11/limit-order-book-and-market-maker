#ifndef EXPERIMENT_HPP
#define EXPERIMENT_HPP

#include "marketmaker.hpp"
#include "simulator.hpp"

#include <cstdint>
#include <vector>

struct SimulationConfig {
    int numberOfSessions = 100;
    int stepsPerSession = 10000;
    MarketMakerConfig marketMaker;
    SimulatorConfig simulator;
};

struct SessionResult {
    int sessionNumber;
    unsigned int randomSeed;
    double pnl;
    double fillRate;
    int maximumInventoryExposure;
    int endingInventory;
    std::int64_t boughtVolume;
    std::int64_t soldVolume;
    double cash;
    double inventoryValue;
    double realizedPnL;
    double unrealizedPnL;
    double tradingFees;
    double terminalLiquidationCosts;
    int terminalLiquidationQuantity;
    double maximumDrawdown;
    double terminalReferencePrice;
    double endingEquity;
    bool ledgerReconciled;
    double averageQuotedSpread;
    double maximumQuotedSpread;
    double averageMakerQuoteDistance;
    double averageBookMidpointDeviation;
    int externalOrdersCanceled;
    double averageExternalOrderLifetime;
    int externalOrdersReplenished;
};

struct SimulationResult {
    int numberOfSessions;
    int stepsPerSession;
    std::int64_t totalSimulatedSteps;

    double averagePnL;
    double medianPnL;
    double pnlStandardDeviation;
    double bestPnL;
    double worstPnL;
    int profitableSessions;
    double profitableSessionPercentage;

    double averageFillRate;
    double averageMaximumInventoryExposure;
    int worstInventoryExposure;
    double averageMaximumDrawdown;
    double totalTradingFees;
    double totalTerminalLiquidationCosts;
    bool allSessionsReconciled;
    double averageQuotedSpread;
    double maximumQuotedSpread;
    double averageMakerQuoteDistance;
    double averageBookMidpointDeviation;
    std::int64_t totalExternalOrdersCanceled;
    double averageExternalOrderLifetime;
    std::int64_t totalExternalOrdersReplenished;

    std::int64_t totalBoughtVolume;
    std::int64_t totalSoldVolume;

    std::vector<SessionResult> sessionResults;
};

struct StrategyComparisonResult {
    SimulationResult inventoryAware;
    SimulationResult fixedSpread;
};

SimulationResult runSimulation(const SimulationConfig& config);
StrategyComparisonResult compareStrategies(const SimulationConfig& config);

#endif
