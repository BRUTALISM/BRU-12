#pragma once

struct Params {
	const ci::vec3 volumeBounds;
	const int densityPerUnit;
	const float gridFillValue;
	const float gridBackgroundValue;
	const float isoValue;
	const float decayMultiplier;
    const float growthMultiplier;
};
