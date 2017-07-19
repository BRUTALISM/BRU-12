#pragma once

#include <stdio.h>

namespace beton {
    /// Abstract base class for a processing stage. Receives input of type TIn, performs work, and
    /// creates output of type TOut. Subclasses only need to override the process method; all the
    /// asynchronous producer/consumer code is encapsulated here.
    template<typename TIn, typename TOut>
    class StageProcessor {
    public:
        StageProcessor() = default;
        virtual ~StageProcessor() = default;

        /// Optional setup function, invoked once on the thread dedicated to running the process fn.
        virtual void threadSetup() {}

        virtual TOut process(TIn& data) = 0;
    };
}
