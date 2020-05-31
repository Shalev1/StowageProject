cmake_minimum_required(VERSION 3.13.2)
project(ShipProject)

set(CMAKE_CXX_STANDARD 20)

#add_executable(ShipProject main.cpp Container.cpp Container.h Route.cpp Route.h Port.cpp Port.h)
add_executable(ShipProject simulator/main.cpp common/Route.cpp common/Route.h common/Port.cpp common/Port.h common/Container.cpp common/Container.h common/Spot.h common/Floor.h common/Utils.cpp common/Utils.h common/ShipPlan.cpp common/ShipPlan.h common/Spot.cpp common/Spot.h common/Floor.cpp common/Floor.h simulator/Simulator.cpp simulator/Simulator.h algorithm/_206223976_a.cpp algorithm/_206223976_a.h common/WeightBalanceCalculator.cpp interfaces/WeightBalanceCalculator.h algorithm/_206223976_b.cpp algorithm/_206223976_b.h interfaces/AbstractAlgorithm.h algorithm/BaseAlgorithm.cpp algorithm/BaseAlgorithm.h algorithm/_206223976_c.cpp algorithm/_206223976_c.h common/ISO_6346.cpp common/ISO_6346.h simulator/ThreadPool.cpp simulator/ThreadPool.h)