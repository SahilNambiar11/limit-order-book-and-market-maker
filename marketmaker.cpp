#include "marketmaker.hpp"

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
      lastProcessedTrade(0) {
}

void MarketMaker::update(long long currentTime, int bidID, int askID) {
    processTrades();
    cancelOldQuotes();
    placeQuotes(currentTime, bidID, askID);
}

void MarketMaker::processTrades() {
    const std::vector<Trade>& trades = book.getTrades();

    for (std::size_t i = lastProcessedTrade; i < trades.size(); i++) {
        const Trade& trade = trades[i];

        if (trade.buyOrderId == bidOrderID) {
            inventory += trade.quantity;
            cash -= trade.price * trade.quantity;

            totalBought += trade.quantity;
            totalFilledQuantity += trade.quantity;
        }

        if (trade.sellOrderId == askOrderID) {
            inventory -= trade.quantity;
            cash += trade.price * trade.quantity;

            totalSold += trade.quantity;
            totalFilledQuantity += trade.quantity;
        }
    }

    int absInventory = std::abs(inventory);

    if (absInventory > maxAbsInventory) {
        maxAbsInventory = absInventory;
    }

    lastProcessedTrade = trades.size();
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

void MarketMaker::placeQuotes(long long currentTime, int bidID, int askID) {
    double bestBid = book.bestBid();
    double bestAsk = book.bestAsk();

    if (bestBid == 0.0 || bestAsk == 0.0) {
        return;
    }

    double bidPrice;
    double askPrice;

    double spread = bestAsk - bestBid;

    if (spread > 0.02) {
        bidPrice = bestBid + 0.01;
        askPrice = bestAsk - 0.01;
    }
    else {
        bidPrice = bestBid;
        askPrice = bestAsk;
    }

    if (inventory > maxInventory / 2) {
        bidPrice -= 0.01;
        askPrice -= 0.01;
    }
    else if (inventory < -maxInventory / 2) {
        bidPrice += 0.01;
        askPrice += 0.01;
    }

    bidPrice = std::round(bidPrice * 100.0) / 100.0;
    askPrice = std::round(askPrice * 100.0) / 100.0;

    if (inventory < maxInventory) {
        bidOrderID = bidID;

        Order bidOrder(
            bidOrderID,
            Side::Buy,
            OrderType::Limit,
            bidPrice,
            quoteSize,
            currentTime
        );

        book.addOrder(bidOrder);

        totalQuotedQuantity += quoteSize;
    }

    if (inventory > -maxInventory) {
        askOrderID = askID;

        Order askOrder(
            askOrderID,
            Side::Sell,
            OrderType::Limit,
            askPrice,
            quoteSize,
            currentTime
        );

        book.addOrder(askOrder);

        totalQuotedQuantity += quoteSize;
    }
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
    double bestBid = book.bestBid();
    double bestAsk = book.bestAsk();

    if (bestBid == 0.0 || bestAsk == 0.0) {
        return 0.0;
    }

    return (bestBid + bestAsk) / 2.0;
}

double MarketMaker::getPnL() const {
    double midPrice = getMidPrice();

    if (midPrice == 0.0) {
        return cash;
    }

    return cash + inventory * midPrice;
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
