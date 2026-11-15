#include "marketmaker.hpp"

#include <algorithm>
#include <cmath>
#include <vector>

MarketMaker::MarketMaker(
    OrderBook& book,
    const MarketMakerConfig& config
)
    : book(book),
      inventory(0),
      cash(0.0),
      totalBought(0),
      totalSold(0),
      totalQuotedQuantity(0),
      totalFilledQuantity(0),
      maxAbsInventory(0),
      bidOrderID(-1),
      askOrderID(-1),
      quoteSize(config.quoteSize),
      maxInventory(config.maximumInventory),
      transactionFeePerUnit(config.transactionFeePerUnit),
      quoteLatencySteps(config.quoteLatencySteps),
      terminalLiquidationEnabled(config.terminalLiquidationEnabled),
      terminalLiquidationCostPerUnit(
          config.terminalLiquidationCostPerUnit
      ),
      inventoryControlEnabled(config.inventoryControlEnabled),
      fixedSpreadEnabled(config.fixedSpreadEnabled),
      fixedSpread(config.fixedSpread),
      lastProcessedTrade(0),
      referencePrice(100.0),
      averageEntryPrice(0.0),
      realizedPnL(0.0),
      tradingFees(0.0),
      terminalLiquidationCosts(0.0),
      terminalLiquidationQuantity(0),
      terminalLiquidationInventoryChange(0),
      grossTradingCashFlow(0.0),
      terminalLiquidationCashFlow(0.0),
      peakEquity(0.0),
      maximumDrawdown(0.0) {
}

void MarketMaker::setReferencePrice(double price) {
    referencePrice = price;
}

void MarketMaker::beginStep(long long currentTime) {
    processTrades();
    activatePendingQuotes(currentTime);
    processTrades();
}

void MarketMaker::update(long long currentTime, int bidID, int askID) {
    processTrades();
    scheduleQuotes(currentTime, bidID, askID);
    processTrades();
    updateRiskMetrics();
}

void MarketMaker::finalize(double terminalReferencePrice) {
    setReferencePrice(terminalReferencePrice);
    processTrades();
    cancelOldQuotes();
    pendingQuotes.active = false;

    if (terminalLiquidationEnabled && inventory != 0) {
        const int quantity = std::abs(inventory);
        const Side side = inventory > 0 ? Side::Sell : Side::Buy;
        recordFill(side, referencePrice, quantity, true);

        const double liquidationCost =
            terminalLiquidationCostPerUnit * quantity;
        terminalLiquidationCosts += liquidationCost;
        cash -= liquidationCost;
        terminalLiquidationQuantity += quantity;
    }

    updateRiskMetrics();
}

void MarketMaker::processTrades() {
    const std::vector<Trade>& trades = book.getTrades();

    for (std::size_t i = lastProcessedTrade; i < trades.size(); ++i) {
        const Trade& trade = trades[i];

        if (bidOrderIDs.count(trade.buyOrderId) != 0) {
            recordFill(Side::Buy, trade.price, trade.quantity, false);
        }

        if (askOrderIDs.count(trade.sellOrderId) != 0) {
            recordFill(Side::Sell, trade.price, trade.quantity, false);
        }
    }

    lastProcessedTrade = trades.size();
}

void MarketMaker::recordFill(
    Side side,
    double price,
    int quantity,
    bool liquidation
) {
    const double signedCashFlow = side == Side::Buy
        ? -price * quantity
        : price * quantity;
    const double fee = transactionFeePerUnit * quantity;

    cash += signedCashFlow - fee;
    tradingFees += fee;

    if (liquidation) {
        terminalLiquidationCashFlow += signedCashFlow;
        terminalLiquidationInventoryChange += side == Side::Buy
            ? quantity
            : -quantity;
    }
    else {
        grossTradingCashFlow += signedCashFlow;
        totalFilledQuantity += quantity;

        if (side == Side::Buy) {
            totalBought += quantity;
        }
        else {
            totalSold += quantity;
        }
    }

    updatePosition(side, price, quantity);
}

