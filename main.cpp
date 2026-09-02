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
    std::cout << "Best P&L: " << result.bestPnL << '\n';
    std::cout << "Worst P&L: " << result.worstPnL << '\n';
    std::cout << "Profitable sessions: "
              << result.profitableSessions << " / "
              << result.numberOfSessions << '\n';

    std::cout << "\nAverage fill rate: "
              << result.averageFillRate * 100.0 << "%\n";
    std::cout << "Average max inventory exposure: "
              << result.averageMaximumInventoryExposure << '\n';
    std::cout << "Worst inventory exposure: "
              << result.worstInventoryExposure << '\n';

    std::cout << "\nTotal MM bought volume: "
              << result.totalBoughtVolume << '\n';
    std::cout << "Total MM sold volume: "
              << result.totalSoldVolume << '\n';

    return 0;
}
