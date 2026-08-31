#include "order.hpp"
#include "trade.hpp"
#include <iostream>

int main() {
  Order order(1, Side::Buy, OrderType::Limit, 100.50, 50, 1);
  
  printOrder(order);

  Trade trade(1, 25, 17, 101.0, 40, 2);
  printTrade(trade);

  return 0;
}
