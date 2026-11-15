#include "experiment.hpp"
#include "orderbook.hpp"

#include <cassert>
#include <cmath>
#include <iostream>
#include <stdexcept>

namespace {

bool nearlyEqual(double left, double right, double tolerance = 1e-8) {
    return std::abs(left - right) < tolerance;
}

void addTwoSidedMarket(OrderBook& book) {
    book.addOrder(Order(1, Side::Buy, OrderType::Limit, 99.0, 100, 0));
    book.addOrder(Order(2, Side::Sell, OrderType::Limit, 101.0, 100, 0));
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

void testFinalStepFillIsProcessedExactlyOnce() {
    OrderBook book;
    addTwoSidedMarket(book);

    MarketMakerConfig config;
    config.transactionFeePerUnit = 0.0;
    config.terminalLiquidationEnabled = false;
    MarketMaker maker(book, config);
    maker.setReferencePrice(100.0);
    maker.update(1, 10, 11);

    book.addOrder(Order(12, Side::Buy, OrderType::Market, 0.0, 4, 2));
    maker.finalize(100.0);

    assert(maker.getTotalBought() == 0);
    assert(maker.getTotalSold() == 4);
    assert(maker.getInventory() == -4);
    assert(nearlyEqual(maker.getCash(), 403.96));
    assert(nearlyEqual(maker.getPnL(), 3.96));
    assert(maker.isLedgerReconciled());

    maker.finalize(100.0);
    assert(maker.getTotalSold() == 4);
    assert(nearlyEqual(maker.getCash(), 403.96));
}

void testFillGeneratedByFinalQuoteIsProcessed() {
    OrderBook book;
    book.addOrder(Order(1, Side::Buy, OrderType::Limit, 100.0, 1, 0));
    book.addOrder(Order(2, Side::Sell, OrderType::Limit, 100.01, 100, 0));

    MarketMakerConfig config;
    config.quoteSize = 10;
    config.maximumInventory = 10;
    config.transactionFeePerUnit = 0.0;
    config.terminalLiquidationEnabled = false;
    MarketMaker maker(book, config);
    maker.setReferencePrice(100.0);
    maker.update(1, 10, 11);

    book.addOrder(Order(12, Side::Sell, OrderType::Market, 0.0, 7, 2));
    book.addOrder(Order(13, Side::Buy, OrderType::Limit, 100.0, 100, 2));
    maker.update(2, 14, 15);

    assert(maker.getTotalBought() == 6);
    assert(maker.getTotalSold() == 10);
    assert(maker.getInventory() == -4);
    assert(maker.isLedgerReconciled());
}

void testHardInventoryLimitAcrossConsecutiveAndTwoSidedFills() {
    OrderBook book;
    addTwoSidedMarket(book);

    MarketMakerConfig config;
    config.quoteSize = 10;
    config.maximumInventory = 5;
    config.transactionFeePerUnit = 0.0;
    config.terminalLiquidationEnabled = false;
    MarketMaker maker(book, config);
    maker.setReferencePrice(100.0);
    maker.update(1, 10, 11);

    book.addOrder(Order(12, Side::Sell, OrderType::Market, 0.0, 10, 2));
    book.addOrder(Order(13, Side::Buy, OrderType::Market, 0.0, 10, 2));
    maker.update(2, 14, 15);

    assert(maker.getTotalBought() == 5);
    assert(maker.getTotalSold() == 5);
    assert(maker.getInventory() == 0);
    assert(maker.getMaxAbsInventory() == 5);

    book.addOrder(Order(16, Side::Sell, OrderType::Market, 0.0, 10, 3));
    maker.update(3, 17, 18);
    book.addOrder(Order(19, Side::Sell, OrderType::Market, 0.0, 10, 4));
    maker.update(4, 20, 21);

    assert(std::abs(maker.getInventory()) <= 5);
    assert(maker.getMaxAbsInventory() <= 5);
    assert(maker.isLedgerReconciled());
}

void testLatencyPreservesInventoryLimit() {
    OrderBook book;
    addTwoSidedMarket(book);

    MarketMakerConfig config;
    config.quoteSize = 20;
    config.maximumInventory = 7;
    config.quoteLatencySteps = 2;
    config.transactionFeePerUnit = 0.0;
    config.terminalLiquidationEnabled = false;
    MarketMaker maker(book, config);
    maker.setReferencePrice(100.0);
    maker.update(1, 10, 11);
    maker.beginStep(2);
    maker.beginStep(3);

    book.addOrder(Order(12, Side::Sell, OrderType::Market, 0.0, 20, 3));
    maker.update(3, 13, 14);
    maker.finalize(100.0);

    assert(maker.getInventory() == 7);
    assert(maker.getMaxAbsInventory() == 7);
    assert(maker.getTotalBought() == 7);
    assert(maker.isLedgerReconciled());
}

void testReferenceValuationOnOneSidedBook() {
    OrderBook book;
    addTwoSidedMarket(book);

    MarketMakerConfig config;
    config.transactionFeePerUnit = 0.0;
    config.terminalLiquidationEnabled = false;
    MarketMaker maker(book, config);
    maker.setReferencePrice(100.0);
    maker.update(1, 10, 11);
    book.addOrder(Order(12, Side::Sell, OrderType::Market, 0.0, 5, 2));
    assert(book.cancelOrder(2));
    maker.update(2, 13, 14);
    maker.setReferencePrice(105.0);
    maker.finalize(105.0);

    assert(book.bestBid() == 99.0);
    assert(book.bestAsk() == 0.0);
    assert(maker.getInventory() == 5);
    assert(nearlyEqual(maker.getInventoryValue(), 525.0));
    assert(nearlyEqual(maker.getPnL(), maker.getCash() + 525.0));
    assert(nearlyEqual(
        maker.getPnL(),
        maker.getRealizedPnL() + maker.getUnrealizedPnL()
    ));
    assert(maker.isLedgerReconciled());
}

void testFeesAndTerminalLiquidation() {
    OrderBook book;
    addTwoSidedMarket(book);

    MarketMakerConfig config;
    config.quoteSize = 5;
    config.transactionFeePerUnit = 0.02;
    config.terminalLiquidationEnabled = true;
    config.terminalLiquidationCostPerUnit = 0.10;
    MarketMaker maker(book, config);
    maker.setReferencePrice(100.0);
    maker.update(1, 10, 11);
    book.addOrder(Order(12, Side::Sell, OrderType::Market, 0.0, 5, 2));
    maker.finalize(100.0);

    assert(maker.getInventory() == 0);
    assert(maker.getTerminalLiquidationQuantity() == 5);
    assert(nearlyEqual(maker.getTradingFees(), 0.20));
    assert(nearlyEqual(maker.getTerminalLiquidationCosts(), 0.50));
    assert(nearlyEqual(maker.getRealizedPnL(), 4.25));
    assert(nearlyEqual(maker.getUnrealizedPnL(), 0.0));
    assert(nearlyEqual(maker.getPnL(), 4.25));
    assert(nearlyEqual(maker.getCash(), 4.25));
    assert(maker.isLedgerReconciled());
}

void testEmptyBookHasValidIndependentMark() {
    OrderBook book;
    MarketMakerConfig config;
    config.terminalLiquidationEnabled = false;
    MarketMaker maker(book, config);
    maker.setReferencePrice(123.45);

    assert(nearlyEqual(book.bestBid(), 0.0));
    assert(nearlyEqual(book.bestAsk(), 0.0));
    assert(nearlyEqual(maker.getReferencePrice(), 123.45));
    assert(nearlyEqual(maker.getPnL(), 0.0));
    assert(maker.isLedgerReconciled());
}

void testStructuredSimulationIsDeterministicAndReconciled() {
    SimulationConfig config;
    config.numberOfSessions = 8;
    config.stepsPerSession = 1000;
    config.marketMaker.quoteSize = 7;
    config.marketMaker.maximumInventory = 25;
    config.marketMaker.transactionFeePerUnit = 0.005;
    config.marketMaker.quoteLatencySteps = 1;
    config.simulator.randomSeed = 42;
    config.simulator.marketOrderProbability = 0.35;
    config.simulator.informedOrderProbability = 0.30;
    config.simulator.referencePriceVolatility = 0.05;
    config.simulator.maximumGeneratedOrderQuantity = 12;

    const SimulationResult first = runSimulation(config);
    const SimulationResult second = runSimulation(config);

    assert(first.numberOfSessions == 8);
    assert(first.totalSimulatedSteps == 8000);
    assert(first.sessionResults.size() == 8);
    assert(nearlyEqual(first.averagePnL, second.averagePnL));
    assert(nearlyEqual(first.medianPnL, second.medianPnL));
    assert(first.totalBoughtVolume == second.totalBoughtVolume);
    assert(first.totalSoldVolume == second.totalSoldVolume);
    assert(first.allSessionsReconciled);

    std::int64_t boughtVolume = 0;
    std::int64_t soldVolume = 0;
    for (std::size_t i = 0; i < first.sessionResults.size(); ++i) {
        const SessionResult& session = first.sessionResults[i];
        assert(session.sessionNumber == static_cast<int>(i + 1));
        assert(session.randomSeed == 42 + i);
        assert(nearlyEqual(session.pnl, second.sessionResults[i].pnl));
        assert(nearlyEqual(session.pnl, session.endingEquity));
        assert(nearlyEqual(
            session.pnl,
            session.realizedPnL + session.unrealizedPnL
        ));
        assert(session.maximumInventoryExposure <= 25);
        assert(session.ledgerReconciled);
        boughtVolume += session.boughtVolume;
        soldVolume += session.soldVolume;
    }
    assert(boughtVolume == first.totalBoughtVolume);
    assert(soldVolume == first.totalSoldVolume);
}

void testStrategyComparisonUsesMatchingSeeds() {
    SimulationConfig config;
    config.numberOfSessions = 3;
    config.stepsPerSession = 200;
    config.simulator.randomSeed = 91;

    const StrategyComparisonResult comparison = compareStrategies(config);
    assert(comparison.inventoryAware.sessionResults.size() == 3);
    assert(comparison.fixedSpread.sessionResults.size() == 3);
    for (std::size_t i = 0; i < 3; ++i) {
        assert(
            comparison.inventoryAware.sessionResults[i].randomSeed ==
            comparison.fixedSpread.sessionResults[i].randomSeed
        );
    }
    assert(comparison.inventoryAware.allSessionsReconciled);
    assert(comparison.fixedSpread.allSessionsReconciled);
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

    config.simulator.marketOrderProbability = 0.2;
    config.marketMaker.transactionFeePerUnit = -0.01;
    threw = false;
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
    testFinalStepFillIsProcessedExactlyOnce();
    testFillGeneratedByFinalQuoteIsProcessed();
    testHardInventoryLimitAcrossConsecutiveAndTwoSidedFills();
    testLatencyPreservesInventoryLimit();
    testReferenceValuationOnOneSidedBook();
    testFeesAndTerminalLiquidation();
    testEmptyBookHasValidIndependentMark();
    testStructuredSimulationIsDeterministicAndReconciled();
    testStrategyComparisonUsesMatchingSeeds();
    testInvalidConfigurationIsRejected();
    std::cout << "All tests passed.\n";
    return 0;
}
