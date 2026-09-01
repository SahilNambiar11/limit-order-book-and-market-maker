#ifndef ORDERBOOK_HPP
#define ORDERBOOK_HPP

#include "order.hpp"
#include "trade.hpp"
#include <map>
#include <deque>
#include <vector>

class OrderBook {
public:
    OrderBook();                        // constructor
    void addOrder(Order order);  // function declaration
    bool cancelOrder(int orderID);
    void printBook() const;
    void printTrades() const;

private:
    // bids
    std::map<double, std::deque<Order>, std::greater<double>> bids;

    //asks
    std::map<double, std::deque<Order>> asks;
    
    //trades
    std::vector<Trade> trades;
    
    int nextTradeID = 1;

    void tryMatch(Order& order);
};

#endif
