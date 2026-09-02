#ifndef MARKETMAKER_HPP
#define MARKETMAKER_HPP

#include "orderbook.hpp"
#include <cstddef>

class MarketMaker {
public:
    MarketMaker(OrderBook& book);

    void update(long long currentTime, int bidID, int askID);

    int getInventory() const;
    double getCash() const;

    int getTotalBought() const;
    int getTotalSold() const;

    double getMidPrice() const;
    double getPnL() const;

    double getFillRate() const;
    int getMaxAbsInventory() const;

private:
    OrderBook& book;

    int inventory;
    double cash;

    int totalBought;
    int totalSold;

    long long totalQuotedQuantity;
    long long totalFilledQuantity;

    int maxAbsInventory;

    int bidOrderID;
    int askOrderID;

    int quoteSize;
    int maxInventory;

    std::size_t lastProcessedTrade;

    void processTrades();
    void cancelOldQuotes();
    void placeQuotes(long long currentTime, int bidID, int askID);
};

#endif
