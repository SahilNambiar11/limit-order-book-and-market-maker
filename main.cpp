#include "experiment.hpp"

#include <iostream>

int main() {
    const SimulationConfig config;
    const SimulationResult result = runSimulation(config);

    std::cout << "\n===== MARKET MAKER METRICS =====\n";
    std::cout << "Sessions: " << result.numberOfSessions << '\n';
    std::cout << "Steps per session: " << result.stepsPerSession << '\n';
    std::cout << "Total simulated steps: "
              << result.totalSimulatedSteps << '\n';

    std::cout << "\nAverage P&L: " << result.averagePnL << '\n';
    std::cout << "Median P&L: " << result.medianPnL << '\n';
    std::cout << "P&L standard deviation: "
              << result.pnlStandardDeviation << '\n';
    std::cout << "Best P&L: " << result.bestPnL << '\n';
    std::cout << "Worst P&L: " << result.worstPnL << '\n';
    std::cout << "Profitable sessions: "
              << result.profitableSessions << " / "
              << result.numberOfSessions << " ("
              << result.profitableSessionPercentage * 100.0 << "%)\n";

    std::cout << "\nAverage fill rate: "
              << result.averageFillRate * 100.0 << "%\n";
    std::cout << "Average max inventory exposure: "
              << result.averageMaximumInventoryExposure << '\n';
    std::cout << "Worst inventory exposure: "
              << result.worstInventoryExposure << '\n';
    std::cout << "Average maximum drawdown: "
              << result.averageMaximumDrawdown << '\n';
    std::cout << "Trading fees: " << result.totalTradingFees << '\n';
    std::cout << "Terminal liquidation costs: "
              << result.totalTerminalLiquidationCosts << '\n';
    std::cout << "All ledgers reconciled: "
              << (result.allSessionsReconciled ? "yes" : "no") << '\n';
    std::cout << "Average quoted spread: "
              << result.averageQuotedSpread << '\n';
    std::cout << "Maximum quoted spread: "
              << result.maximumQuotedSpread << '\n';
    std::cout << "Average maker quote distance: "
              << result.averageMakerQuoteDistance << '\n';
    std::cout << "Average book midpoint deviation: "
              << result.averageBookMidpointDeviation << '\n';
    std::cout << "External orders canceled: "
              << result.totalExternalOrdersCanceled << '\n';
    std::cout << "Average external order lifetime: "
              << result.averageExternalOrderLifetime << '\n';
    std::cout << "External orders replenished: "
              << result.totalExternalOrdersReplenished << '\n';

    std::cout << "\nTotal MM bought volume: "
              << result.totalBoughtVolume << '\n';
    std::cout << "Total MM sold volume: "
              << result.totalSoldVolume << '\n';

    return 0;
}
