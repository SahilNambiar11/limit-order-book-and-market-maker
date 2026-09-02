#include "experiment.hpp"

#include "orderbook.hpp"

#include <algorithm>
#include <limits>
#include <stdexcept>

namespace {

void validateConfig(const SimulationConfig& config) {
    if (config.numberOfSessions <= 0) {
        throw std::invalid_argument("numberOfSessions must be greater than zero");
    }
    if (config.stepsPerSession <= 0) {
        throw std::invalid_argument("stepsPerSession must be greater than zero");
    }
    if (config.marketMaker.quoteSize <= 0) {
        throw std::invalid_argument("quoteSize must be greater than zero");
    }
    if (config.marketMaker.maximumInventory <= 0) {
        throw std::invalid_argument("maximumInventory must be greater than zero");
    }
    if (config.simulator.marketOrderProbability < 0.0 ||
        config.simulator.marketOrderProbability > 1.0) {
        throw std::invalid_argument(
            "marketOrderProbability must be between zero and one"
        );
    }
    if (config.simulator.maximumGeneratedOrderQuantity <= 0) {
        throw std::invalid_argument(
            "maximumGeneratedOrderQuantity must be greater than zero"
        );
    }
}

}  // namespace

SimulationResult runSimulation(const SimulationConfig& config) {
    validateConfig(config);

    SimulationResult result{};
    result.numberOfSessions = config.numberOfSessions;
    result.stepsPerSession = config.stepsPerSession;
    result.totalSimulatedSteps =
        static_cast<std::int64_t>(config.numberOfSessions) *
        config.stepsPerSession;
    result.bestPnL = -std::numeric_limits<double>::infinity();
    result.worstPnL = std::numeric_limits<double>::infinity();
    result.sessionResults.reserve(config.numberOfSessions);

    double totalPnL = 0.0;
    double totalFillRate = 0.0;
    double totalMaximumInventoryExposure = 0.0;

    for (int i = 0; i < config.numberOfSessions; ++i) {
        OrderBook book;
        MarketMaker marketMaker(book, config.marketMaker);

        SimulatorConfig sessionConfig = config.simulator;
        sessionConfig.randomSeed += static_cast<unsigned int>(i);
        Simulator simulator(book, marketMaker, sessionConfig);
        simulator.run(config.stepsPerSession);

        SessionResult session{
            i + 1,
            sessionConfig.randomSeed,
            marketMaker.getPnL(),
            marketMaker.getFillRate(),
            marketMaker.getMaxAbsInventory(),
            marketMaker.getInventory(),
            marketMaker.getTotalBought(),
            marketMaker.getTotalSold()
        };

        result.sessionResults.push_back(session);
        totalPnL += session.pnl;
        totalFillRate += session.fillRate;
        totalMaximumInventoryExposure += session.maximumInventoryExposure;
        result.totalBoughtVolume += session.boughtVolume;
        result.totalSoldVolume += session.soldVolume;

        result.bestPnL = std::max(result.bestPnL, session.pnl);
        result.worstPnL = std::min(result.worstPnL, session.pnl);
        result.worstInventoryExposure = std::max(
            result.worstInventoryExposure,
            session.maximumInventoryExposure
        );

        if (session.pnl > 0.0) {
            ++result.profitableSessions;
        }
    }

    const double sessionCount = static_cast<double>(config.numberOfSessions);
    result.averagePnL = totalPnL / sessionCount;
    result.averageFillRate = totalFillRate / sessionCount;
    result.averageMaximumInventoryExposure =
        totalMaximumInventoryExposure / sessionCount;

    return result;
}
