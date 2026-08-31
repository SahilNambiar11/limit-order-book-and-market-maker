#ifndef ORDER_HPP
#define ORDER_HPP

enum class Side {
  Buy,
  Sell
};

enum class OrderType {
  Limit,
  Market
};

struct Order {
  int id;
  Side side;
  OrderType type;
  double price;
  int quantity;
  long long timestamp;

  Order(int id, Side side, OrderType type, double price, int quantity, long long timestamp);
};

void printOrder(const Order& order);

#endif
