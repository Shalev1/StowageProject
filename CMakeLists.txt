cmake_minimum_required(VERSION 3.15)
project(ShipProject)

set(CMAKE_CXX_STANDARD 20)

#add_executable(ShipProject main.cpp Container.cpp Container.h Route.cpp Route.h Port.cpp Port.h)
add_executable(ShipProject main.cpp myMain.cpp Route.cpp Route.h Port.cpp Port.h Container.cpp Container.h Spot.h Floor.h Utils.cpp Utils.h shipPlan.cpp shipPlan.h spot.cpp spot.h floor.cpp floor.h SimTest.cpp SimTest.h Simulator.cpp Simulator.h Algorithm.cpp Algorithm.h WeightBalanceCalculator.cpp WeightBalanceCalculator.h AlgorithmReverse.cpp AlgorithmReverse.h)