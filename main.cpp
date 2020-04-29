
#include "Simulator.h"




int main(int argc, char *argv[])
{
	if (argc != 2)
	{
	    cout << "ERROR: expected exactly one command line parameter" << endl;
		return EXIT_FAILURE;
	}
	Simulator sim(argv[1]);
	sim.runSimulation();

	sim.printSimulationDetails();
	return EXIT_SUCCESS;
}
