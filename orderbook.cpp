#include "orderbook.hpp"
#include <iostream>
#include <algorithm>
#include <iomanip>

OrderBook::OrderBook() {}

void OrderBook::addOrder(Order order) {
  tryMatch(order);
  
  if (order.quantity > 0 && order.type == OrderType::Limit) {
    if (order.side == Side::Buy) {
      bids[order.price].push_back(order);
    } 
    else if (order.side == Side::Sell) {
      asks[order.price].push_back(order);
    }
  }
}

void OrderBook::tryMatch(Order& order) {
  if (order.side == Side::Buy) {
    while (order.quantity > 0 && !asks.empty()) {
      auto bestAsksLevel = asks.begin();
      double bestAskPrice = bestAsksLevel->first;

      if (order.type == OrderType::Limit && order.price < bestAskPrice) {
        break;
      }

      Order& restingOrder = bestAsksLevel->second.front();

      int tradeQuantity = std::min(order.quantity, restingOrder.quantity);

      Trade trade(nextTradeID++, order.id, restingOrder.id, bestAskPrice, tradeQuantity, order.timestamp);
      
      trades.push_back(trade);

      order.quantity -= tradeQuantity;
      restingOrder.quantity -= tradeQuantity;
      
      if (restingOrder.quantity == 0) {
        bestAsksLevel->second.pop_front();
      }

      if (bestAsksLevel->second.empty()) {
        asks.erase(bestAsksLevel);
      }
    }
  }
  else {
    while (order.quantity > 0 && !bids.empty()) {

      auto bestBidLevel = bids.begin();

      double bestBidPrice = bestBidLevel->first;

      if (order.type == OrderType::Limit && order.price > bestBidPrice) {
        break;
      }

      Order& restingOrder = bestBidLevel->second.front();

      int tradeQuantity = std::min(order.quantity, restingOrder.quantity);

      Trade trade(nextTradeID++, restingOrder.id, order.id, bestBidPrice, tradeQuantity, order.timestamp);

      trades.push_back(trade);
            
      order.quantity -= tradeQuantity;
      restingOrder.quantity -= tradeQuantity;

      if (restingOrder.quantity == 0) {
        bestBidLevel->second.pop_front();
      }

      if (bestBidLevel->second.empty()) {
        bids.erase(bestBidLevel);
      }
    }
  }
}

void OrderBook::printBook() const {
    std::cout << "\n========== ORDER BOOK ==========\n\n";

    std::cout << "ASKS\n";
    std::cout << std::left
              << std::setw(12) << "Price"
              << std::setw(12) << "Total Qty"
              << "Orders\n";

    std::cout << "---------------------------------------------\n";

    for (const auto& priceLevel : asks) {
        int totalQuantity = 0;

        for (const Order& order : priceLevel.second) {
            totalQuantity += order.quantity;
        }

        std::cout << std::left
                  << std::setw(12) << std::fixed << std::setprecision(2)
                  << priceLevel.first
                  << std::setw(12) << totalQuantity;

        for (const Order& order : priceLevel.second) {
            std::cout << "#" << order.id
                      << "(" << order.quantity << ") ";
        }

        std::cout << '\n';
    }

    std::cout << "\nBIDS\n";
    std::cout << std::left
              << std::setw(12) << "Price"
              << std::setw(12) << "Total Qty"
              << "Orders\n";

    std::cout << "---------------------------------------------\n";

    for (const auto& priceLevel : bids) {
        int totalQuantity = 0;

        for (const Order& order : priceLevel.second) {
            totalQuantity += order.quantity;
        }

        std::cout << std::left
                  << std::setw(12) << std::fixed << std::setprecision(2)
                  << priceLevel.first
                  << std::setw(12) << totalQuantity;

        for (const Order& order : priceLevel.second) {
            std::cout << "#" << order.id
                      << "(" << order.quantity << ") ";
        }

        std::cout << '\n';
    }

    std::cout << '\n';
}

void OrderBook::printTrades() const {
    std::cout << "========== TRADES ==============\n\n";

    std::cout << std::left
              << std::setw(10) << "ID"
              << std::setw(12) << "Buyer"
              << std::setw(12) << "Seller"
              << std::setw(12) << "Price"
              << std::setw(10) << "Qty"
              << "Time\n";

    std::cout << "------------------------------------------------------------\n";

    for (const Trade& trade : trades) {
        std::cout << std::left
                  << std::setw(10) << trade.tradeId
                  << std::setw(12) << trade.buyOrderId
                  << std::setw(12) << trade.sellOrderId
                  << std::setw(12) << std::fixed << std::setprecision(2)
                  << trade.price
                  << std::setw(10) << trade.quantity
                  << trade.timestamp
                  << '\n';
    }

    std::cout << '\n';
}
