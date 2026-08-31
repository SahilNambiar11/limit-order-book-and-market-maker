#include "order.hpp"
#include <iostream>

int main() {
  Order order(1, Side::Buy, OrderType::Limit, 100.50, 50, 1);
  
  printOrder(order);

  return 0;
}
