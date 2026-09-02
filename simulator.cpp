#include "simulator.hpp"
#include "marketmaker.hpp"

#include <cmath>

Simulator::Simulator(
    OrderBook& book,
    MarketMaker& marketMaker,
    unsigned int seed
)
    : book(book),
      marketMaker(marketMaker),
      currentTime(0),
      nextOrderID(1),
      referencePrice(100.0),
      rng(seed) {
}

int Simulator::getNextOrderID() {
    return nextOrderID++;
}

void Simulator::run(int steps) {
    for (int i = 0; i < steps; i++) {
        currentTime++;

        std::uniform_int_distribution<int> moveDist(-1, 1);
        int move = moveDist(rng);

        referencePrice += move * 0.01;
        referencePrice =
            std::round(referencePrice * 100.0) / 100.0;

        Order order = generateOrder();
        book.addOrder(order);

        int bidID = getNextOrderID();
        int askID = getNextOrderID();

        marketMaker.update(currentTime, bidID, askID);
    }
}

Order Simulator::generateOrder() {
    std::uniform_int_distribution<int> sideDist(0, 1);

    Side side =
        sideDist(rng) == 0 ? Side::Buy : Side::Sell;

    std::uniform_int_distribution<int> typeDist(1, 100);

    OrderType type =
        typeDist(rng) <= 80
            ? OrderType::Limit
            : OrderType::Market;

    std::uniform_int_distribution<int> quantityDist(1, 50);
    int quantity = quantityDist(rng);

    std::uniform_int_distribution<int> offsetDist(1, 5);
    double offset = offsetDist(rng) * 0.01;

    double price = 0.0;

    if (type == OrderType::Limit) {
        if (side == Side::Buy) {
            price = referencePrice - offset;
        }
        else {
            price = referencePrice + offset;
        }

        price =
            std::round(price * 100.0) / 100.0;
    }

    return Order(
        getNextOrderID(),
        side,
        type,
        price,
        quantity,
        currentTime
    );
}
