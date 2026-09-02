import subprocess
import sys
from pathlib import Path

import altair as alt
import pandas as pd
import streamlit as st

st.set_page_config(
    page_title="Order Book Simulator",
    page_icon="📈",
    layout="wide",
)

try:
    import orderbook_cpp
except ImportError:
    repository_directory = Path(__file__).resolve().parent
    try:
        subprocess.run(
            ["make", "python-module", f"PYTHON={sys.executable}"],
            cwd=repository_directory,
            check=True,
            capture_output=True,
            text=True,
        )
    except (OSError, subprocess.CalledProcessError) as error:
        st.error("Failed to build the C++ Python extension.")
        build_output = "\n".join(
            output.strip()
            for output in (
                getattr(error, "stdout", ""),
                getattr(error, "stderr", ""),
            )
            if output and output.strip()
        )
        if build_output:
            st.code(build_output)
        st.exception(error)
        st.stop()

    import orderbook_cpp

st.title("Limit Order Book Market Maker")
st.caption(
    "Configure and run the native C++ matching engine, order-flow simulator, "
    "and inventory-aware market maker."
)

with st.sidebar:
    st.header("Simulation parameters")
    with st.form("simulation_parameters"):
        number_of_sessions = st.number_input(
            "Number of sessions",
            min_value=1,
            max_value=10_000,
            value=100,
            step=1,
        )
        steps_per_session = st.number_input(
            "Steps per session",
            min_value=1,
            max_value=1_000_000,
            value=10_000,
            step=1_000,
        )
        random_seed = st.number_input(
            "Random seed",
            min_value=0,
            max_value=4_294_967_295,
            value=1,
            step=1,
        )
        quote_size = st.number_input(
            "Market maker quote size",
            min_value=1,
            max_value=100_000,
            value=10,
            step=1,
        )
        maximum_inventory = st.number_input(
            "Maximum inventory",
            min_value=1,
            max_value=1_000_000,
            value=100,
            step=10,
        )
        market_order_probability = st.slider(
            "Market-order probability",
            min_value=0.0,
            max_value=1.0,
            value=0.20,
            step=0.01,
            format="%.2f",
        )
        maximum_order_quantity = st.number_input(
            "Maximum generated order quantity",
            min_value=1,
            max_value=100_000,
            value=50,
            step=1,
        )

        submitted = st.form_submit_button(
            "Run Simulation",
            type="primary",
            width="stretch",
        )

if submitted:
    config = orderbook_cpp.SimulationConfig()
    config.number_of_sessions = int(number_of_sessions)
    config.steps_per_session = int(steps_per_session)

    market_maker_config = orderbook_cpp.MarketMakerConfig()
    market_maker_config.quote_size = int(quote_size)
    market_maker_config.maximum_inventory = int(maximum_inventory)
    config.market_maker = market_maker_config

    simulator_config = orderbook_cpp.SimulatorConfig()
    simulator_config.random_seed = int(random_seed)
    simulator_config.market_order_probability = float(
        market_order_probability
    )
    simulator_config.maximum_generated_order_quantity = int(
        maximum_order_quantity
    )
    config.simulator = simulator_config

    try:
        with st.spinner("Running native C++ simulation..."):
            st.session_state["simulation_result"] = (
                orderbook_cpp.run_simulation(config)
            )
    except (RuntimeError, ValueError) as error:
        st.error(f"Simulation failed: {error}")

result = st.session_state.get("simulation_result")

if result is None:
    st.info("Choose parameters in the sidebar, then click **Run Simulation**.")
    st.stop()

st.subheader("Aggregate metrics")

metrics_row_one = st.columns(4)
metrics_row_one[0].metric("Average P&L", f"${result.average_pnl:,.2f}")
metrics_row_one[1].metric("Best P&L", f"${result.best_pnl:,.2f}")
metrics_row_one[2].metric("Worst P&L", f"${result.worst_pnl:,.2f}")
metrics_row_one[3].metric(
    "Profitable sessions",
    f"{result.profitable_sessions:,} / {result.number_of_sessions:,}",
)

metrics_row_two = st.columns(3)
metrics_row_two[0].metric(
    "Average fill rate",
    f"{result.average_fill_rate:.2%}",
)
metrics_row_two[1].metric(
    "Average max inventory",
    f"{result.average_maximum_inventory_exposure:,.2f}",
)
metrics_row_two[2].metric(
    "Worst inventory exposure",
    f"{result.worst_inventory_exposure:,}",
)

metrics_row_three = st.columns(3)
metrics_row_three[0].metric(
    "Total bought volume",
    f"{result.total_bought_volume:,}",
)
metrics_row_three[1].metric(
    "Total sold volume",
    f"{result.total_sold_volume:,}",
)
metrics_row_three[2].metric(
    "Simulated steps",
    f"{result.total_simulated_steps:,}",
)

session_data = pd.DataFrame(
    [
        {
            "Session": session.session_number,
            "Seed": session.random_seed,
            "P&L": session.pnl,
            "Fill rate": session.fill_rate,
            "Maximum inventory exposure": session.maximum_inventory_exposure,
            "Ending inventory": session.ending_inventory,
            "Bought volume": session.bought_volume,
            "Sold volume": session.sold_volume,
        }
        for session in result.session_results
    ]
)

st.subheader("Session analysis")
chart_column_one, chart_column_two = st.columns(2)

with chart_column_one:
    pnl_histogram = (
        alt.Chart(session_data)
        .mark_bar(color="#2E86AB")
        .encode(
            x=alt.X("P&L:Q", bin=alt.Bin(maxbins=20), title="P&L"),
            y=alt.Y("count():Q", title="Sessions"),
            tooltip=[alt.Tooltip("count():Q", title="Sessions")],
        )
        .properties(title="P&L distribution", height=300)
    )
    st.altair_chart(pnl_histogram, width="stretch")

with chart_column_two:
    inventory_chart = (
        alt.Chart(session_data)
        .mark_line(point=True, color="#F18F01")
        .encode(
            x=alt.X("Session:Q", title="Session"),
            y=alt.Y(
                "Maximum inventory exposure:Q",
                title="Maximum absolute inventory",
            ),
            tooltip=[
                "Session:Q",
                "Maximum inventory exposure:Q",
                alt.Tooltip("P&L:Q", format=",.2f"),
            ],
        )
        .properties(title="Inventory exposure by session", height=300)
    )
    st.altair_chart(inventory_chart, width="stretch")

with st.expander("Per-session results"):
    st.dataframe(
        session_data.style.format(
            {
                "P&L": "${:,.2f}",
                "Fill rate": "{:.2%}",
            }
        ),
        width="stretch",
        hide_index=True,
    )
