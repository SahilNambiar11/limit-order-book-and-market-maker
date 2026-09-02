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
        );

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
        .def_readonly("sold_volume", &SessionResult::soldVolume);

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
        .def_readonly("best_pnl", &SimulationResult::bestPnL)
        .def_readonly("worst_pnl", &SimulationResult::worstPnL)
        .def_readonly(
            "profitable_sessions",
            &SimulationResult::profitableSessions
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

    module.def(
        "run_simulation",
        [](const SimulationConfig& config) {
            py::gil_scoped_release release;
            return runSimulation(config);
        },
        py::arg("config"),
        "Run the C++ market-maker experiment and return structured results."
    );
}
