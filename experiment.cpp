#include "experiment.hpp"

#include "orderbook.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <vector>

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
    if (!std::isfinite(config.marketMaker.transactionFeePerUnit) ||
        config.marketMaker.transactionFeePerUnit < 0.0) {
        throw std::invalid_argument(
            "transactionFeePerUnit must not be negative"
        );
    }
    if (config.marketMaker.quoteLatencySteps < 0) {
        throw std::invalid_argument("quoteLatencySteps must not be negative");
    }
    if (!std::isfinite(
            config.marketMaker.terminalLiquidationCostPerUnit
        ) || config.marketMaker.terminalLiquidationCostPerUnit < 0.0) {
        throw std::invalid_argument(
            "terminalLiquidationCostPerUnit must not be negative"
        );
    }
    if (!std::isfinite(config.marketMaker.fixedSpread) ||
        config.marketMaker.fixedSpread <= 0.0) {
        throw std::invalid_argument("fixedSpread must be greater than zero");
    }
    if (!std::isfinite(
            config.marketMaker.maximumQuoteDistanceFromReference
        ) || config.marketMaker.maximumQuoteDistanceFromReference < 0.01) {
        throw std::invalid_argument(
            "maximumQuoteDistanceFromReference must be at least 0.01"
        );
    }
    if (!std::isfinite(config.simulator.marketOrderProbability) ||
        config.simulator.marketOrderProbability < 0.0 ||
        config.simulator.marketOrderProbability > 1.0) {
        throw std::invalid_argument(
            "marketOrderProbability must be between zero and one"
        );
    }
    if (!std::isfinite(config.simulator.informedOrderProbability) ||
        config.simulator.informedOrderProbability < 0.0 ||
        config.simulator.informedOrderProbability > 1.0) {
        throw std::invalid_argument(
            "informedOrderProbability must be between zero and one"
        );
    }
    if (config.simulator.maximumGeneratedOrderQuantity <= 0) {
        throw std::invalid_argument(
            "maximumGeneratedOrderQuantity must be greater than zero"
        );
    }
    if (!std::isfinite(config.simulator.referencePriceVolatility) ||
        config.simulator.referencePriceVolatility < 0.0) {
        throw std::invalid_argument(
            "referencePriceVolatility must not be negative"
        );
    }
    if (!std::isfinite(config.simulator.referencePriceTrend)) {
        throw std::invalid_argument("referencePriceTrend must be finite");
    }
    if (config.simulator.initialBookDepth < 0) {
        throw std::invalid_argument("initialBookDepth must not be negative");
    }
    if (config.simulator.initialLevelQuantity <= 0) {
        throw std::invalid_argument(
            "initialLevelQuantity must be greater than zero"
        );
    }
    if (config.simulator.externalOrderMaximumLifetimeSteps < 0) {
        throw std::invalid_argument(
            "externalOrderMaximumLifetimeSteps must not be negative"
        );
    }
    if (!std::isfinite(config.simulator.externalOrderMaximumDistance) ||
        config.simulator.externalOrderMaximumDistance < 0.0) {
        throw std::invalid_argument(
            "externalOrderMaximumDistance must not be negative"
        );
    }
}

