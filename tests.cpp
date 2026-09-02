#include "experiment.hpp"
#include "orderbook.hpp"

#include <cassert>
#include <cmath>
#include <iostream>
#include <stdexcept>

namespace {

bool nearlyEqual(double left, double right) {
    return std::abs(left - right) < 1e-12;
}

void testPriceTimePriorityPartialFillsAndCancellation() {
    OrderBook book;
    book.addOrder(Order(1, Side::Sell, OrderType::Limit, 100.0, 5, 1));
    book.addOrder(Order(2, Side::Sell, OrderType::Limit, 100.0, 7, 2));
    book.addOrder(Order(3, Side::Buy, OrderType::Market, 0.0, 8, 3));

    const auto& trades = book.getTrades();
    assert(trades.size() == 2);
    assert(trades[0].sellOrderId == 1);
    assert(trades[0].quantity == 5);
    assert(trades[1].sellOrderId == 2);
    assert(trades[1].quantity == 3);
    assert(nearlyEqual(book.bestAsk(), 100.0));

    assert(book.cancelOrder(2));
    assert(nearlyEqual(book.bestAsk(), 0.0));
    assert(!book.cancelOrder(999));
}

void testLimitPriceProtectionAndMarketSell() {
    OrderBook book;
    book.addOrder(Order(1, Side::Sell, OrderType::Limit, 101.0, 10, 1));
    book.addOrder(Order(2, Side::Buy, OrderType::Limit, 100.0, 10, 2));

    assert(book.getTrades().empty());
    assert(nearlyEqual(book.bestBid(), 100.0));
    assert(nearlyEqual(book.bestAsk(), 101.0));

    book.addOrder(Order(3, Side::Sell, OrderType::Market, 0.0, 4, 3));
    assert(book.getTrades().size() == 1);
    assert(book.getTrades()[0].buyOrderId == 2);
    assert(book.getTrades()[0].quantity == 4);
    assert(nearlyEqual(book.getTrades()[0].price, 100.0));
}

void testStructuredSimulationIsDeterministic() {
    SimulationConfig config;
    config.numberOfSessions = 4;
    config.stepsPerSession = 500;
    config.marketMaker.quoteSize = 7;
    config.marketMaker.maximumInventory = 25;
    config.simulator.randomSeed = 42;
    config.simulator.marketOrderProbability = 0.35;
    config.simulator.maximumGeneratedOrderQuantity = 12;

    const SimulationResult first = runSimulation(config);
    const SimulationResult second = runSimulation(config);

    assert(first.numberOfSessions == 4);
    assert(first.totalSimulatedSteps == 2000);
    assert(first.sessionResults.size() == 4);
    assert(nearlyEqual(first.averagePnL, second.averagePnL));
    assert(first.totalBoughtVolume == second.totalBoughtVolume);
    assert(first.totalSoldVolume == second.totalSoldVolume);

    std::int64_t boughtVolume = 0;
    std::int64_t soldVolume = 0;
    for (std::size_t i = 0; i < first.sessionResults.size(); ++i) {
        const SessionResult& session = first.sessionResults[i];
        assert(session.sessionNumber == static_cast<int>(i + 1));
        assert(session.randomSeed == 42 + i);
        assert(nearlyEqual(session.pnl, second.sessionResults[i].pnl));
        boughtVolume += session.boughtVolume;
        soldVolume += session.soldVolume;
    }
    assert(boughtVolume == first.totalBoughtVolume);
    assert(soldVolume == first.totalSoldVolume);
}

void testInvalidConfigurationIsRejected() {
    SimulationConfig config;
    config.simulator.marketOrderProbability = 1.1;

    bool threw = false;
    try {
        (void)runSimulation(config);
    }
    catch (const std::invalid_argument&) {
        threw = true;
    }
    assert(threw);
}

}  // namespace

int main() {
    testPriceTimePriorityPartialFillsAndCancellation();
    testLimitPriceProtectionAndMarketSell();
    testStructuredSimulationIsDeterministic();
    testInvalidConfigurationIsRejected();
    std::cout << "All tests passed.\n";
    return 0;
}
