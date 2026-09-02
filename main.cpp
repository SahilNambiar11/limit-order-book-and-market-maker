#include "orderbook.hpp"
#include "marketmaker.hpp"
#include "simulator.hpp"

#include <iostream>
#include <limits>
#include <cmath>

int main() {
    const int sessions = 100;
    const int stepsPerSession = 10000;

    double totalPnL = 0.0;

    double bestPnL =
        -std::numeric_limits<double>::infinity();

    double worstPnL =
        std::numeric_limits<double>::infinity();

    int profitableSessions = 0;

    double totalFillRate = 0.0;
    double totalMaxInventory = 0.0;

    int worstInventoryExposure = 0;

    long long totalBought = 0;
    long long totalSold = 0;

    for (int i = 0; i < sessions; i++) {
        OrderBook book;
        MarketMaker marketMaker(book);

        Simulator simulator(
            book,
            marketMaker,
            static_cast<unsigned int>(i + 1)
        );

        simulator.run(stepsPerSession);

        double pnl = marketMaker.getPnL();
        double fillRate = marketMaker.getFillRate();
        int maxInventory = marketMaker.getMaxAbsInventory();

        totalPnL += pnl;
        totalFillRate += fillRate;
        totalMaxInventory += maxInventory;

        totalBought += marketMaker.getTotalBought();
        totalSold += marketMaker.getTotalSold();

        if (pnl > 0.0) {
            profitableSessions++;
        }

        if (pnl > bestPnL) {
            bestPnL = pnl;
        }

        if (pnl < worstPnL) {
            worstPnL = pnl;
        }

        if (maxInventory > worstInventoryExposure) {
            worstInventoryExposure = maxInventory;
        }
    }

    double averagePnL =
        totalPnL / sessions;

    double averageFillRate =
        totalFillRate / sessions;

    double averageMaxInventory =
        totalMaxInventory / sessions;

    std::cout << "\n===== MARKET MAKER METRICS =====\n";

    std::cout << "Sessions: "
              << sessions << '\n';

    std::cout << "Steps per session: "
              << stepsPerSession << '\n';

    std::cout << "Total simulated steps: "
              << static_cast<long long>(sessions) * stepsPerSession
              << '\n';

    std::cout << "\nAverage P&L: "
              << averagePnL << '\n';

    std::cout << "Best P&L: "
              << bestPnL << '\n';

    std::cout << "Worst P&L: "
              << worstPnL << '\n';

    std::cout << "Profitable sessions: "
              << profitableSessions
              << " / "
              << sessions
              << '\n';

    std::cout << "\nAverage fill rate: "
              << averageFillRate * 100.0
              << "%\n";

    std::cout << "Average max inventory exposure: "
              << averageMaxInventory
              << '\n';

    std::cout << "Worst inventory exposure: "
              << worstInventoryExposure
              << '\n';

    std::cout << "\nTotal MM bought volume: "
              << totalBought
              << '\n';

    std::cout << "Total MM sold volume: "
              << totalSold
              << '\n';

    return 0;
}
