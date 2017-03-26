#ifndef Stage_hpp
#define Stage_hpp

#include <thread>

#include "Queue.hpp"
#include "StageProcessor.hpp"

namespace beton {

    /// Represents a single stage in the processing pipeline, parameterized by the type of input
    /// data and type of output data. Contains one or more StageProcessors, each running on a
    /// separate thread.
    template <typename TIn, typename TOut>
    class Stage {
        using InQueueRef = std::shared_ptr<Queue<TIn>>;
        using OutQueueRef = std::shared_ptr<Queue<TOut>>;
        using InOutProcessor = StageProcessor<TIn, TOut>;

        std::vector<std::thread> threads;
        std::vector<InOutProcessor> processors;
        std::atomic<bool> running;

        InQueueRef inQueue;
        OutQueueRef outQueue;

        void loop(InOutProcessor& processor);

    public:
        Stage(InQueueRef inQueue, OutQueueRef outQueue, int initialProcessors);
        ~Stage();
    };

}

#endif /* Stage_hpp */
