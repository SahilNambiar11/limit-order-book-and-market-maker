#include "trade.hpp"
#include <iostream>

Trade::Trade(int tradeId,
             int buyOrderId,
             int sellOrderId,
             double price,
             int quantity,
             long long timestamp)
    : tradeId(tradeId),
      buyOrderId(buyOrderId),
      sellOrderId(sellOrderId),
      price(price),
      quantity(quantity),
      timestamp(timestamp) {
}

void printTrade(const Trade& trade) {
  std::cout << "Trade ID: " << trade.tradeId << std::endl;
  std::cout << "Buy Order ID: " << trade.buyOrderId << std::endl;
  std::cout << "Sell Order ID: " << trade.sellOrderId << std::endl;
  std::cout << "Price: " << trade.price << std::endl;
  std::cout << "Quantity: " << trade.quantity << std::endl;
  std::cout << "Timestamp: " << trade.timestamp << std::endl;
}

