#pragma once

#include <random>
#include "StageProcessor.hpp"
#include "cinder/Perlin.h"
#include "PreparedGrid.hpp"
#include "BRU12Pipeline.hpp"

class Decay: public beton::StageProcessor<BRU12Pipeline::Input, PreparedGrid> {
    std::mt19937 generator;
    std::uniform_real_distribution<double> decayJitter;
    ci::Perlin perlin;

public:
    PreparedGrid process(BRU12Pipeline::Input& input) override;
};
