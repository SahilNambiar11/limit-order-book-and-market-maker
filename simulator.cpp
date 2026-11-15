#include "simulator.hpp"
#include "marketmaker.hpp"

#include <algorithm>
#include <cmath>

Simulator::Simulator(
    OrderBook& book,
    MarketMaker& marketMaker,
    const SimulatorConfig& config
)
    : book(book),
      marketMaker(marketMaker),
      currentTime(0),
      nextOrderID(1),
      referencePrice(100.0),
      marketOrderProbability(config.marketOrderProbability),
      maximumGeneratedOrderQuantity(config.maximumGeneratedOrderQuantity),
      referencePriceVolatility(config.referencePriceVolatility),
      referencePriceTrend(config.referencePriceTrend),
      informedOrderProbability(config.informedOrderProbability),
      initialBookDepth(config.initialBookDepth),
      initialLevelQuantity(config.initialLevelQuantity),
      rng(config.randomSeed) {
}

int Simulator::getNextOrderID() {
    return nextOrderID++;
}

double Simulator::getReferencePrice() const {
    return referencePrice;
}

void Simulator::seedInitialBook() {
    for (int level = 1; level <= initialBookDepth; ++level) {
        const double offset = level * 0.01;
        book.addOrder(Order(
            getNextOrderID(),
            Side::Buy,
            OrderType::Limit,
            referencePrice - offset,
            initialLevelQuantity,
            currentTime
        ));
        book.addOrder(Order(
            getNextOrderID(),
            Side::Sell,
            OrderType::Limit,
            referencePrice + offset,
            initialLevelQuantity,
            currentTime
        ));
    }
}

void Simulator::run(int steps) {
    seedInitialBook();
    marketMaker.setReferencePrice(referencePrice);

    for (int i = 0; i < steps; ++i) {
        ++currentTime;

        std::normal_distribution<double> priceMoveDistribution(
            referencePriceTrend,
            referencePriceVolatility
        );
        const double previousReferencePrice = referencePrice;
        referencePrice += priceMoveDistribution(rng);
        referencePrice = std::max(
            0.01,
            std::round(referencePrice * 100.0) / 100.0
        );
        const double priceChange =
            referencePrice - previousReferencePrice;

        marketMaker.setReferencePrice(referencePrice);
        marketMaker.beginStep(currentTime);

        book.addOrder(generateOrder(priceChange));

        const int bidID = getNextOrderID();
        const int askID = getNextOrderID();
        marketMaker.update(currentTime, bidID, askID);
    }

    marketMaker.finalize(referencePrice);
}

Order Simulator::generateOrder(double priceChange) {
    std::uniform_int_distribution<int> sideDistribution(0, 1);
    std::bernoulli_distribution informedDistribution(
        informedOrderProbability
    );
    const bool informed = informedDistribution(rng);

    Side side;
    OrderType type;

    if (informed) {
        if (priceChange > 0.0) {
            side = Side::Buy;
        }
        else if (priceChange < 0.0) {
            side = Side::Sell;
        }
        else {
            side = sideDistribution(rng) == 0
                ? Side::Buy
                : Side::Sell;
        }
        type = OrderType::Market;
    }
    else {
        side = sideDistribution(rng) == 0 ? Side::Buy : Side::Sell;
        std::bernoulli_distribution marketOrderDistribution(
            marketOrderProbability
        );
        type = marketOrderDistribution(rng)
            ? OrderType::Market
            : OrderType::Limit;
    }

    std::uniform_int_distribution<int> quantityDistribution(
        1,
        maximumGeneratedOrderQuantity
    );
    const int quantity = quantityDistribution(rng);

    std::uniform_int_distribution<int> offsetDistribution(1, 5);
    const double offset = offsetDistribution(rng) * 0.01;

    double price = 0.0;
    if (type == OrderType::Limit) {
        price = side == Side::Buy
            ? referencePrice - offset
            : referencePrice + offset;
        price = std::max(
            0.01,
            std::round(price * 100.0) / 100.0
        );
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
