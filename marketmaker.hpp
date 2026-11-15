#ifndef MARKETMAKER_HPP
#define MARKETMAKER_HPP

#include "orderbook.hpp"
#include <cstddef>
#include <unordered_set>

struct MarketMakerConfig {
    int quoteSize = 10;
    int maximumInventory = 100;
    double transactionFeePerUnit = 0.001;
    int quoteLatencySteps = 0;
    bool terminalLiquidationEnabled = true;
    double terminalLiquidationCostPerUnit = 0.01;
    bool inventoryControlEnabled = true;
    bool fixedSpreadEnabled = false;
    double fixedSpread = 0.04;
};

class MarketMaker {
public:
    MarketMaker(
        OrderBook& book,
        const MarketMakerConfig& config = MarketMakerConfig{}
    );

    void setReferencePrice(double price);
    void beginStep(long long currentTime);
    void update(long long currentTime, int bidID, int askID);
    void finalize(double terminalReferencePrice);

    int getInventory() const;
    double getCash() const;

    int getTotalBought() const;
    int getTotalSold() const;

    double getMidPrice() const;
    double getReferencePrice() const;
    double getInventoryValue() const;
    double getRealizedPnL() const;
    double getUnrealizedPnL() const;
    double getPnL() const;
    double getTradingFees() const;
    double getTerminalLiquidationCosts() const;
    int getTerminalLiquidationQuantity() const;
    double getMaximumDrawdown() const;
    bool isLedgerReconciled() const;

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
    double transactionFeePerUnit;
    int quoteLatencySteps;
    bool terminalLiquidationEnabled;
    double terminalLiquidationCostPerUnit;
    bool inventoryControlEnabled;
    bool fixedSpreadEnabled;
    double fixedSpread;

    std::size_t lastProcessedTrade;
    double referencePrice;
    double averageEntryPrice;
    double realizedPnL;
    double tradingFees;
    double terminalLiquidationCosts;
    int terminalLiquidationQuantity;
    int terminalLiquidationInventoryChange;
    double grossTradingCashFlow;
    double terminalLiquidationCashFlow;
    double peakEquity;
    double maximumDrawdown;

    std::unordered_set<int> bidOrderIDs;
    std::unordered_set<int> askOrderIDs;

    struct PendingQuotes {
        bool active = false;
        long long activationTime = 0;
        int bidID = -1;
        int askID = -1;
        double bidPrice = 0.0;
        double askPrice = 0.0;
    };

    PendingQuotes pendingQuotes;

    void processTrades();
    void cancelOldQuotes();
    void scheduleQuotes(long long currentTime, int bidID, int askID);
    void activatePendingQuotes(long long currentTime);
    void placeQuotes(
        long long currentTime,
        int bidID,
        int askID,
        double bidPrice,
        double askPrice
    );
    void recordFill(Side side, double price, int quantity, bool liquidation);
    void updatePosition(Side side, double price, int quantity);
    void updateRiskMetrics();
};

#endif
