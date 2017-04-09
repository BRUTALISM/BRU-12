#include "BRU12Pipeline.hpp"

using namespace std;

BRU12Pipeline::BRU12Pipeline() {
    inQueue = make_shared<beton::Queue<Input>>();
    outQueue = make_shared<beton::Queue<Output>>();

    // TODO: grid = ...
}