double median(std::vector<double> values) {
    std::sort(values.begin(), values.end());
    const std::size_t middle = values.size() / 2;
    if (values.size() % 2 == 0) {
        return (values[middle - 1] + values[middle]) / 2.0;
    }
    return values[middle];
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
    result.allSessionsReconciled = true;
    result.sessionResults.reserve(config.numberOfSessions);

    double totalPnL = 0.0;
    double totalFillRate = 0.0;
    double totalMaximumInventoryExposure = 0.0;
    double totalMaximumDrawdown = 0.0;
    double totalAverageQuotedSpread = 0.0;
    double totalAverageMakerQuoteDistance = 0.0;
    double totalAverageBookMidpointDeviation = 0.0;
    double totalAverageExternalOrderLifetime = 0.0;
    std::vector<double> pnlValues;
    pnlValues.reserve(config.numberOfSessions);

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
            marketMaker.getTotalSold(),
            marketMaker.getCash(),
            marketMaker.getInventoryValue(),
            marketMaker.getRealizedPnL(),
            marketMaker.getUnrealizedPnL(),
            marketMaker.getTradingFees(),
            marketMaker.getTerminalLiquidationCosts(),
            marketMaker.getTerminalLiquidationQuantity(),
            marketMaker.getMaximumDrawdown(),
            simulator.getReferencePrice(),
            marketMaker.getCash() + marketMaker.getInventoryValue(),
            marketMaker.isLedgerReconciled(),
            marketMaker.getAverageQuotedSpread(),
            marketMaker.getMaximumQuotedSpread(),
            marketMaker.getAverageQuoteDistanceFromReference(),
            marketMaker.getAverageBookMidpointDeviation(),
            simulator.getExternalOrdersCanceled(),
            simulator.getAverageExternalOrderLifetime(),
            simulator.getExternalOrdersReplenished()
        };

        result.sessionResults.push_back(session);
        pnlValues.push_back(session.pnl);
        totalPnL += session.pnl;
        totalFillRate += session.fillRate;
        totalMaximumInventoryExposure += session.maximumInventoryExposure;
        totalMaximumDrawdown += session.maximumDrawdown;
        totalAverageQuotedSpread += session.averageQuotedSpread;
        totalAverageMakerQuoteDistance +=
            session.averageMakerQuoteDistance;
        totalAverageBookMidpointDeviation +=
            session.averageBookMidpointDeviation;
        totalAverageExternalOrderLifetime +=
            session.averageExternalOrderLifetime;
        result.totalBoughtVolume += session.boughtVolume;
        result.totalSoldVolume += session.soldVolume;
        result.totalTradingFees += session.tradingFees;
        result.totalTerminalLiquidationCosts +=
            session.terminalLiquidationCosts;
        result.allSessionsReconciled =
            result.allSessionsReconciled && session.ledgerReconciled;
        result.maximumQuotedSpread = std::max(
            result.maximumQuotedSpread,
            session.maximumQuotedSpread
        );
        result.totalExternalOrdersCanceled +=
            session.externalOrdersCanceled;
        result.totalExternalOrdersReplenished +=
            session.externalOrdersReplenished;

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
    result.medianPnL = median(pnlValues);
    result.averageFillRate = totalFillRate / sessionCount;
    result.averageMaximumInventoryExposure =
        totalMaximumInventoryExposure / sessionCount;
    result.averageMaximumDrawdown = totalMaximumDrawdown / sessionCount;
    result.averageQuotedSpread =
        totalAverageQuotedSpread / sessionCount;
    result.averageMakerQuoteDistance =
        totalAverageMakerQuoteDistance / sessionCount;
    result.averageBookMidpointDeviation =
        totalAverageBookMidpointDeviation / sessionCount;
    result.averageExternalOrderLifetime =
        totalAverageExternalOrderLifetime / sessionCount;
    result.profitableSessionPercentage =
        result.profitableSessions / sessionCount;

    double squaredDeviationTotal = 0.0;
    for (const double pnl : pnlValues) {
        const double deviation = pnl - result.averagePnL;
        squaredDeviationTotal += deviation * deviation;
    }
    result.pnlStandardDeviation =
        std::sqrt(squaredDeviationTotal / sessionCount);

    return result;
}

StrategyComparisonResult compareStrategies(const SimulationConfig& config) {
    SimulationConfig inventoryAwareConfig = config;
    inventoryAwareConfig.marketMaker.inventoryControlEnabled = true;
    inventoryAwareConfig.marketMaker.fixedSpreadEnabled = false;

    SimulationConfig fixedSpreadConfig = config;
    fixedSpreadConfig.marketMaker.inventoryControlEnabled = false;
    fixedSpreadConfig.marketMaker.fixedSpreadEnabled = true;

    return StrategyComparisonResult{
        runSimulation(inventoryAwareConfig),
        runSimulation(fixedSpreadConfig)
    };
}
