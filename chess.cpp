#include <bits/stdc++.h>
using namespace std;

/*-----------------------------------------------------
   Forward declarations
-----------------------------------------------------*/
class Board;
class Piece;

/*-----------------------------------------------------
   Spot
-----------------------------------------------------*/
class Spot {
    unique_ptr<Piece> piece;   // owns the piece
    int x, y;

public:
    Spot(int x = 0, int y = 0, Piece* p = nullptr)
        : piece(p), x(x), y(y) {}

    Piece* getPiece() const { return piece.get(); }
    void setPiece(Piece* p) { piece.reset(p); }

    int getX() const { return x; }
    int getY() const { return y; }
};

/*-----------------------------------------------------
   Piece (Abstract)
-----------------------------------------------------*/
class Piece {
protected:
    bool white;
    bool killed;

public:
    Piece(bool white) : white(white), killed(false) {}
    virtual ~Piece() = default;

    bool isWhite() const { return white; }
    bool isKilled() const { return killed; }
    void setKilled(bool k) { killed = k; }

    virtual bool canMove(Board& board, Spot* start, Spot* end) = 0;
};

/*-----------------------------------------------------
   Board
-----------------------------------------------------*/
class Rook;
class Knight;
class Bishop;
class Queen;
class King;
class Pawn;

class Board {
public:
    unique_ptr<Spot> boxes[8][8];

    Board() { resetBoard(); }

    Spot* getBox(int x, int y) {
        if (x < 0 || x >= 8 || y < 0 || y >= 8)
            throw out_of_range("Invalid position");
        return boxes[x][y].get();
    }

    void resetBoard();
};

/*-----------------------------------------------------
   Piece Implementations (basic rules)
-----------------------------------------------------*/

class King : public Piece {
public:
    King(bool white) : Piece(white) {}

    bool canMove(Board& board, Spot* start, Spot* end) override {
        if (end->getPiece() &&
            end->getPiece()->isWhite() == this->isWhite())
            return false;

        int dx = abs(start->getX() - end->getX());
        int dy = abs(start->getY() - end->getY());

        return (dx <= 1 && dy <= 1); // king moves 1 step
    }
};

class Knight : public Piece {
public:
    Knight(bool white) : Piece(white) {}

    bool canMove(Board& board, Spot* start, Spot* end) override {
        if (end->getPiece() &&
            end->getPiece()->isWhite() == this->isWhite())
            return false;

        int dx = abs(start->getX() - end->getX());
        int dy = abs(start->getY() - end->getY());

        return dx * dy == 2; // L-shape
    }
};

class Rook : public Piece {
public:
    Rook(bool white) : Piece(white) {}

    bool canMove(Board& board, Spot* start, Spot* end) override {
        if (end->getPiece() &&
            end->getPiece()->isWhite() == this->isWhite())
            return false;

        return start->getX() == end->getX() ||
               start->getY() == end->getY();
    }
};

class Bishop : public Piece {
public:
    Bishop(bool white) : Piece(white) {}

    bool canMove(Board& board, Spot* start, Spot* end) override {
        if (end->getPiece() &&
            end->getPiece()->isWhite() == this->isWhite())
            return false;

        return abs(start->getX() - end->getX()) ==
               abs(start->getY() - end->getY());
    }
};

class Queen : public Piece {
public:
    Queen(bool white) : Piece(white) {}

    bool canMove(Board& board, Spot* start, Spot* end) override {
        if (end->getPiece() &&
            end->getPiece()->isWhite() == this->isWhite())
            return false;

        int dx = abs(start->getX() - end->getX());
        int dy = abs(start->getY() - end->getY());

        return dx == dy || start->getX() == end->getX() ||
               start->getY() == end->getY();
    }
};

class Pawn : public Piece {
public:
    Pawn(bool white) : Piece(white) {}

    bool canMove(Board& board, Spot* start, Spot* end) override {
        int direction = white ? 1 : -1;

        // move forward
        if (start->getX() + direction == end->getX() &&
            start->getY() == end->getY() &&
            end->getPiece() == nullptr)
            return true;

        // attack diagonally
        if (start->getX() + direction == end->getX() &&
            abs(start->getY() - end->getY()) == 1 &&
            end->getPiece() &&
            end->getPiece()->isWhite() != this->isWhite())
            return true;

        return false;
    }
};

