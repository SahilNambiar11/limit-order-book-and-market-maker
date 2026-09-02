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
- Mark-to-market P&L and fill-rate tracking
- Configurable, reproducible multi-session experiments
- Aggregate and per-session metrics exposed to Python
- Streamlit metrics, P&L distribution, inventory chart, and result table

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
count, steps, seed, quote size, inventory limit, market-order probability, and
maximum generated order size.

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
cancellation, market/limit behavior, configuration validation, aggregate
volume accounting, and deterministic seeded runs.

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

- average, best, and worst P&L
- profitable-session count
- average fill rate
- average and worst maximum inventory exposure
- total bought and sold volume
- one `SessionResult` per session for charts and further analysis

Run `make clean` to remove native executables and compiled Python modules.
