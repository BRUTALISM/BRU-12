#include "TestPipeline.hpp"

using namespace std;
using namespace beton;

DataOut TestStageProcessor::process(DataIn& data) {
    DataOut result {
        .string = std::to_string(data.number)
    };

    return result;
}

// ------------

TestPipeline::TestPipeline() : inQueue(make_shared<Queue<DataIn>>()), outQueue(make_shared<Queue<DataOut>>()), stage(inQueue, outQueue) {}
