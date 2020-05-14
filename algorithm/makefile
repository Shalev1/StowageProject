COMP = g++-8.3.0

SHARED_OBJS = _206223976_a.so _206223976_b.so
OBJS1 = ShipPlan.o Floor.o Spot.o Container.o Port.o Route.o Utils.o  WeightBalanceCalculator.o BaseAlgorithm.o _206223976_a.o
OBJS2 = ShipPlan.o Floor.o Spot.o Container.o Port.o Route.o Utils.o  WeightBalanceCalculator.o BaseAlgorithm.o _206223976_b.o
CPP_COMP_FLAG = -std=c++2a -Wall -Wextra -Werror -pedantic-errors -DNDEBUG -fPIC -I../common
CPP_LINK_FLAG = -shared

all: $(SHARED_OBJS)

_206223976_a.so: $(OBJS1)
	$(COMP) $(CPP_LINK_FLAG) -o $@ $^
_206223976_b.so: $(OBJS2)
	$(COMP) $(CPP_LINK_FLAG) -o $@ $^

ShipPlan.o: ../common/ShipPlan.cpp ../common/ShipPlan.h ../common/Floor.h ../common/Spot.h ../common/Container.h
	$(COMP) $(CPP_COMP_FLAG) -c ../common/$*.cpp
Floor.o: ../common/Floor.cpp ../common/Floor.h ../common/Spot.h ../common/Container.h
	$(COMP) $(CPP_COMP_FLAG) -c ../common/$*.cpp
Spot.o: ../common/Spot.cpp ../common/Spot.h ../common/Container.h
	$(COMP) $(CPP_COMP_FLAG) -c ../common/$*.cpp
Container.o: ../common/Container.cpp ../common/Container.h
	$(COMP) $(CPP_COMP_FLAG) -c ../common/$*.cpp
Port.o: ../common/Port.cpp ../common/Port.h ../common/Container.h ../common/Utils.h ../common/ShipPlan.h
	$(COMP) $(CPP_COMP_FLAG) -c ../common/$*.cpp
Route.o: ../common/Route.cpp ../common/Route.h ../common/Port.h ../common/Container.h ../common/Utils.h ../common/ShipPlan.h
	$(COMP) $(CPP_COMP_FLAG) -c ../common/$*.cpp
Utils.o: ../common/Utils.cpp ../common/Utils.h
	$(COMP) $(CPP_COMP_FLAG) -c ../common/$*.cpp
WeightBalanceCalculator.o: ../common/WeightBalanceCalculator.cpp ../interfaces/WeightBalanceCalculator.h ../common/ShipPlan.h ../common/Floor.h ../common/Spot.h ../common/Container.h
	$(COMP) $(CPP_COMP_FLAG) -c ../common/$*.cpp
BaseAlgorithm.o: BaseAlgorithm.cpp BaseAlgorithm.h ../common/ShipPlan.h ../common/Floor.h ../common/Spot.h ../common/Container.h ../common/Route.h ../common/Port.h ../common/Utils.h ../interfaces/WeightBalanceCalculator.h
	$(COMP) $(CPP_COMP_FLAG) -c $*.cpp
_206223976_a.o: _206223976_a.cpp _206223976_a.h BaseAlgorithm.h
	$(COMP) $(CPP_COMP_FLAG) -c $*.cpp
_206223976_b.o: _206223976_b.cpp _206223976_b.h BaseAlgorithm.h
	$(COMP) $(CPP_COMP_FLAG) -c $*.cpp

clean:
	rm -f $(OBJS1) $(OBJS2) $(SHARED_OBJS)
