COMP = g++-8.3.0
OBJS = main.o Simulator.o ShipPlan.o Floor.o Spot.o Container.o Port.o Route.o Utils.o BaseAlgorithm.o  WeightBalanceCalculator.o AlgorithmReverse.o
EXEC = ex1
CPP_COMP_FLAG = -std=c++2a -Wall -Wextra -Werror -pedantic-errors -DNDEBUG
CPP_LINK_FLAG = -lstdc++fs

$(EXEC): $(OBJS)
	$(COMP) $(OBJS) $(CPP_LINK_FLAG) -o $@
main.o: main.cpp ShipPlan.h Floor.h Spot.h Container.h
	$(COMP) $(CPP_COMP_FLAG) -c $*.cpp
Simulator.o: Simulator.cpp Simulator.h BaseAlgorithm.h ShipPlan.h Floor.h Spot.h Container.h Route.h Port.h Utils.h WeightBalanceCalculator.h AlgorithmReverse.h
	$(COMP) $(CPP_COMP_FLAG) -c $*.cpp
ShipPlan.o: ShipPlan.cpp ShipPlan.h Floor.h Spot.h Container.h
	$(COMP) $(CPP_COMP_FLAG) -c $*.cpp
Floor.o: Floor.cpp Floor.h Spot.h Container.h
	$(COMP) $(CPP_COMP_FLAG) -c $*.cpp
Spot.o: Spot.cpp Spot.h Container.h
	$(COMP) $(CPP_COMP_FLAG) -c $*.cpp
Container.o: Container.cpp Container.h
	$(COMP) $(CPP_COMP_FLAG) -c $*.cpp
Port.o: Port.cpp Port.h Container.h Utils.h
	$(COMP) $(CPP_COMP_FLAG) -c $*.cpp
Route.o: Route.cpp Route.h Port.h Container.h Utils.h
	$(COMP) $(CPP_COMP_FLAG) -c $*.cpp
Utils.o: Utils.cpp Utils.h
	$(COMP) $(CPP_COMP_FLAG) -c $*.cpp
WeightBalanceCalculator.o: WeightBalanceCalculator.cpp WeightBalanceCalculator.h ShipPlan.h Floor.h Spot.h Container.h
	$(COMP) $(CPP_COMP_FLAG) -c $*.cpp
BaseAlgorithm.o: BaseAlgorithm.cpp BaseAlgorithm.h ShipPlan.h Floor.h Spot.h Container.h Route.h Port.h Utils.h WeightBalanceCalculator.h
	$(COMP) $(CPP_COMP_FLAG) -c $*.cpp
AlgorithmReverse.o: AlgorithmReverse.cpp AlgorithmReverse.h BaseAlgorithm.h ShipPlan.h Route.h
	$(COMP) $(CPP_COMP_FLAG) -c $*.cpp
clean:
	rm -f $(OBJS) $(EXEC)
