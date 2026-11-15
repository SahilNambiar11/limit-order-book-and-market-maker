# C++ Limit Order Book and Market Maker

LIVE DEMO: https://limit-order-book-and-market-maker-hjanxlvpk4i84zbesfpwrc.streamlit.app/

This project implements a price-time-priority limit order book, order-flow
simulator, and inventory-aware market maker in C++17. A pybind11 extension
exposes the native multi-session experiment to a Streamlit dashboard; no
matching or market-making logic is duplicated in Python.

## Features

- Market and limit orders with partial fills
- FIFO time priority within each price level
- Order cancellation
- Inventory-aware two-sided market-maker quotes
- Independent-reference mark-to-market and realized/unrealized P&L
- Hard inventory limits that include outstanding quote exposure
- Configurable informed flow, volatility, trend, fees, latency, and liquidation
- Reproducible inventory-aware versus fixed-spread comparisons
- Aggregate and per-session risk metrics exposed to Python and Streamlit

## Prerequisites

- A C++17 compiler (`g++` or `clang++`)
- GNU Make
- Python 3 with development headers

Using a virtual environment keeps the Python build dependencies isolated:

```bash
python3 -m venv .venv
source .venv/bin/activate
python -m pip install --upgrade pip
python -m pip install -r requirements.txt
```

## Build and run the C++ program

```bash
make
make run
```

The native executable uses the defaults in `SimulationConfig`. Applications
can construct that object and pass it to `runSimulation` to configure session
count, steps, seed, quote size, inventory limit, market-order probability,
maximum generated order size, reference-price process, informed flow, fees,
quote latency, and terminal liquidation.

## Build the Python module

Build the extension with the same interpreter that will run Streamlit:

```bash
make python-module PYTHON=python
python -c "import orderbook_cpp; print(orderbook_cpp.__doc__)"
```

If the virtual environment is not activated, use its interpreter explicitly:

```bash
make python-module PYTHON=.venv/bin/python
```

The build creates an ABI-tagged module such as
`orderbook_cpp.cpython-313-darwin.so` in the project directory.

## Run the Streamlit dashboard

From the project directory and the same Python environment used to build the
extension:

```bash
python -m streamlit run streamlit_app.py
```

Alternatively, build and launch in one command:

```bash
make streamlit PYTHON=python
```

Use the sidebar to configure the simulation and click **Run Simulation**. The
dashboard calls the compiled C++ `run_simulation` function and displays the
aggregate metrics and per-session charts.

## Tests

```bash
make test
```

The native test executable covers price-time priority, partial fills,
cancellation, final-event settlement, ledger and cash reconciliation, hard
inventory limits, latency, independent valuation, fees, terminal liquidation,
configuration validation, and deterministic stressed runs.

For a quick Python smoke test after building the module:

```bash
python - <<'PY'
import orderbook_cpp

config = orderbook_cpp.SimulationConfig()
config.number_of_sessions = 2
config.steps_per_session = 100
result = orderbook_cpp.run_simulation(config)
print(result.average_pnl, len(result.session_results))
PY
```

## Configuration and results

The C++ API groups inputs into `SimulationConfig`, `MarketMakerConfig`, and
`SimulatorConfig`. `runSimulation` returns a `SimulationResult` containing:

- mean, median, standard deviation, best, and worst P&L
- profitable-session count and percentage
- average fill rate
- average and worst maximum inventory exposure
- average maximum drawdown
- trading fees, liquidation costs, and ledger-reconciliation status
- total bought and sold volume
- one `SessionResult` per session for charts and further analysis

`compareStrategies` runs the inventory-aware strategy and a fixed-spread
baseline with the same session seeds and order-flow configuration.

## Simulation assumptions

The reference price follows a configurable arithmetic process with a per-step
trend and normally distributed innovation. Uninformed orders choose sides
randomly. Informed orders trade in the direction of the latest exogenous price
move, so they can adversely select quotes that were active before that move.
Quote latency delays activation without giving the market maker future price
information. Remaining inventory is either valued at the independent terminal
reference price or liquidated there with configured per-unit fees and costs.

The model remains a research simulation: it does not model exchange-specific
queue mechanics, variable depth, network jitter, impact, or calibration to
historical data. Dashboard results are simulated outcomes, not claims about
live-trading performance.

Run `make clean` to remove native executables and compiled Python modules.
