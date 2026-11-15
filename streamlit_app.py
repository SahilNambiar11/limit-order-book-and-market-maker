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
    "and inventory-aware market maker. Results are simulated and are not "
    "evidence of actual trading performance."
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
        reference_price_volatility = st.number_input(
            "Reference-price volatility per step ($)",
            min_value=0.0,
            max_value=10.0,
            value=0.02,
            step=0.01,
            format="%.3f",
        )
        reference_price_trend = st.number_input(
            "Reference-price trend per step ($)",
            min_value=-1.0,
            max_value=1.0,
            value=0.0,
            step=0.001,
            format="%.3f",
        )
        informed_order_probability = st.slider(
            "Informed-order probability",
            min_value=0.0,
            max_value=1.0,
            value=0.15,
            step=0.01,
            format="%.2f",
        )
        transaction_fee_per_unit = st.number_input(
            "Transaction fee per unit ($)",
            min_value=0.0,
            max_value=10.0,
            value=0.001,
            step=0.001,
            format="%.4f",
        )
        quote_latency_steps = st.number_input(
            "Quote latency (steps)",
            min_value=0,
            max_value=1_000,
            value=0,
            step=1,
        )
        terminal_liquidation_enabled = st.checkbox(
            "Liquidate terminal inventory",
            value=True,
        )
        terminal_liquidation_cost_per_unit = st.number_input(
            "Terminal liquidation cost per unit ($)",
            min_value=0.0,
            max_value=10.0,
            value=0.01,
            step=0.01,
            format="%.3f",
        )
        compare_fixed_spread = st.checkbox(
            "Compare fixed-spread baseline",
            value=False,
        )
        fixed_spread = st.number_input(
            "Baseline fixed spread ($)",
            min_value=0.01,
            max_value=10.0,
            value=0.04,
            step=0.01,
            format="%.2f",
        )
        maximum_quote_distance = st.number_input(
            "Maximum maker quote distance from reference ($)",
            min_value=0.01,
            max_value=10.0,
            value=0.10,
            step=0.01,
            format="%.2f",
        )
        external_order_maximum_lifetime = st.number_input(
            "External order maximum lifetime (steps)",
            min_value=0,
            max_value=1_000_000,
            value=250,
            step=10,
            help="Set to zero to disable lifetime-based expiration.",
        )
        external_order_maximum_distance = st.number_input(
            "External order maximum reference distance ($)",
            min_value=0.0,
            max_value=100.0,
            value=0.25,
            step=0.01,
            format="%.2f",
            help="Set to zero to disable distance-based cancellation.",
        )
        replenish_external_liquidity = st.checkbox(
            "Replenish external liquidity",
            value=True,
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
    market_maker_config.transaction_fee_per_unit = float(
        transaction_fee_per_unit
    )
    market_maker_config.quote_latency_steps = int(quote_latency_steps)
    market_maker_config.terminal_liquidation_enabled = bool(
        terminal_liquidation_enabled
    )
    market_maker_config.terminal_liquidation_cost_per_unit = float(
        terminal_liquidation_cost_per_unit
    )
    market_maker_config.fixed_spread = float(fixed_spread)
    market_maker_config.maximum_quote_distance_from_reference = float(
        maximum_quote_distance
    )
    config.market_maker = market_maker_config

    simulator_config = orderbook_cpp.SimulatorConfig()
    simulator_config.random_seed = int(random_seed)
    simulator_config.market_order_probability = float(
        market_order_probability
    )
    simulator_config.maximum_generated_order_quantity = int(
        maximum_order_quantity
    )
    simulator_config.reference_price_volatility = float(
        reference_price_volatility
    )
    simulator_config.reference_price_trend = float(reference_price_trend)
    simulator_config.informed_order_probability = float(
        informed_order_probability
    )
    simulator_config.external_order_maximum_lifetime_steps = int(
        external_order_maximum_lifetime
    )
    simulator_config.external_order_maximum_distance = float(
        external_order_maximum_distance
    )
    simulator_config.external_liquidity_replenishment_enabled = bool(
        replenish_external_liquidity
    )
    config.simulator = simulator_config

    try:
        with st.spinner("Running native C++ simulation..."):
            if compare_fixed_spread:
                comparison = orderbook_cpp.compare_strategies(config)
                st.session_state["simulation_result"] = (
                    comparison.inventory_aware
                )
                st.session_state["strategy_comparison"] = comparison
            else:
                st.session_state["simulation_result"] = (
                    orderbook_cpp.run_simulation(config)
                )
                st.session_state.pop("strategy_comparison", None)
    except (RuntimeError, ValueError) as error:
        st.error(f"Simulation failed: {error}")

result = st.session_state.get("simulation_result")

if result is None:
    st.info("Choose parameters in the sidebar, then click **Run Simulation**.")
    st.stop()

st.subheader("Aggregate metrics")

metrics_row_one = st.columns(4)
metrics_row_one[0].metric("Average P&L", f"${result.average_pnl:,.2f}")
metrics_row_one[1].metric("Median P&L", f"${result.median_pnl:,.2f}")
metrics_row_one[2].metric(
    "P&L standard deviation",
    f"${result.pnl_standard_deviation:,.2f}",
)
metrics_row_one[3].metric(
    "Profitable sessions",
    f"{result.profitable_session_percentage:.1%}",
)

metrics_row_two = st.columns(4)
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
metrics_row_two[3].metric(
    "Average maximum drawdown",
    f"${result.average_maximum_drawdown:,.2f}",
)

metrics_row_three = st.columns(4)
metrics_row_three[0].metric(
    "Trading fees",
    f"${result.total_trading_fees:,.2f}",
)
metrics_row_three[1].metric(
    "Terminal liquidation costs",
    f"${result.total_terminal_liquidation_costs:,.2f}",
)
metrics_row_three[2].metric(
    "Simulated steps",
    f"{result.total_simulated_steps:,}",
)
metrics_row_three[3].metric(
    "Ledger reconciliation",
    "Passed" if result.all_sessions_reconciled else "Failed",
)

st.caption(
    "P&L includes transaction fees, independent-reference mark-to-market, "
    "and configured terminal liquidation costs."
)

st.subheader("Liquidity and quote diagnostics")
diagnostic_row_one = st.columns(4)
diagnostic_row_one[0].metric(
    "Average quoted spread",
    f"${result.average_quoted_spread:,.4f}",
)
diagnostic_row_one[1].metric(
    "Maximum quoted spread",
    f"${result.maximum_quoted_spread:,.4f}",
)
diagnostic_row_one[2].metric(
    "Average maker quote distance",
    f"${result.average_maker_quote_distance:,.4f}",
)
diagnostic_row_one[3].metric(
    "Average midpoint deviation",
    f"${result.average_book_midpoint_deviation:,.4f}",
)

diagnostic_row_two = st.columns(3)
diagnostic_row_two[0].metric(
    "External orders canceled",
    f"{result.total_external_orders_canceled:,}",
)
diagnostic_row_two[1].metric(
    "Average external order lifetime",
    f"{result.average_external_order_lifetime:,.2f} steps",
)
diagnostic_row_two[2].metric(
    "External orders replenished",
    f"{result.total_external_orders_replenished:,}",
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
            "Realized P&L": session.realized_pnl,
            "Unrealized P&L": session.unrealized_pnl,
            "Trading fees": session.trading_fees,
            "Liquidation costs": session.terminal_liquidation_costs,
            "Maximum drawdown": session.maximum_drawdown,
            "Terminal reference price": session.terminal_reference_price,
            "Ledger reconciled": session.ledger_reconciled,
            "Average quoted spread": session.average_quoted_spread,
            "Maximum quoted spread": session.maximum_quoted_spread,
            "Average maker quote distance": (
                session.average_maker_quote_distance
            ),
            "Average midpoint deviation": (
                session.average_book_midpoint_deviation
            ),
            "External orders canceled": session.external_orders_canceled,
            "Average external order lifetime": (
                session.average_external_order_lifetime
            ),
            "External orders replenished": (
                session.external_orders_replenished
            ),
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
    risk_chart = (
        alt.Chart(session_data)
        .mark_circle(color="#F18F01", size=70)
        .encode(
            x=alt.X(
                "Maximum inventory exposure:Q",
                title="Maximum absolute inventory",
            ),
            y=alt.Y(
                "Maximum drawdown:Q",
                title="Maximum drawdown",
            ),
            tooltip=[
                "Session:Q",
                "Maximum inventory exposure:Q",
                alt.Tooltip("Maximum drawdown:Q", format=",.2f"),
                alt.Tooltip("P&L:Q", format=",.2f"),
            ],
        )
        .properties(title="Inventory and drawdown risk", height=300)
    )
    st.altair_chart(risk_chart, width="stretch")

comparison = st.session_state.get("strategy_comparison")
if comparison is not None:
    st.subheader("Strategy comparison")
    comparison_data = pd.DataFrame(
        [
            {
                "Strategy": "Inventory-aware",
                "Mean P&L": comparison.inventory_aware.average_pnl,
                "Median P&L": comparison.inventory_aware.median_pnl,
                "P&L standard deviation": (
                    comparison.inventory_aware.pnl_standard_deviation
                ),
                "Profitable sessions": (
                    comparison.inventory_aware.profitable_session_percentage
                ),
                "Average max drawdown": (
                    comparison.inventory_aware.average_maximum_drawdown
                ),
                "Fill rate": comparison.inventory_aware.average_fill_rate,
                "Average max inventory": (
                    comparison.inventory_aware
                    .average_maximum_inventory_exposure
                ),
                "Average quoted spread": (
                    comparison.inventory_aware.average_quoted_spread
                ),
                "Average midpoint deviation": (
                    comparison.inventory_aware
                    .average_book_midpoint_deviation
                ),
            },
            {
                "Strategy": "Fixed-spread baseline",
                "Mean P&L": comparison.fixed_spread.average_pnl,
                "Median P&L": comparison.fixed_spread.median_pnl,
                "P&L standard deviation": (
                    comparison.fixed_spread.pnl_standard_deviation
                ),
                "Profitable sessions": (
                    comparison.fixed_spread.profitable_session_percentage
                ),
                "Average max drawdown": (
                    comparison.fixed_spread.average_maximum_drawdown
                ),
                "Fill rate": comparison.fixed_spread.average_fill_rate,
                "Average max inventory": (
                    comparison.fixed_spread.average_maximum_inventory_exposure
                ),
                "Average quoted spread": (
                    comparison.fixed_spread.average_quoted_spread
                ),
                "Average midpoint deviation": (
                    comparison.fixed_spread.average_book_midpoint_deviation
                ),
            },
        ]
    )
    st.dataframe(
        comparison_data.style.format(
            {
                "Mean P&L": "${:,.2f}",
                "Median P&L": "${:,.2f}",
                "P&L standard deviation": "${:,.2f}",
                "Profitable sessions": "{:.1%}",
                "Average max drawdown": "${:,.2f}",
                "Fill rate": "{:.2%}",
                "Average max inventory": "{:,.2f}",
                "Average quoted spread": "${:,.4f}",
                "Average midpoint deviation": "${:,.4f}",
            }
        ),
        width="stretch",
        hide_index=True,
    )

with st.expander("Per-session results"):
    st.dataframe(
        session_data.style.format(
            {
                "P&L": "${:,.2f}",
                "Realized P&L": "${:,.2f}",
                "Unrealized P&L": "${:,.2f}",
                "Trading fees": "${:,.2f}",
                "Liquidation costs": "${:,.2f}",
                "Maximum drawdown": "${:,.2f}",
                "Terminal reference price": "${:,.2f}",
                "Average quoted spread": "${:,.4f}",
                "Maximum quoted spread": "${:,.4f}",
                "Average maker quote distance": "${:,.4f}",
                "Average midpoint deviation": "${:,.4f}",
                "Average external order lifetime": "{:,.2f}",
                "Fill rate": "{:.2%}",
            }
        ),
        width="stretch",
        hide_index=True,
    )
