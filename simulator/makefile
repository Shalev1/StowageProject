COMP = g++-8.3.0
OBJS = main.o Simulator.o ShipPlan.o Floor.o Spot.o Container.o Port.o Route.o Utils.o  WeightBalanceCalculator.o AlgorithmRegistration.o
EXEC = simulator
CPP_COMP_FLAG = -std=c++2a -Wall -Wextra -Werror -pedantic-errors -DNDEBUG -I../common
CPP_LINK_FLAG = -lstdc++fs -ldl -export-dynamic #(-rdynamic)

$(EXEC): $(OBJS)
	$(COMP) $(OBJS) $(CPP_LINK_FLAG) -o $@
AlgorithmRegistration.o: AlgorithmRegistration.cpp ../interfaces/AlgorithmRegistration.h Simulator.h ../common/ShipPlan.h ../common/Floor.h ../common/Spot.h ../common/Container.h
	$(COMP) $(CPP_COMP_FLAG) -c $*.cpp
main.o: main.cpp ../common/ShipPlan.h ../common/Floor.h ../common/Spot.h ../common/Container.h
	$(COMP) $(CPP_COMP_FLAG) -c $*.cpp
Simulator.o: Simulator.cpp Simulator.h ../common/ShipPlan.h ../common/Floor.h ../common/Spot.h ../common/Container.h ../common/Route.h ../common/Port.h ../common/Utils.h ../interfaces/WeightBalanceCalculator.h ../interfaces/AbstractAlgorithm.h
	$(COMP) $(CPP_COMP_FLAG) -c $*.cpp
ShipPlan.o: ../common/ShipPlan.cpp ../common/ShipPlan.h ../common/Floor.h ../common/Spot.h ../common/Container.h
	$(COMP) $(CPP_COMP_FLAG) -c ../common/$*.cpp
Floor.o: ../common/Floor.cpp ../common/Floor.h ../common/Spot.h ../common/Container.h
	$(COMP) $(CPP_COMP_FLAG) -c ../common/$*.cpp
Spot.o: ../common/Spot.cpp ../common/Spot.h ../common/Container.h
	$(COMP) $(CPP_COMP_FLAG) -c ../common/$*.cpp
Container.o: ../common/Container.cpp ../common/Container.h
	$(COMP) $(CPP_COMP_FLAG) -c ../common/$*.cpp
Port.o: ../common/Port.cpp ../common/Port.h ../common/Container.h ../common/Utils.h
	$(COMP) $(CPP_COMP_FLAG) -c ../common/$*.cpp
Route.o: ../common/Route.cpp ../common/Route.h ../common/Port.h ../common/Container.h ../common/Utils.h
	$(COMP) $(CPP_COMP_FLAG) -c ../common/$*.cpp
Utils.o: ../common/Utils.cpp ../common/Utils.h
	$(COMP) $(CPP_COMP_FLAG) -c ../common/$*.cpp
WeightBalanceCalculator.o: ../common/WeightBalanceCalculator.cpp ../interfaces/WeightBalanceCalculator.h ../common/ShipPlan.h ../common/Floor.h ../common/Spot.h ../common/Container.h
	$(COMP) $(CPP_COMP_FLAG) -c ../common/$*.cpp
clean:
	rm -f $(OBJS) $(EXEC)
