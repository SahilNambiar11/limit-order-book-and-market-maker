CXX ?= g++
PYTHON ?= python3
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -pedantic

CORE_SOURCES = order.cpp trade.cpp orderbook.cpp simulator.cpp marketmaker.cpp experiment.cpp
CORE_HEADERS = order.hpp trade.hpp orderbook.hpp simulator.hpp marketmaker.hpp experiment.hpp

PYTHON_EXTENSION_SUFFIX := $(shell $(PYTHON) -c "import sysconfig; print(sysconfig.get_config_var('EXT_SUFFIX') or '.so')")
PYTHON_MODULE = orderbook_cpp$(PYTHON_EXTENSION_SUFFIX)
PYBIND11_INCLUDES := $(shell $(PYTHON) -m pybind11 --includes 2>/dev/null)
UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Darwin)
PYTHON_MODULE_LDFLAGS = -undefined dynamic_lookup
endif

.PHONY: all run test python-module streamlit clean

all: orderbook

orderbook: main.cpp $(CORE_SOURCES) $(CORE_HEADERS)
	$(CXX) $(CXXFLAGS) main.cpp $(CORE_SOURCES) -o $@

orderbook_tests: tests.cpp $(CORE_SOURCES) $(CORE_HEADERS)
	$(CXX) $(CXXFLAGS) tests.cpp $(CORE_SOURCES) -o $@

run: orderbook
	./orderbook

test: orderbook_tests
	./orderbook_tests

python-module: $(PYTHON_MODULE)

$(PYTHON_MODULE): python_bindings.cpp $(CORE_SOURCES) $(CORE_HEADERS)
	@if [ -z "$(PYBIND11_INCLUDES)" ]; then \
		echo "pybind11 is missing; run: $(PYTHON) -m pip install -r requirements.txt"; \
		exit 1; \
	fi
	$(CXX) $(CXXFLAGS) -shared -fPIC $(PYBIND11_INCLUDES) \
		python_bindings.cpp $(CORE_SOURCES) $(PYTHON_MODULE_LDFLAGS) -o $@

streamlit: python-module
	$(PYTHON) -m streamlit run streamlit_app.py

clean:
	rm -f orderbook orderbook_tests orderbook_cpp*.so orderbook_cpp*.dylib orderbook_cpp*.pyd
