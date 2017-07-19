#pragma once

#include <thread>
#include <boost/optional.hpp>

#include "Queue.hpp"
#include "StageProcessor.hpp"

namespace beton {
    /// Represents a single stage in the processing pipeline, parameterized by the type of input
    /// data and type of output data. Contains one or more StageProcessors, each running on a
    /// separate thread.
    template <typename TIn, typename TOut>
    class Stage {
        std::vector<std::thread> threads;
        std::vector<std::shared_ptr<StageProcessor<TIn, TOut>>> processors;
        std::atomic<bool> running;

        QueueRef<TIn> inQueue;
        QueueRef<TOut> outQueue;

    public:
        Stage(QueueRef<TIn> inQueue, QueueRef<TOut> outQueue) :
            inQueue(inQueue), outQueue(outQueue), running(true) {}

        ~Stage() {
            running = false;

            for (auto& thread : threads) {
                thread.join();
            }
        }

        const QueueRef<TIn> getInQueue() { return inQueue; }
        const QueueRef<TOut> getOutQueue() { return outQueue; }

        void pushProcessor(std::shared_ptr<StageProcessor<TIn, TOut>> processor) {
            processors.push_back(processor);
            threads.push_back(std::thread([=] () {
                processor->threadSetup();

                while (running) {
                    // Wait with timeout, otherwise thread can't be joined.
                    boost::optional<TIn> dataIn = inQueue->tryPop(std::chrono::milliseconds(1000));

                    if (dataIn) {
                        auto data = *dataIn;
                        auto result = processor->process(data);
                        outQueue->push(result);
                    }
                }
            }));
        }
    };
}
