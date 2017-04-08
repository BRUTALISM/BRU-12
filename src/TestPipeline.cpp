#include "TestPipeline.hpp"

using namespace std;
using namespace beton;

const int NUMBER_OF_PROCESSORS = 5;
const int NUMBER_OF_INPUTS = 10000;

DataOut TestStageProcessor::process(DataIn& data) {
    DataOut result {
        .string = std::to_string(data.number),
        .threadId = std::this_thread::get_id()
    };

    return result;
}

// ------------

TestPipeline::TestPipeline() : inQueue(make_shared<Queue<DataIn>>()), outQueue(make_shared<Queue<DataOut>>()), stage(inQueue, outQueue) {
    for (int i = 0; i < NUMBER_OF_PROCESSORS; i++) {
        stage.pushProcessor(make_shared<TestStageProcessor>());
    }

    for (int i = 0; i < NUMBER_OF_INPUTS; i++) {
        inQueue->push(DataIn { .number = i });
    }

    for (int i = 0; i < NUMBER_OF_INPUTS; i++) {
        auto output = outQueue->pop();
        cout << "Received: " << output.string << " from " << output.threadId << endl;
    }
}
