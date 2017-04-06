#ifndef TestPipeline_hpp
#define TestPipeline_hpp

#include "Stage.hpp"
#include "StageProcessor.hpp"
#include "Queue.hpp"

struct DataIn {
    int number;
};

struct DataOut {
    std::string string;
};

class TestStageProcessor : beton::StageProcessor<DataIn, DataOut> {
    DataOut process(DataIn& data);
};

class TestPipeline {
    beton::QueueRef<DataIn> inQueue;
    beton::QueueRef<DataOut> outQueue;
    beton::Stage<DataIn, DataOut> stage;

    TestPipeline();
};

#endif /* TestPipeline_hpp */
