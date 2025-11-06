#include <bits/stdc++.h>
using namespace std;

/*---------------------------------------------------
    Player
---------------------------------------------------*/
class Player {
    string name;
    char symbol; // 'X' or 'O'

public:
    Player(string n, char s) : name(move(n)), symbol(s) {}

    string getName() const { return name; }
    char getSymbol() const { return symbol; }
};

/*---------------------------------------------------
    Board
---------------------------------------------------*/
class Board {
    int size;
    vector<vector<char>> grid;

public:
    Board(int n = 3) : size(n), grid(n, vector<char>(n, ' ')) {}

    bool isCellFree(int r, int c) const {
        return grid[r][c] == ' ';
    }

    bool markCell(int r, int c, char symbol) {
        if (!isCellFree(r, c)) return false;
        grid[r][c] = symbol;
        return true;
    }

    void print() const {
        cout << "\n";
        for (int i = 0; i < size; i++) {
            cout << " ";
            for (int j = 0; j < size; j++) {
                cout << grid[i][j];
                if (j < size - 1) cout << " | ";
            }
            cout << "\n";
            if (i < size - 1) cout << string(size * 4 - 3, '-') << "\n";
        }
        cout << "\n";
    }

    bool checkWin(char symbol) const {
        // rows
        for (int i = 0; i < size; i++) {
            bool win = true;
            for (int j = 0; j < size; j++)
                if (grid[i][j] != symbol) win = false;
            if (win) return true;
        }

        // cols
        for (int j = 0; j < size; j++) {
            bool win = true;
            for (int i = 0; i < size; i++)
                if (grid[i][j] != symbol) win = false;
            if (win) return true;
        }

        // main diagonal
        bool diag1 = true;
        for (int i = 0; i < size; i++)
            if (grid[i][i] != symbol) diag1 = false;
        if (diag1) return true;

        // anti-diagonal
        bool diag2 = true;
        for (int i = 0; i < size; i++)
            if (grid[i][size - i - 1] != symbol) diag2 = false;
        if (diag2) return true;

        return false;
    }

    bool checkDraw() const {
        for (int i = 0; i < size; i++)
            for (int j = 0; j < size; j++)
                if (grid[i][j] == ' ')
                    return false;
        return true;
    }
};

/*---------------------------------------------------
    Game
---------------------------------------------------*/
class Game {
    Board board;
    Player p1;
    Player p2;
    Player* current;

public:
    Game(string p1Name, string p2Name)
        : board(3),
          p1(p1Name, 'X'),
          p2(p2Name, 'O'),
          current(&p1)
    {}

    void switchTurn() {
        current = (current == &p1) ? &p2 : &p1;
    }

    void start() {
        cout << "Tic Tac Toe Started!\n";

        while (true) {
            board.print();
            cout << current->getName() << " (" << current->getSymbol()
                 << ") turn. Enter row & col (0-based): ";

            int r, c;
            cin >> r >> c;

            if (!cin || r < 0 || r > 2 || c < 0 || c > 2) {
                cout << "Invalid input, try again!\n";
                cin.clear();
                cin.ignore(1000, '\n');
                continue;
            }

            if (!board.markCell(r, c, current->getSymbol())) {
                cout << "Cell already occupied. Try again.\n";
                continue;
            }

            // check win
            if (board.checkWin(current->getSymbol())) {
                board.print();
                cout << "🏆 " << current->getName() << " wins!\n";
                break;
            }

            // check draw
            if (board.checkDraw()) {
                board.print();
                cout << "It's a draw!\n";
                break;
            }

            // next player's turn
            switchTurn();
        }
    }
};

/*---------------------------------------------------
    Main
---------------------------------------------------*/
int main() {
    Game game("Player1", "Player2");
    game.start();
    return 0;
}
