#ifndef VolumeNode_hpp
#define VolumeNode_hpp

#include <stdio.h>
#include <math.h>

struct VolumeNode {
    float life;
    ci::Color color;

    VolumeNode() : life(0), color(0, 0, 0) {}
    VolumeNode(double life) : life(life), color(life, life, life) {}
    VolumeNode(int life) : VolumeNode((float)life) {}
    VolumeNode(double life, ci::Color color) : life(life), color(color) {}

    explicit operator double() const { return life; }
    explicit operator float() const { return life; }
};

inline VolumeNode operator+(const VolumeNode& lhs, const VolumeNode& rhs) {
    return VolumeNode(lhs.life + rhs.life, lhs.color + rhs.color);
}

inline VolumeNode operator-(const VolumeNode& lhs, const VolumeNode& rhs) {
    return VolumeNode(lhs.life - rhs.life, lhs.color - rhs.color);
}

inline VolumeNode operator*(const VolumeNode& lhs, const float rhs) {
    return VolumeNode(lhs.life * rhs, lhs.color * rhs);
}

inline VolumeNode operator*(const float lhs, const VolumeNode& rhs) {
    return rhs * lhs;
}

inline VolumeNode operator*(const VolumeNode& lhs, const VolumeNode& rhs) {
    return VolumeNode(lhs.life * rhs.life, lhs.color * rhs.color);
}


inline VolumeNode Abs(const VolumeNode& node) {
    return VolumeNode(std::abs(node.life), node.color);
}

inline bool operator>(const VolumeNode& lhs, const VolumeNode& rhs) {
    return lhs.life > rhs.life;
}

inline bool operator<(const VolumeNode& lhs, const VolumeNode& rhs) {
    return lhs.life < rhs.life;
}

inline bool operator==(const VolumeNode& lhs, const VolumeNode& rhs) {
    // Warning: float equality conversion.
    return lhs.life == rhs.life && lhs.color == rhs.color;
}

inline VolumeNode operator-(const VolumeNode& node) {
    return VolumeNode(-node.life, node.color);
}

inline std::ostream& operator<<(std::ostream& lhs, const VolumeNode& rhs) {
    lhs << "VolumeNode(life: " << rhs.life << ", color: " << rhs.color << ")";
    return lhs;
}

#endif /* VolumeNode_hpp */