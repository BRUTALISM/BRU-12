#include "Process.hpp"
#include <random>

using namespace std;
using namespace ci;

Process::Process(int nodeCount) {
    std::random_device rd;
    std::mt19937 gen(rd());

    auto xDistribution = uniform_real_distribution<double>(0.0, 10.0);
    auto yzDistribution = uniform_real_distribution<double>(0.0, 2.0);
    auto colorDistribution = uniform_real_distribution<double>(0.0, 1.0);

    nodes.reserve(nodeCount);
    for (int i = 0; i < nodeCount; i++) {
        Node node;
        node.position = vec3(xDistribution(gen), yzDistribution(gen), yzDistribution(gen));
        node.color = Color(colorDistribution(gen), 0.0, 0.0);
        nodes.push_back(node);
    }

    layout.append(geom::Attrib::POSITION, 3, sizeof(Node), offsetof(Node, position));
    layout.append(geom::Attrib::COLOR, 3, sizeof(Node), offsetof(Node, color));
}

void Process::update() {
    for (Node& node: nodes) {
        node.update(perlin);
    }
}
