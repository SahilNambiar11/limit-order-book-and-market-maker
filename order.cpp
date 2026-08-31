#include <iostream>
#include "order.hpp"

Order::Order(int id, Side side, OrderType type, double price, int quantity, long long timestamp) : id(id), side(side), type(type), price(price), quantity(quantity), timestamp(timestamp) {}

void printOrder(const Order& order) {
  std::cout << "Order ID: " << order.id << std::endl;
  std::cout << "Side: "
            << (order.side == Side::Buy ? "BUY" : "SELL")
            << std::endl;
  std::cout << "Type: "
            << (order.type == OrderType::Limit ? "LIMIT" : "MARKET")
            << std::endl;
  std::cout << "Price: " << order.price << std::endl;
  std::cout << "Quantity: " << order.quantity << std::endl;
  std::cout << "Timestamp: " << order.timestamp << std::endl;
}
