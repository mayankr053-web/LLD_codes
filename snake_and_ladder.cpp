#include <bits/stdc++.h>
using namespace std;

/*-----------------------------------------
    Snake or Ladder
------------------------------------------*/
class Jump {
public:
    int start, end;
    Jump(int s, int e) : start(s), end(e) {}
};

/*-----------------------------------------
    Board
------------------------------------------*/
class Board {
    int size;
    unordered_map<int, int> jumps; // start -> end

public:
    Board(int size) : size(size) {}

    void addSnake(int start, int end) {
        jumps[start] = end;
    }

    void addLadder(int start, int end) {
        jumps[start] = end;
    }

    int getSize() const { return size; }

    int getNextPosition(int cur) const {
        if (jumps.count(cur))
            return jumps.at(cur);
        return cur;
    }
};

/*-----------------------------------------
    Dice
------------------------------------------*/
class Dice {
    int sides;

public:
    Dice(int s = 6) : sides(s) {}

    int roll() const {
        return rand() % sides + 1;
    }
};

/*-----------------------------------------
    Player
------------------------------------------*/
class Player {
    string name;
    int position;

public:
    Player(string n) : name(move(n)), position(0) {}

    string getName() const { return name; }
    int getPosition() const { return position; }

    void setPosition(int pos) { position = pos; }
};

/*-----------------------------------------
    Game
------------------------------------------*/
class Game {
    Board board;
    Dice dice;
    vector<Player> players;
    int currentTurn;

public:
    Game(int boardSize)
        : board(boardSize), dice(6), currentTurn(0) {}

    void addPlayer(const string& name) {
        players.emplace_back(name);
    }

    Board& getBoard() { return board; }

    bool playTurn() {
        Player& player = players[currentTurn];

        int roll = dice.roll();
        int next = player.getPosition() + roll;

        if (next > board.getSize()) {
            // Must land exactly on last square
            cout << player.getName() << " rolled " << roll
                 << " but cannot move.\n";
        } else {
            int finalPos = board.getNextPosition(next);
            cout << player.getName() << " rolled " << roll
                 << " moves from " << player.getPosition()
                 << " to " << finalPos << "\n";

            player.setPosition(finalPos);

            if (finalPos == board.getSize()) {
                cout << "\n🏆 Winner: " << player.getName() << "\n";
                return true;
            }
        }

        currentTurn = (currentTurn + 1) % players.size();
        return false;
    }

    void start() {
        cout << "Starting Snake & Ladder Game...\n\n";
        while (true) {
            if (playTurn()) break;
        }
    }
};

/*-----------------------------------------
    Main (Example Setup)
------------------------------------------*/
int main() {
    srand(time(NULL));

    Game game(100);

    // Snakes
    game.getBoard().addSnake(99, 10);
    game.getBoard().addSnake(70, 55);
    game.getBoard().addSnake(52, 42);

    // Ladders
    game.getBoard().addLadder(3, 22);
    game.getBoard().addLadder(5, 25);
    game.getBoard().addLadder(11, 40);
    game.getBoard().addLadder(20, 60);

    // Add players
    game.addPlayer("Alice");
    game.addPlayer("Bob");

    // Start game
    game.start();

    return 0;
}