void MarketMaker::updatePosition(
    Side side,
    double price,
    int quantity
) {
    if (side == Side::Buy) {
        if (inventory >= 0) {
            const int newInventory = inventory + quantity;
            averageEntryPrice = newInventory == 0
                ? 0.0
                : (averageEntryPrice * inventory + price * quantity)
                    / newInventory;
            inventory = newInventory;
        }
        else {
            const int closedQuantity = std::min(quantity, -inventory);
            realizedPnL +=
                (averageEntryPrice - price) * closedQuantity;
            inventory += quantity;

            if (inventory > 0) {
                averageEntryPrice = price;
            }
            else if (inventory == 0) {
                averageEntryPrice = 0.0;
            }
        }
    }
    else {
        if (inventory <= 0) {
            const int oldShortQuantity = -inventory;
            const int newShortQuantity = oldShortQuantity + quantity;
            averageEntryPrice = newShortQuantity == 0
                ? 0.0
                : (averageEntryPrice * oldShortQuantity + price * quantity)
                    / newShortQuantity;
            inventory -= quantity;
        }
        else {
            const int closedQuantity = std::min(quantity, inventory);
            realizedPnL +=
                (price - averageEntryPrice) * closedQuantity;
            inventory -= quantity;

            if (inventory < 0) {
                averageEntryPrice = price;
            }
            else if (inventory == 0) {
                averageEntryPrice = 0.0;
            }
        }
    }

    maxAbsInventory = std::max(maxAbsInventory, std::abs(inventory));
}

void MarketMaker::cancelOldQuotes() {
    if (bidOrderID != -1) {
        book.cancelOrder(bidOrderID);
        bidOrderID = -1;
    }

    if (askOrderID != -1) {
        book.cancelOrder(askOrderID);
        askOrderID = -1;
    }
}

void MarketMaker::scheduleQuotes(
    long long currentTime,
    int bidID,
    int askID
) {
    if (quoteLatencySteps > 0 && pendingQuotes.active) {
        return;
    }

    const double externalBestBid =
        book.bestBidExcluding(bidOrderID, askOrderID);
    const double externalBestAsk =
        book.bestAskExcluding(bidOrderID, askOrderID);

    double bidPrice = referencePrice - fixedSpread / 2.0;
    double askPrice = referencePrice + fixedSpread / 2.0;

    if (!fixedSpreadEnabled &&
        externalBestBid != 0.0 && externalBestAsk != 0.0) {
        const double spread = externalBestAsk - externalBestBid;

        if (spread > 0.02) {
            bidPrice = externalBestBid + 0.01;
            askPrice = externalBestAsk - 0.01;
        }
        else {
            bidPrice = externalBestBid;
            askPrice = externalBestAsk;
        }
    }

    if (inventoryControlEnabled) {
        if (inventory > maxInventory / 2) {
            bidPrice -= 0.01;
            askPrice -= 0.01;
        }
        else if (inventory < -maxInventory / 2) {
            bidPrice += 0.01;
            askPrice += 0.01;
        }
    }

    bidPrice = std::max(
        0.01,
        std::round(bidPrice * 100.0) / 100.0
    );
    askPrice = std::max(
        0.01,
        std::round(askPrice * 100.0) / 100.0
    );

    if (quoteLatencySteps == 0) {
        cancelOldQuotes();
        placeQuotes(
            currentTime,
            bidID,
            askID,
            bidPrice,
            askPrice
        );
        return;
    }

    pendingQuotes.active = true;
    pendingQuotes.activationTime = currentTime + quoteLatencySteps;
    pendingQuotes.bidID = bidID;
    pendingQuotes.askID = askID;
    pendingQuotes.bidPrice = bidPrice;
    pendingQuotes.askPrice = askPrice;
}

void MarketMaker::activatePendingQuotes(long long currentTime) {
    if (!pendingQuotes.active ||
        pendingQuotes.activationTime > currentTime) {
        return;
    }

    const PendingQuotes quotes = pendingQuotes;
    pendingQuotes.active = false;
    cancelOldQuotes();
    placeQuotes(
        currentTime,
        quotes.bidID,
        quotes.askID,
        quotes.bidPrice,
        quotes.askPrice
    );
}

