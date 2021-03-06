#ifndef TestPipeline_hpp
#define TestPipeline_hpp

#include "Stage.hpp"
#include "StageProcessor.hpp"
#include "Queue.hpp"

struct DataIn {
    const int number;
};

struct DataOut {
    const std::string string;
    const std::thread::id threadId;
};

class TestStageProcessor : public beton::StageProcessor<DataIn, DataOut> {
    DataOut process(DataIn& data);
};

class TestPipeline {
    beton::QueueRef<DataIn> inQueue;
    beton::QueueRef<DataOut> outQueue;
    beton::Stage<DataIn, DataOut> stage;

public:
    TestPipeline();
};

#endif /* TestPipeline_hpp */
