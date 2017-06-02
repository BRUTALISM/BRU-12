#pragma once

#include <random>
#include "StageProcessor.hpp"
#include "PreparedGrid.hpp"
#include "BRU12Pipeline.hpp"

class Decay: public beton::StageProcessor<PreparedGrid, PreparedGrid> {
    std::mt19937 generator;
    std::uniform_real_distribution<double> decayJitter;

public:
    PreparedGrid process(PreparedGrid& input) override;
};