void MarketMaker::placeQuotes(
    long long currentTime,
    int bidID,
    int askID,
    double bidPrice,
    double askPrice
) {
    const int bidQuantity = std::min(
        quoteSize,
        std::max(0, maxInventory - inventory)
    );
    const int askQuantity = std::min(
        quoteSize,
        std::max(0, maxInventory + inventory)
    );

    if (bidQuantity > 0) {
        bidOrderID = bidID;
        bidOrderIDs.insert(bidOrderID);
        book.addOrder(Order(
            bidOrderID,
            Side::Buy,
            OrderType::Limit,
            bidPrice,
            bidQuantity,
            currentTime
        ));
        totalQuotedQuantity += bidQuantity;
    }

    if (askQuantity > 0) {
        askOrderID = askID;
        askOrderIDs.insert(askOrderID);
        book.addOrder(Order(
            askOrderID,
            Side::Sell,
            OrderType::Limit,
            askPrice,
            askQuantity,
            currentTime
        ));
        totalQuotedQuantity += askQuantity;
    }
}

void MarketMaker::updateRiskMetrics() {
    const double equity = getPnL();
    peakEquity = std::max(peakEquity, equity);
    maximumDrawdown = std::max(maximumDrawdown, peakEquity - equity);
}

int MarketMaker::getInventory() const {
    return inventory;
}

double MarketMaker::getCash() const {
    return cash;
}

int MarketMaker::getTotalBought() const {
    return totalBought;
}

int MarketMaker::getTotalSold() const {
    return totalSold;
}

double MarketMaker::getMidPrice() const {
    const double bestBid = book.bestBid();
    const double bestAsk = book.bestAsk();

    if (bestBid == 0.0 || bestAsk == 0.0) {
        return 0.0;
    }

    return (bestBid + bestAsk) / 2.0;
}

double MarketMaker::getReferencePrice() const {
    return referencePrice;
}

double MarketMaker::getInventoryValue() const {
    return inventory * referencePrice;
}

double MarketMaker::getRealizedPnL() const {
    return realizedPnL - tradingFees - terminalLiquidationCosts;
}

double MarketMaker::getUnrealizedPnL() const {
    return inventory * (referencePrice - averageEntryPrice);
}

double MarketMaker::getPnL() const {
    return cash + getInventoryValue();
}

double MarketMaker::getTradingFees() const {
    return tradingFees;
}

double MarketMaker::getTerminalLiquidationCosts() const {
    return terminalLiquidationCosts;
}

int MarketMaker::getTerminalLiquidationQuantity() const {
    return terminalLiquidationQuantity;
}

double MarketMaker::getMaximumDrawdown() const {
    return maximumDrawdown;
}

bool MarketMaker::isLedgerReconciled() const {
    int ledgerBought = 0;
    int ledgerSold = 0;
    double ledgerCashFlow = 0.0;

    for (const Trade& trade : book.getTrades()) {
        if (bidOrderIDs.count(trade.buyOrderId) != 0) {
            ledgerBought += trade.quantity;
            ledgerCashFlow -= trade.price * trade.quantity;
        }
        if (askOrderIDs.count(trade.sellOrderId) != 0) {
            ledgerSold += trade.quantity;
            ledgerCashFlow += trade.price * trade.quantity;
        }
    }

    const double expectedCash = ledgerCashFlow
        + terminalLiquidationCashFlow
        - tradingFees
        - terminalLiquidationCosts;
    const bool inventoryReconciled = inventory ==
        ledgerBought - ledgerSold + terminalLiquidationInventoryChange;

    return ledgerBought == totalBought &&
        ledgerSold == totalSold &&
        inventoryReconciled &&
        std::abs(ledgerCashFlow - grossTradingCashFlow) < 1e-8 &&
        std::abs(expectedCash - cash) < 1e-8 &&
        std::abs(getPnL() - (getRealizedPnL() + getUnrealizedPnL()))
            < 1e-8;
}

double MarketMaker::getFillRate() const {
    if (totalQuotedQuantity == 0) {
        return 0.0;
    }

    return static_cast<double>(totalFilledQuantity)
           / totalQuotedQuantity;
}

int MarketMaker::getMaxAbsInventory() const {
    return maxAbsInventory;
}
