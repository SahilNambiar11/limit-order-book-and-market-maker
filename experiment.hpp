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
};

struct SimulationResult {
    int numberOfSessions;
    int stepsPerSession;
    std::int64_t totalSimulatedSteps;

    double averagePnL;
    double bestPnL;
    double worstPnL;
    int profitableSessions;

    double averageFillRate;
    double averageMaximumInventoryExposure;
    int worstInventoryExposure;

    std::int64_t totalBoughtVolume;
    std::int64_t totalSoldVolume;

    std::vector<SessionResult> sessionResults;
};

SimulationResult runSimulation(const SimulationConfig& config);

#endif
