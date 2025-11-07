#include <bits/stdc++.h>
using namespace std;

struct CellStyle {
    string fontName;
    int fontSize = 0;
    bool isBold = false;
    bool isItalic = false;
};

struct Cell {
    string text;      // empty → cell is empty
    CellStyle style;
};

class Spreadsheet {
private:
    vector<vector<Cell>> grid;
    int rows;
    int cols;

public:
    Spreadsheet(int initialRows = 5, int initialCols = 5)
        : rows(initialRows), cols(initialCols) 
    {
        grid.assign(rows, vector<Cell>(cols));
    }

    // ---------------------- Add Row ------------------------
    void addRow(int index) {
        if (index < 0 || index > rows) {
            throw invalid_argument("Invalid row index");
        }

        vector<Cell> emptyRow(cols);
        grid.insert(grid.begin() + index, emptyRow);

        rows++;
    }

    // ---------------------- Add Column ----------------------
    void addColumn(int index) {
        if (index < 0 || index > cols) {
            throw invalid_argument("Invalid column index");
        }

        for (auto &row : grid) {
            row.insert(row.begin() + index, Cell());
        }

        cols++;
    }

    // ---------------------- Add Entry -----------------------
    void addEntry(int row, int col,
                  const string &text,
                  const string &fontName,
                  int fontSize,
                  bool isBold,
                  bool isItalic)
    {
        if (row < 0 || row >= rows || col < 0 || col >= cols)
            throw invalid_argument("Invalid cell position");

        // Replace content
        grid[row][col].text = text;
        grid[row][col].style.fontName = fontName;
        grid[row][col].style.fontSize = fontSize;
        grid[row][col].style.isBold = isBold;
        grid[row][col].style.isItalic = isItalic;
    }

    // ---------------------- Get Entry -----------------------
    string getEntry(int row, int col) {
        if (row < 0 || row >= rows || col < 0 || col >= cols)
            throw invalid_argument("Invalid cell position");

        Cell &cell = grid[row][col];

        if (cell.text.empty())
            return ""; // empty cell

        string result = cell.text +
                        "-" + cell.style.fontName +
                        "-" + to_string(cell.style.fontSize);

        // bold/italic flags
        if (cell.style.isBold) result += "-b";
        if (cell.style.isItalic) result += "-i";

        return result;
    }

    // Helpers for debugging
    int rowCount() const { return rows; }
    int colCount() const { return cols; }
};
