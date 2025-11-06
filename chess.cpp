#include <bits/stdc++.h>
using namespace std;

/*-----------------------------------------------------
   Spot
-----------------------------------------------------*/
class Piece; // forward

class Spot {
    Piece* piece;
    int x, y;

public:
    Spot(int x = 0, int y = 0, Piece* piece = nullptr)
        : x(x), y(y), piece(piece) {}

    Piece* getPiece() const { return piece; }
    void setPiece(Piece* p) { piece = p; }

    int getX() const { return x; }
    int getY() const { return y; }

    void setX(int xx) { x = xx; }
    void setY(int yy) { y = yy; }
};

/*-----------------------------------------------------
   Piece (Abstract)
-----------------------------------------------------*/
class Board;

class Piece {
protected:
    bool killed;
    bool white;

public:
    Piece(bool white) : white(white), killed(false) {}

    bool isWhite() const { return white; }
    bool isKilled() const { return killed; }

    void setWhite(bool w) { white = w; }
    void setKilled(bool k) { killed = k; }

    virtual bool canMove(Board& board, Spot* start, Spot* end) = 0;

    virtual ~Piece() = default;
};

/*-----------------------------------------------------
   King
-----------------------------------------------------*/
class King : public Piece {
    bool castlingDone;

public:
    King(bool white) : Piece(white), castlingDone(false) {}

    bool isCastlingDone() const { return castlingDone; }
    void setCastlingDone(bool v) { castlingDone = v; }

    bool canMove(Board& board, Spot* start, Spot* end) override {
        if (end->getPiece() != nullptr &&
            end->getPiece()->isWhite() == this->isWhite())
            return false;

        int x = abs(start->getX() - end->getX());
        int y = abs(start->getY() - end->getY());

        // normal king move
        if (x + y == 1) return true;

        // else try castling
        return isValidCastling(board, start, end);
    }

    bool isValidCastling(Board& board, Spot* start, Spot* end);
    bool isCastlingMove(Spot* start, Spot* end);
};

/*-----------------------------------------------------
   Board
-----------------------------------------------------*/
class Rook;
class Knight;
class Bishop;
class Queen;
class Pawn;

class Board {
public:
    Spot* boxes[8][8];

    Board() { resetBoard(); }

    Spot* getBox(int x, int y) {
        if (x < 0 || x > 7 || y < 0 || y > 7)
            throw out_of_range("Index out of bound");
        return boxes[x][y];
    }

    void resetBoard();
};

/*-----------------------------------------------------
   Other Pieces
-----------------------------------------------------*/
class Rook : public Piece {
public:
    Rook(bool white) : Piece(white) {}

    bool canMove(Board& board, Spot* start, Spot* end) override {
        if (end->getPiece() != nullptr &&
            end->getPiece()->isWhite() == this->isWhite())
            return false;

        // rook moves straight
        return (start->getX() == end->getX() ||
                start->getY() == end->getY());
    }
};

class Knight : public Piece {
public:
    Knight(bool white) : Piece(white) {}

    bool canMove(Board& board, Spot* start, Spot* end) override {
        if (end->getPiece() != nullptr &&
            end->getPiece()->isWhite() == this->isWhite())
            return false;

        int dx = abs(start->getX() - end->getX());
        int dy = abs(start->getY() - end->getY());
        return dx * dy == 2;
    }
};

class Bishop : public Piece {
public:
    Bishop(bool white) : Piece(white) {}

    bool canMove(Board& board, Spot* start, Spot* end) override {
        if (end->getPiece() != nullptr &&
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
        if (end->getPiece() != nullptr &&
            end->getPiece()->isWhite() == this->isWhite())
            return false;

        int dx = abs(start->getX() - end->getX());
        int dy = abs(start->getY() - end->getY());

        return (start->getX() == end->getX() ||
                start->getY() == end->getY() ||
                dx == dy);
    }
};

class Pawn : public Piece {
public:
    Pawn(bool white) : Piece(white) {}

