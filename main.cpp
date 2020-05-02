
#include "Simulator.h"

enum PathType {
    Travel, Algo, Output, None
};

PathType getTypeOfPath(const string &input) {
    if (input == "-travel_path") return Travel;
    if (input == "-algorithm_path") return Algo;
    if (input == "-output") return Output;
    return None;

}

bool initializeParameters(string &travel_path, string &algorithm_path, string &output_path, int num_of_params,
                          char *argv[]) {
    if (num_of_params < 2 || num_of_params % 2 == 0) {
        return false;
    }
    for (int i = 1; i < num_of_params; i += 2) {
        PathType file_type = getTypeOfPath(string(argv[i]));
        if (file_type == None) {
            return false;
        }
        switch (file_type) {
            case Travel: {
                if (!travel_path.empty()) return false; //travel_path was already initialized
                travel_path = argv[i + 1]; // i+1 must exists because of the way we iterate through the words
                break;
            }
            case Algo: {
                if (!algorithm_path.empty()) return false; //algorithm_path was already initialized
                algorithm_path = argv[i + 1]; // i+1 must exists because of the way we iterate through the words
                break;
            }
            case Output: {
                if (!output_path.empty()) return false; //output_path was already initialized
                output_path = argv[i + 1]; // i+1 must exists because of the way we iterate through the words
                break;
            }
            case None:{
                break;
            }
        }
    }
    return true;
}

int main(int argc, char *argv[]) {
    string travel_path = "";
    string algorithm_path = "";
    string output_path = "";
    bool clean_run;
    if (argc > 7) {
        cout << "ERROR: Too many agruments given." << endl;
        return EXIT_FAILURE;
    }

    if (!initializeParameters(travel_path, algorithm_path, output_path, argc, argv)) {
        cout << "ERROR: Invalid parameters given." << endl;
        return EXIT_FAILURE;
    }

    Simulator sim(output_path);
    clean_run = sim.runSimulation(algorithm_path, travel_path);
    sim.printSimulationErrors();
    if(clean_run){
        sim.printSimulationResults();
    }
    return EXIT_SUCCESS;
}