/*-----------------------------------------------------
   Board::resetBoard()
-----------------------------------------------------*/
void Board::resetBoard() {
    // white
    boxes[0][0] = make_unique<Spot>(0, 0, new Rook(true));
    boxes[0][1] = make_unique<Spot>(0, 1, new Knight(true));
    boxes[0][2] = make_unique<Spot>(0, 2, new Bishop(true));
    boxes[0][3] = make_unique<Spot>(0, 3, new Queen(true));
    boxes[0][4] = make_unique<Spot>(0, 4, new King(true));
    boxes[0][5] = make_unique<Spot>(0, 5, new Bishop(true));
    boxes[0][6] = make_unique<Spot>(0, 6, new Knight(true));
    boxes[0][7] = make_unique<Spot>(0, 7, new Rook(true));

    for (int j = 0; j < 8; j++)
        boxes[1][j] = make_unique<Spot>(1, j, new Pawn(true));

    // black
    boxes[7][0] = make_unique<Spot>(7, 0, new Rook(false));
    boxes[7][1] = make_unique<Spot>(7, 1, new Knight(false));
    boxes[7][2] = make_unique<Spot>(7, 2, new Bishop(false));
    boxes[7][3] = make_unique<Spot>(7, 3, new Queen(false));
    boxes[7][4] = make_unique<Spot>(7, 4, new King(false));
    boxes[7][5] = make_unique<Spot>(7, 5, new Bishop(false));
    boxes[7][6] = make_unique<Spot>(7, 6, new Knight(false));
    boxes[7][7] = make_unique<Spot>(7, 7, new Rook(false));

    for (int j = 0; j < 8; j++)
        boxes[6][j] = make_unique<Spot>(6, j, new Pawn(false));

    // empty
    for (int i = 2; i <= 5; i++)
        for (int j = 0; j < 8; j++)
            boxes[i][j] = make_unique<Spot>(i, j, nullptr);
}

/*-----------------------------------------------------
   Player
-----------------------------------------------------*/
class Player {
    bool whiteSide;

public:
    Player(bool whiteSide) : whiteSide(whiteSide) {}
    bool isWhiteSide() const { return whiteSide; }
};

/*-----------------------------------------------------
   Move
-----------------------------------------------------*/
class Move {
public:
    Player* player;
    Spot* start;
    Spot* end;
    Piece* pieceMoved;
    Piece* pieceKilled;

    Move(Player* p, Spot* s, Spot* e)
        : player(p), start(s), end(e),
          pieceMoved(s->getPiece()), pieceKilled(nullptr) {}
};

/*-----------------------------------------------------
   Game
-----------------------------------------------------*/
enum class GameStatus { ACTIVE, WHITE_WIN, BLACK_WIN };

class Game {
    Player* players[2];
    Player* currentTurn;
    Board board;
    GameStatus status;

public:
    Game(Player* p1, Player* p2) {
        players[0] = p1;
        players[1] = p2;
        currentTurn = p1->isWhiteSide() ? p1 : p2;
        status = GameStatus::ACTIVE;
    }

    bool makeMove(Player* player, int sx, int sy, int ex, int ey) {
        if (player != currentTurn) return false;

        Spot* start = board.getBox(sx, sy);
        Spot* end = board.getBox(ex, ey);

        Piece* piece = start->getPiece();
        if (!piece) return false;
        if (piece->isWhite() != player->isWhiteSide()) return false;

        if (!piece->canMove(board, start, end)) return false;

        // capture
        if (end->getPiece()) {
            end->getPiece()->setKilled(true);
        }

        // move
        end->setPiece(start->getPiece());
        start->setPiece(nullptr);

        // win?
        if (dynamic_cast<King*>(end->getPiece()) &&
            end->getPiece()->isKilled())
        {
            status = player->isWhiteSide()
                         ? GameStatus::WHITE_WIN
                         : GameStatus::BLACK_WIN;
        }

        currentTurn = (currentTurn == players[0] ? players[1] : players[0]);
        return true;
    }
};

/*-----------------------------------------------------
   Main demo
-----------------------------------------------------*/
int main() {
    Player white(true);
    Player black(false);

    Game game(&white, &black);

    // Example move: white pawn forward
    cout << game.makeMove(&white, 1, 0, 2, 0) << endl;  // 1

    // Example illegal move
    cout << game.makeMove(&black, 7, 0, 5, 0) << endl;  // 1 (black rook)
}
