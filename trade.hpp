#ifndef TRADE_HPP
#define TRADE_HPP

struct Trade {
  int tradeId;
  int buyOrderId;
  int sellOrderId;
  double price;
  int quantity;
  long long timestamp;

  Trade(int tradeId,
        int buyOrderId,
        int sellOrderId,
        double price,
        int quantity,
        long long timestamp);
};

void printTrade(const Trade& trade);

#endif
