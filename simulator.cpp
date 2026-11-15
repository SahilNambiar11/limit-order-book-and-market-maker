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
      externalOrderMaximumLifetimeSteps(
          config.externalOrderMaximumLifetimeSteps
      ),
      externalOrderMaximumDistance(config.externalOrderMaximumDistance),
      externalLiquidityReplenishmentEnabled(
          config.externalLiquidityReplenishmentEnabled
      ),
      externalOrdersCanceled(0),
      externalOrderLifetimeTotal(0),
      externalOrderLifetimeSamples(0),
      externalOrdersReplenished(0),
      rng(config.randomSeed) {
}

int Simulator::getNextOrderID() {
    return nextOrderID++;
}

double Simulator::getReferencePrice() const {
    return referencePrice;
}

int Simulator::getExternalOrdersCanceled() const {
    return externalOrdersCanceled;
}

double Simulator::getAverageExternalOrderLifetime() const {
    return externalOrderLifetimeSamples == 0
        ? 0.0
        : static_cast<double>(externalOrderLifetimeTotal) /
            externalOrderLifetimeSamples;
}

int Simulator::getExternalOrdersReplenished() const {
    return externalOrdersReplenished;
}

void Simulator::seedInitialBook() {
    replenishExternalLiquidity(false);
}

void Simulator::addExternalLimitOrder(
    Side side,
    double price,
    int quantity,
    bool countAsReplenishment
) {
    const int orderID = getNextOrderID();
    book.addOrder(Order(
        orderID,
        side,
        OrderType::Limit,
        price,
        quantity,
        currentTime
    ));

    if (book.getOrderRemainingQuantity(orderID) > 0) {
        externalOrders.push_back(ExternalOrderState{
            orderID,
            side,
            price,
            currentTime
        });
    }
    else {
        ++externalOrderLifetimeSamples;
    }

    if (countAsReplenishment) {
        ++externalOrdersReplenished;
    }
}

void Simulator::replenishExternalLiquidity(bool countAsReplenishment) {
    if (!externalLiquidityReplenishmentEnabled && countAsReplenishment) {
        return;
    }

    int activeBids = 0;
    int activeAsks = 0;
    for (const ExternalOrderState& order : externalOrders) {
        if (book.getOrderRemainingQuantity(order.orderID) == 0) {
            continue;
        }
        if (order.side == Side::Buy) {
            ++activeBids;
        }
        else {
            ++activeAsks;
        }
    }

    for (int level = activeBids + 1;
         level <= initialBookDepth;
         ++level) {
        const double price = std::max(
            0.01,
            std::round((referencePrice - level * 0.01) * 100.0) / 100.0
        );
        addExternalLimitOrder(
            Side::Buy,
            price,
            initialLevelQuantity,
            countAsReplenishment
        );
    }

    for (int level = activeAsks + 1;
         level <= initialBookDepth;
         ++level) {
        const double price = std::round(
            (referencePrice + level * 0.01) * 100.0
        ) / 100.0;
        addExternalLimitOrder(
            Side::Sell,
            price,
            initialLevelQuantity,
            countAsReplenishment
        );
    }
}

void Simulator::maintainExternalLiquidity() {
    auto order = externalOrders.begin();
    while (order != externalOrders.end()) {
        const int remainingQuantity =
            book.getOrderRemainingQuantity(order->orderID);
        const long long lifetime = currentTime - order->submissionTime;

        if (remainingQuantity == 0) {
            externalOrderLifetimeTotal += lifetime;
            ++externalOrderLifetimeSamples;
            order = externalOrders.erase(order);
            continue;
        }

        const bool expired = externalOrderMaximumLifetimeSteps > 0 &&
            lifetime >= externalOrderMaximumLifetimeSteps;
        const bool tooFar = externalOrderMaximumDistance > 0.0 &&
            std::abs(order->price - referencePrice) >
                externalOrderMaximumDistance;
        const bool wrongSide = order->side == Side::Buy
            ? order->price >= referencePrice
            : order->price <= referencePrice;

        if (expired || tooFar || wrongSide) {
            if (book.cancelOrder(order->orderID)) {
                ++externalOrdersCanceled;
            }
            externalOrderLifetimeTotal += lifetime;
            ++externalOrderLifetimeSamples;
            order = externalOrders.erase(order);
            continue;
        }

        ++order;
    }

    replenishExternalLiquidity(true);
}

void Simulator::recordRemainingExternalOrderLifetimes() {
    for (const ExternalOrderState& order : externalOrders) {
        externalOrderLifetimeTotal += currentTime - order.submissionTime;
        ++externalOrderLifetimeSamples;
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
        maintainExternalLiquidity();
        marketMaker.beginStep(currentTime);

        const Order externalOrder = generateOrder(priceChange);
        if (externalOrder.type == OrderType::Limit) {
            addExternalLimitOrder(
                externalOrder.side,
                externalOrder.price,
                externalOrder.quantity,
                false
            );
        }
        else {
            book.addOrder(externalOrder);
        }

        const int bidID = getNextOrderID();
        const int askID = getNextOrderID();
        marketMaker.update(currentTime, bidID, askID);
    }

    marketMaker.finalize(referencePrice);
    recordRemainingExternalOrderLifetimes();
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
