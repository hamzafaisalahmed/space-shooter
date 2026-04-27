#pragma once

#include <iostream>
#include <string>

class ScoreEvent
{
public:
    std::string description;
    int points;
    float timestamp;
    ScoreEvent() : description(""), points(0), timestamp(0.f) {}
    ScoreEvent(const std::string &d, int p, float t)
        : description(d), points(p), timestamp(t) {}
    friend std::ostream &operator<<(std::ostream &os, const ScoreEvent &ev)
    {
        os << ev.description << " +" << ev.points;
        return os;
    }
};
