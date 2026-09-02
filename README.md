# Limit Order Book & Market Maker

A C++ limit order book and market simulation implementing price-time-priority matching, market and limit orders, partial fills, cancellations, and an inventory-aware market-making strategy.

The project simulates order flow, executes trades through a central limit order book, and evaluates the market maker using mark-to-market P&L, fill rate, and inventory exposure across repeated trading sessions.

## Features

### Limit Order Book
- Price-time-priority matching
- Limit and market orders
- Partial fills
- Order cancellation
- FIFO ordering within each price level
- Best bid and ask tracking
- Trade execution and history
- Multi-level order matching

### Market Maker
- Continuously quotes both bid and ask orders
- Cancels and replaces quotes as the market changes
- Improves quotes inside wider spreads
- Adjusts quotes based on inventory exposure
- Stops increasing exposure beyond inventory thresholds
- Tracks cash, inventory, filled volume, and quoted volume
- Calculates mark-to-market P&L

### Market Simulator
- Generates randomized buy and sell order flow
- Supports both market and limit orders
- Simulates changing reference prices
- Uses deterministic random seeds for reproducible experiments
- Supports repeated independent trading sessions

## Architecture

```text
                    Market Simulator
                          |
                  Random Order Flow
                          |
                          v
                 +-----------------+
                 |    OrderBook    |
                 |                 |
                 | Matching Engine |
                 +-----------------+
                    ^           |
                    |           | Trades
                    |           v
                 +-----------------+
                 |   MarketMaker   |
                 |                 |
                 | Quotes Bid/Ask  |
                 | Tracks Inventory|
                 | Tracks P&L      |
                 +-----------------+
```

The `OrderBook` acts as the exchange and maintains resting bids and asks. The `Simulator` generates external market activity, while the `MarketMaker` reacts to the current book and submits its own quotes.

## Order Book Design

Bids and asks are stored as ordered maps of price levels:

```cpp
std::map<double, std::deque<Order>, std::greater<double>> bids;
std::map<double, std::deque<Order>> asks;
```

This provides:

- Highest bid first
- Lowest ask first
- FIFO execution within each price level

Incoming orders repeatedly match against the best available opposing price until the order is completely filled, no liquidity remains, or its limit price can no longer cross the spread.

## Market-Making Strategy

The market maker observes the current best bid and ask and maintains quotes on both sides of the book.

When the spread is sufficiently wide, it improves the existing prices by one tick:

```text
Best Bid:  $100.00
Best Ask:  $100.05

MM Bid:    $100.01
MM Ask:    $100.04
```

The strategy also manages inventory risk.

When inventory becomes too positive, quotes are shifted downward to discourage additional buying and encourage selling. When inventory becomes too negative, quotes are shifted upward to encourage buying and discourage additional selling.

Position thresholds prevent the strategy from continually increasing exposure in one direction.

## P&L

Market-maker performance is measured using mark-to-market P&L:

```text
P&L = Cash + Inventory × Midprice
```

where:

```text
Midprice = (Best Bid + Best Ask) / 2
```

This accounts for both realized cash flows and the current value of remaining inventory.

## Simulation Results

The strategy was evaluated across 100 independent simulations with 10,000 market steps per session.

```text
Sessions:                       100
Steps per session:              10,000
Total simulated steps:          1,000,000

Average P&L:                    $566.55
Best P&L:                       $2,120.56
Worst P&L:                      $87.61
Profitable sessions:            100 / 100

Average fill rate:              12.95%
Average max inventory exposure: 100.49
Worst inventory exposure:       109
```

These results are specific to the simplified simulated order-flow model and are intended to evaluate the behavior of the implementation rather than represent real-world trading performance.

## Build

Requires a C++17-compatible compiler.

```bash
make
```

Run the simulation:

```bash
make run
```

Clean generated binaries:

```bash
make clean
```

## Project Structure

```text
.
├── main.cpp
├── order.hpp
├── order.cpp
├── trade.hpp
├── trade.cpp
├── orderbook.hpp
├── orderbook.cpp
├── simulator.hpp
├── simulator.cpp
├── marketmaker.hpp
├── marketmaker.cpp
├── Makefile
└── README.md
```

## Technologies

- C++17
- STL containers (`std::map`, `std::deque`, `std::vector`)
- Make
