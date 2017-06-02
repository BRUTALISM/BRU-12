#pragma once

#include "StageProcessor.hpp"
#include "PreparedGrid.hpp"
#include "BRU12Pipeline.hpp"

class Muncher: public beton::StageProcessor<BRU12Pipeline::Input, PreparedGrid> {
    bool fill;
    int stepsLeft;
    float currentScale;
public:
    Muncher();
    PreparedGrid process(BRU12Pipeline::Input& input) override;
};
