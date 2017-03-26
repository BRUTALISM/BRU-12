#include "Stage.hpp"

using namespace beton;

template <typename TIn, typename TOut>
Stage<TIn, TOut>::Stage(InQueueRef inQueue, OutQueueRef outQueue, int initialProcessors) : processors(initialProcessors), threads(initialProcessors), inQueue(inQueue), outQueue(outQueue), running(true) {
    for (int i = 0; i < initialProcessors; i++) {
        processors.push_back(InOutProcessor());
    }

    // Fire up a thread for each processor
    for (auto& processor : processors) {
        auto thread = std::thread(&Stage<TIn, TOut>::loop, processor);
        threads.push_back(thread);
    }
}

template <typename TIn, typename TOut>
void Stage<TIn, TOut>::loop(InOutProcessor& processor) {
    while (running) {
        // TODO: Wait with timeout, otherwise thread can't be joined.
        auto dataIn = inQueue.pop();

        outQueue.push(processor.process(dataIn));
    }
}

template <typename TIn, typename TOut>
Stage<TIn, TOut>::~Stage() {
    running = false;

    for (auto& thread : threads) {
        thread.join();
    }
}
