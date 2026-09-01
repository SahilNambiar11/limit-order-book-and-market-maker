#include "order.hpp"
#include "trade.hpp"
#include "orderbook.hpp"
#include <iostream>


int main() {
    OrderBook book;

    book.addOrder(Order(1, Side::Buy, OrderType::Limit, 101.0, 20, 1));
    book.addOrder(Order(2, Side::Buy, OrderType::Limit, 101.0, 30, 2));
    book.addOrder(Order(3, Side::Buy, OrderType::Limit, 100.0, 40, 3));

    book.addOrder(Order(4, Side::Sell, OrderType::Limit, 103.0, 25, 4));
    book.addOrder(Order(5, Side::Sell, OrderType::Limit, 104.0, 35, 5));

    std::cout << "BEFORE CANCELLATION\n";
    book.printBook();

    std::cout << "Cancel order 2\n";
    std::cout << book.cancelOrder(2) << "\n";

    std::cout << "AFTER CANCELLING ORDER 2\n";
    book.printBook();

    std::cout << "Cancel order 3\n";
    std::cout << book.cancelOrder(3) << "\n";

    std::cout << "AFTER CANCELLING ORDER 3\n";
    book.printBook();

    std::cout << "Cancel nonexistent order 999\n";
    std::cout << book.cancelOrder(999) << "\n";
    
    std::cout << "Best bid: " << book.bestBid() << "\n";
    std::cout << "Best ask: " << book.bestAsk() << "\n";

    return 0;

}

