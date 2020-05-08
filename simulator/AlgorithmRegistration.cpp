//
// Created by Shalev on 05-May-20.
//

#include "Simulator.h"
#include "../interfaces/AlgorithmRegistration.h"

AlgorithmRegistration::AlgorithmRegistration(std::function<std::unique_ptr<AbstractAlgorithm>()> algo_func){
    Simulator::getInstance().registerAlgorithm(algo_func);
}