#!/bin/bash
cd $(dirname $0)
g++ ../../controller/JobServer.cc ../../controller/ExperimentDataQueue.cc example.cc  FaultCoverageExperiment.pb.cc -o ./ExperimentData_example -l protobuf -pthread
