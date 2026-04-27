#pragma once

class GameState
{
private:
    int id;

public:
    GameState(int i = 0) : id(i) {}
    int getId() const { return id; }
    bool operator==(const GameState &o) const { return id == o.id; }
    bool operator!=(const GameState &o) const { return id != o.id; }
    static const GameState Home;
    static const GameState LevelSelect;
    static const GameState Playing;
    static const GameState Paused;
    static const GameState GameOver;
    static const GameState LevelComplete;
};
