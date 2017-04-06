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
        using InOutProcessor = StageProcessor<TIn, TOut>;

        std::vector<std::thread> threads;
        std::vector<std::shared_ptr<InOutProcessor>> processors;
        std::atomic<bool> running;

        QueueRef<TIn> inQueue;
        QueueRef<TOut> outQueue;

        void loop(InOutProcessor& processor) {
            while (running) {
                // Wait with timeout, otherwise thread can't be joined.
                boost::optional<TIn> dataIn = inQueue->tryPop(std::chrono::milliseconds(1000));

                if (dataIn) {
                    outQueue->push(processor.process(*dataIn));
                }
            }
        }

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

        void pushProcessor(std::shared_ptr<InOutProcessor> processor) {
            processors.push_back(processor);

            auto thread = std::thread(&Stage<TIn, TOut>::loop, this, processor);
            threads.push_back(thread);
        }
    };
}
