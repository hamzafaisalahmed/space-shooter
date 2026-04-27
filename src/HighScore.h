#pragma once

#include <iostream>
#include <string>

class HighScore
{
private:
    std::string playerName;
    int score;
    int levelIndex;

public:
    HighScore() : playerName("---"), score(0), levelIndex(0) {}
    HighScore(const std::string &n, int s, int l)
        : playerName(n), score(s), levelIndex(l) {}

    std::string getName() const { return playerName; }
    int getScore() const { return score; }
    int getLevelIndex() const { return levelIndex; }

    void setName(const std::string &n) { playerName = n; }
    void setScore(int s) { score = s; }
    void setLevelIndex(int l) { levelIndex = l; }

    bool operator<(const HighScore &o) const { return score > o.score; }
    bool operator>(const HighScore &o) const { return score < o.score; }
    bool operator==(const HighScore &o) const
    {
        return score == o.score && playerName == o.playerName;
    }

    friend std::ostream &operator<<(std::ostream &os, const HighScore &hs)
    {
        os << hs.playerName << " - " << hs.score;
        return os;
    }
};
