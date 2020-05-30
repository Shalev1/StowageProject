
#include "Simulator.h"

enum PathType {
    Travel, Algo, Output, NumThreads, None
};

PathType getTypeOfPath(const string &input) {
    if (input == "-travel_path") return Travel;
    if (input == "-algorithm_path") return Algo;
    if (input == "-output") return Output;
    if (input == "-num_threads") return NumThreads;
    return None;

}

bool initializeParameters(string &travel_path, string &algorithm_path, string &output_path, unsigned int &num_of_threads,
                          int num_of_params,
                          char *argv[]) {
    if (num_of_params < 2 || num_of_params % 2 == 0) {
        cout << "@ FATAL ERROR: Wrong number of arguments was given." << endl;
        return false;
    }
    for (int i = 1; i < num_of_params; i += 2) {
        PathType file_type = getTypeOfPath(string(argv[i]));
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
            case NumThreads: {
                if (!isPositiveNumber(argv[i + 1]) || string2int(argv[i + 1]) < 1) {
                    cout << "@ ERROR: Number of threads given is invalid, setting the number to 1." << endl;
                    num_of_threads = 1;
                } else
                    num_of_threads = (unsigned int)string2int(argv[i + 1]);
                break;
            }
            case None: {
                cout << "@ FATAL ERROR: Invalid parameters was given." << endl;
                return false;
            }
        }
    }
    if (travel_path.empty()) {
        cout << "@ FATAL ERROR: No travel_path was given." << endl;
        return false; // return false if there was not -travel_path param
    }
    return true;
}

int main(int argc, char *argv[]) {
    string travel_path = "";
    string algorithm_path = "";
    string output_path = "";
    unsigned int num_of_threads = 1;
    bool clean_run;
    if (argc > 9) {
        cout << "@ FATAL ERROR: Too many arguments given." << endl;
        return EXIT_FAILURE;
    }
    if (!initializeParameters(travel_path, algorithm_path, output_path, num_of_threads, argc, argv)) {
        // README: if any flag is declared and the path given is empty, an error will be printed and the simulation will not start.
        return EXIT_FAILURE;
    }
    cout << "Threads given:" << num_of_threads << endl;
    Simulator sim(output_path, num_of_threads);
    clean_run = sim.start(algorithm_path, travel_path);
    sim.printSimulationErrors();
    if (clean_run) {
        sim.printSimulationResults();
    }
    return EXIT_SUCCESS;
}
