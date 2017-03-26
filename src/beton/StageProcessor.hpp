#ifndef StageProcessor_hpp
#define StageProcessor_hpp

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

        virtual TOut process(TIn& data) = 0;
    };

}

#endif /* StageProcessor_hpp */
