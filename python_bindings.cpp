#include "experiment.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

PYBIND11_MODULE(orderbook_cpp, module) {
    module.doc() = "C++ limit-order-book market-maker simulation";

    py::class_<MarketMakerConfig>(module, "MarketMakerConfig")
        .def(py::init<>())
        .def_readwrite("quote_size", &MarketMakerConfig::quoteSize)
        .def_readwrite(
            "maximum_inventory",
            &MarketMakerConfig::maximumInventory
        )
        .def_readwrite(
            "transaction_fee_per_unit",
            &MarketMakerConfig::transactionFeePerUnit
        )
        .def_readwrite(
            "quote_latency_steps",
            &MarketMakerConfig::quoteLatencySteps
        )
        .def_readwrite(
            "terminal_liquidation_enabled",
            &MarketMakerConfig::terminalLiquidationEnabled
        )
        .def_readwrite(
            "terminal_liquidation_cost_per_unit",
            &MarketMakerConfig::terminalLiquidationCostPerUnit
        )
        .def_readwrite(
            "inventory_control_enabled",
            &MarketMakerConfig::inventoryControlEnabled
        )
        .def_readwrite(
            "fixed_spread_enabled",
            &MarketMakerConfig::fixedSpreadEnabled
        )
        .def_readwrite("fixed_spread", &MarketMakerConfig::fixedSpread);

    py::class_<SimulatorConfig>(module, "SimulatorConfig")
        .def(py::init<>())
        .def_readwrite("random_seed", &SimulatorConfig::randomSeed)
        .def_readwrite(
            "market_order_probability",
            &SimulatorConfig::marketOrderProbability
        )
        .def_readwrite(
            "maximum_generated_order_quantity",
            &SimulatorConfig::maximumGeneratedOrderQuantity
        )
        .def_readwrite(
            "reference_price_volatility",
            &SimulatorConfig::referencePriceVolatility
        )
        .def_readwrite(
            "reference_price_trend",
            &SimulatorConfig::referencePriceTrend
        )
        .def_readwrite(
            "informed_order_probability",
            &SimulatorConfig::informedOrderProbability
        )
        .def_readwrite(
            "initial_book_depth",
            &SimulatorConfig::initialBookDepth
        )
        .def_readwrite(
            "initial_level_quantity",
            &SimulatorConfig::initialLevelQuantity
        );

    py::class_<SimulationConfig>(module, "SimulationConfig")
        .def(py::init<>())
        .def_readwrite(
            "number_of_sessions",
            &SimulationConfig::numberOfSessions
        )
        .def_readwrite(
            "steps_per_session",
            &SimulationConfig::stepsPerSession
        )
        .def_readwrite("market_maker", &SimulationConfig::marketMaker)
        .def_readwrite("simulator", &SimulationConfig::simulator);

    py::class_<SessionResult>(module, "SessionResult")
        .def_readonly("session_number", &SessionResult::sessionNumber)
        .def_readonly("random_seed", &SessionResult::randomSeed)
        .def_readonly("pnl", &SessionResult::pnl)
        .def_readonly("fill_rate", &SessionResult::fillRate)
        .def_readonly(
            "maximum_inventory_exposure",
            &SessionResult::maximumInventoryExposure
        )
        .def_readonly("ending_inventory", &SessionResult::endingInventory)
        .def_readonly("bought_volume", &SessionResult::boughtVolume)
        .def_readonly("sold_volume", &SessionResult::soldVolume)
        .def_readonly("cash", &SessionResult::cash)
        .def_readonly("inventory_value", &SessionResult::inventoryValue)
        .def_readonly("realized_pnl", &SessionResult::realizedPnL)
        .def_readonly("unrealized_pnl", &SessionResult::unrealizedPnL)
        .def_readonly("trading_fees", &SessionResult::tradingFees)
        .def_readonly(
            "terminal_liquidation_costs",
            &SessionResult::terminalLiquidationCosts
        )
        .def_readonly(
            "terminal_liquidation_quantity",
            &SessionResult::terminalLiquidationQuantity
        )
        .def_readonly(
            "maximum_drawdown",
            &SessionResult::maximumDrawdown
        )
        .def_readonly(
            "terminal_reference_price",
            &SessionResult::terminalReferencePrice
        )
        .def_readonly("ending_equity", &SessionResult::endingEquity)
        .def_readonly(
            "ledger_reconciled",
            &SessionResult::ledgerReconciled
        );

    py::class_<SimulationResult>(module, "SimulationResult")
        .def_readonly(
            "number_of_sessions",
            &SimulationResult::numberOfSessions
        )
        .def_readonly(
            "steps_per_session",
            &SimulationResult::stepsPerSession
        )
        .def_readonly(
            "total_simulated_steps",
            &SimulationResult::totalSimulatedSteps
        )
        .def_readonly("average_pnl", &SimulationResult::averagePnL)
        .def_readonly("median_pnl", &SimulationResult::medianPnL)
        .def_readonly(
            "pnl_standard_deviation",
            &SimulationResult::pnlStandardDeviation
        )
        .def_readonly("best_pnl", &SimulationResult::bestPnL)
        .def_readonly("worst_pnl", &SimulationResult::worstPnL)
        .def_readonly(
            "profitable_sessions",
            &SimulationResult::profitableSessions
        )
        .def_readonly(
            "profitable_session_percentage",
            &SimulationResult::profitableSessionPercentage
        )
        .def_readonly(
            "average_fill_rate",
            &SimulationResult::averageFillRate
        )
        .def_readonly(
            "average_maximum_inventory_exposure",
            &SimulationResult::averageMaximumInventoryExposure
        )
        .def_readonly(
            "worst_inventory_exposure",
            &SimulationResult::worstInventoryExposure
        )
        .def_readonly(
            "average_maximum_drawdown",
            &SimulationResult::averageMaximumDrawdown
        )
        .def_readonly(
            "total_trading_fees",
            &SimulationResult::totalTradingFees
        )
        .def_readonly(
            "total_terminal_liquidation_costs",
            &SimulationResult::totalTerminalLiquidationCosts
        )
        .def_readonly(
            "all_sessions_reconciled",
            &SimulationResult::allSessionsReconciled
        )
        .def_readonly(
            "total_bought_volume",
            &SimulationResult::totalBoughtVolume
        )
        .def_readonly(
            "total_sold_volume",
            &SimulationResult::totalSoldVolume
        )
        .def_readonly(
            "session_results",
            &SimulationResult::sessionResults
        );

    py::class_<StrategyComparisonResult>(module, "StrategyComparisonResult")
        .def_readonly(
            "inventory_aware",
            &StrategyComparisonResult::inventoryAware
        )
        .def_readonly(
            "fixed_spread",
            &StrategyComparisonResult::fixedSpread
        );

    module.def(
        "run_simulation",
        [](const SimulationConfig& config) {
            py::gil_scoped_release release;
            return runSimulation(config);
        },
        py::arg("config"),
        "Run the C++ market-maker experiment and return structured results."
    );

    module.def(
        "compare_strategies",
        [](const SimulationConfig& config) {
            py::gil_scoped_release release;
            return compareStrategies(config);
        },
        py::arg("config"),
        "Compare inventory-aware and fixed-spread strategies with equal seeds."
    );
}
