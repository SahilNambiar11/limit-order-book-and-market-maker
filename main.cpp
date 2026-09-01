#include "order.hpp"
#include "trade.hpp"
#include "orderbook.hpp"
#include <iostream>


int main() {
    OrderBook book;

    book.addOrder(Order(1, Side::Sell, OrderType::Limit, 101.0, 20, 1));
    book.addOrder(Order(2, Side::Sell, OrderType::Limit, 102.0, 30, 2));
    book.addOrder(Order(3, Side::Sell, OrderType::Limit, 103.0, 40, 3));

    book.addOrder(Order(4, Side::Buy, OrderType::Market, 0.0, 60, 4));

    book.printBook();
    book.printTrades();

    return 0;
}

