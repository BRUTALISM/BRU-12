#pragma once

#include "BRU12Pipeline.hpp"
#include "PreparedGrid.hpp"
#include "StageProcessor.hpp"

class Mesher : public beton::StageProcessor<PreparedGrid, BRU12Pipeline::Output> {
    ci::gl::ContextRef glContext;

public:
    Mesher(ci::gl::ContextRef context);
    void threadSetup() override;

    BRU12Pipeline::Output process(PreparedGrid& input) override;
};