    bool canMove(Board& board, Spot* start, Spot* end) override {
        // extremely simplified — same as your Java template
        if (end->getPiece() != nullptr &&
            end->getPiece()->isWhite() == this->isWhite())
            return false;

        int direction = isWhite() ? 1 : -1;

        if (start->getX() + direction == end->getX() &&
            start->getY() == end->getY() &&
            end->getPiece() == nullptr)
            return true;

        return false;
    }
};

/*-----------------------------------------------------
   Implement castling logic of King
-----------------------------------------------------*/
bool King::isValidCastling(Board& board, Spot* start, Spot* end) {
    if (this->isCastlingDone()) return false;

    // You can add your castling rules here
    return false; // placeholder like your Java code
}

bool King::isCastlingMove(Spot* start, Spot* end) {
    return false; // placeholder
}

/*-----------------------------------------------------
   Board::resetBoard()
-----------------------------------------------------*/
void Board::resetBoard() {
    // White pieces
    boxes[0][0] = new Spot(0, 0, new Rook(true));
    boxes[0][1] = new Spot(0, 1, new Knight(true));
    boxes[0][2] = new Spot(0, 2, new Bishop(true));
    boxes[0][3] = new Spot(0, 3, new Queen(true));
    boxes[0][4] = new Spot(0, 4, new King(true));
    boxes[0][5] = new Spot(0, 5, new Bishop(true));
    boxes[0][6] = new Spot(0, 6, new Knight(true));
    boxes[0][7] = new Spot(0, 7, new Rook(true));

    for (int j = 0; j < 8; j++)
        boxes[1][j] = new Spot(1, j, new Pawn(true));

    // Black pieces
    boxes[7][0] = new Spot(7, 0, new Rook(false));
    boxes[7][1] = new Spot(7, 1, new Knight(false));
    boxes[7][2] = new Spot(7, 2, new Bishop(false));
    boxes[7][3] = new Spot(7, 3, new Queen(false));
    boxes[7][4] = new Spot(7, 4, new King(false));
    boxes[7][5] = new Spot(7, 5, new Bishop(false));
    boxes[7][6] = new Spot(7, 6, new Knight(false));
    boxes[7][7] = new Spot(7, 7, new Rook(false));

    for (int j = 0; j < 8; j++)
        boxes[6][j] = new Spot(6, j, new Pawn(false));

    // Empty squares
    for (int i = 2; i <= 5; i++)
        for (int j = 0; j < 8; j++)
            boxes[i][j] = new Spot(i, j, nullptr);
}

/*-----------------------------------------------------
   Player abstraction
-----------------------------------------------------*/
class Player {
public:
    bool whiteSide;
    bool humanPlayer;

    bool isWhiteSide() const { return whiteSide; }
    bool isHumanPlayer() const { return humanPlayer; }
};

class HumanPlayer : public Player {
public:
    HumanPlayer(bool whiteSide) {
        this->whiteSide = whiteSide;
        this->humanPlayer = true;
    }
};

class ComputerPlayer : public Player {
public:
    ComputerPlayer(bool whiteSide) {
        this->whiteSide = whiteSide;
        this->humanPlayer = false;
    }
};

/*-----------------------------------------------------
   Move
-----------------------------------------------------*/
class Move {
    Player* player;
    Spot* start;
    Spot* end;
    Piece* pieceMoved;
    Piece* pieceKilled;
    bool castlingMove;

public:
    Move(Player* player, Spot* start, Spot* end)
        : player(player),
          start(start),
          end(end),
          pieceMoved(start->getPiece()),
          pieceKilled(nullptr),
          castlingMove(false) {}

    Spot* getStart() const { return start; }
    Spot* getEnd() const { return end; }
    Piece* getPieceMoved() const { return pieceMoved; }

    void setPieceKilled(Piece* p) { pieceKilled = p; }
    void setCastlingMove(bool v) { castlingMove = v; }
};

/*-----------------------------------------------------
   GameStatus
-----------------------------------------------------*/
enum class GameStatus {
    ACTIVE,
    BLACK_WIN,
    WHITE_WIN,
    FORFEIT,
    STALEMATE,
    RESIGNATION
};

/*-----------------------------------------------------
   Game
-----------------------------------------------------*/
class Game {
    Player* players[2];
    Board board;
    Player* currentTurn;
    GameStatus status;
    vector<Move*> movesPlayed;

public:
    Game(Player* p1, Player* p2) {
        initialize(p1, p2);
    }

    void initialize(Player* p1, Player* p2) {
        players[0] = p1;
        players[1] = p2;

        board.resetBoard();

        currentTurn = p1->isWhiteSide() ? p1 : p2;

        movesPlayed.clear();
        status = GameStatus::ACTIVE;
    }

    bool isEnd() { return status != GameStatus::ACTIVE; }

    GameStatus getStatus() { return status; }
    void setStatus(GameStatus s) { status = s; }

    bool playerMove(Player* player, int startX,
                    int startY, int endX, int endY) {

        Spot* start = board.getBox(startX, startY);
        Spot* end = board.getBox(endX, endY);

        Move* move = new Move(player, start, end);
        return makeMove(move, player);
    }

    bool makeMove(Move* move, Player* player) {
        Spot* start = move->getStart();
        Spot* end = move->getEnd();
        Piece* sourcePiece = start->getPiece();

        if (!sourcePiece) return false;

        if (player != currentTurn) return false;

        if (sourcePiece->isWhite() != player->isWhiteSide())
            return false;

        if (!sourcePiece->canMove(board, start, end))
            return false;

        // kill?
        Piece* destPiece = end->getPiece();
        if (destPiece) {
            destPiece->setKilled(true);
            move->setPieceKilled(destPiece);
        }

        // castling?
        King* king = dynamic_cast<King*>(sourcePiece);
        if (king && king->isCastlingMove(start, end)) {
            move->setCastlingMove(true);
        }

        movesPlayed.push_back(move);

        // move piece
        end->setPiece(start->getPiece());
        start->setPiece(nullptr);

        if (dynamic_cast<King*>(destPiece)) {
            status = player->isWhiteSide()
                         ? GameStatus::WHITE_WIN
                         : GameStatus::BLACK_WIN;
        }

        currentTurn = (currentTurn == players[0] ? players[1]
                                                 : players[0]);

        return true;
    }
};
