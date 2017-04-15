#pragma once

#include <random>
#include "StageProcessor.hpp"
#include "cinder/Perlin.h"

#include "BRU12Pipeline.hpp"

class DecayProcessor: public beton::StageProcessor<BRU12Pipeline::Input, BRU12Pipeline::Output> {
    std::mt19937 generator;
    std::uniform_real_distribution<double> decayJitter;
    ci::Perlin perlin;

    BRU12Pipeline::Output process(BRU12Pipeline::Input& input);
};
