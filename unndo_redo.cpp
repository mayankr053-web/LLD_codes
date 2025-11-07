#include <bits/stdc++.h>
using namespace std;

// Forward declarations
class TextEditor;

// -------- Command base --------
class ICommand {
public:
    virtual ~ICommand() = default;
    virtual void apply() = 0;     // do/redo
    virtual void unapply() = 0;   // undo
};

// ----------------------
// TextEditor
// ----------------------
class TextEditor {
    friend class AddTextCommand;
    friend class DeleteTextCommand;

public:

    // 1) addText
    void addText(int row, int column, const string& text) {
        ICommand* cmd = new AddTextCommand(*this, row, column, text);
        cmd->apply();
        undoStack.push(cmd);
        clearRedo();
    }

    // 2) deleteText
    void deleteText(int row, int startColumn, int length) {
        ICommand* cmd = new DeleteTextCommand(*this, row, startColumn, length);
        cmd->apply();
        undoStack.push(cmd);
        clearRedo();
    }

    // 3) undo
    void undo() {
        if (undoStack.empty()) return;
        ICommand* cmd = undoStack.top(); undoStack.pop();
        cmd->unapply();
        redoStack.push(cmd);
    }

    // 4) redo
    void redo() {
        if (redoStack.empty()) return;
        ICommand* cmd = redoStack.top(); redoStack.pop();
        cmd->apply();
        undoStack.push(cmd);
    }

    // 5) readLine
    string readLine(int row) const {
        return rows[row];
    }

    int rowCount() const { return rows.size(); }

    // Destructor: clean up all remaining commands in stacks
    ~TextEditor() {
        while (!undoStack.empty()) { delete undoStack.top(); undoStack.pop(); }
        while (!redoStack.empty()) { delete redoStack.top(); redoStack.pop(); }
    }

private:
    vector<string> rows;

    stack<ICommand*> undoStack;
    stack<ICommand*> redoStack;

    void clearRedo() {
        while (!redoStack.empty()) {
            delete redoStack.top();
            redoStack.pop();
        }
    }

    // -------- AddTextCommand --------
    class AddTextCommand : public ICommand {
    public:
        AddTextCommand(TextEditor& ed, int r, int c, const string& t)
            : editor(ed), row(r), col(c), text(t) {}

        void apply() override {
            createdRow = false;
            if (row == (int)editor.rows.size()) {
                editor.rows.emplace_back("");  // create new row
                createdRow = true;
            }
            editor.rows[row].insert(col, text);
        }

        void unapply() override {
            editor.rows[row].erase(col, text.size());
            if (createdRow &&
                row == (int)editor.rows.size() - 1 &&
                editor.rows[row].empty()) 
            {
                editor.rows.pop_back();
            }
        }

    private:
        TextEditor& editor;
        int row;
        int col;
        string text;
        bool createdRow = false;
    };

    // -------- DeleteTextCommand --------
    class DeleteTextCommand : public ICommand {
    public:
        DeleteTextCommand(TextEditor& ed, int r, int c, int len)
            : editor(ed), row(r), col(c), length(len) {}

        void apply() override {
            removed = editor.rows[row].substr(col, length);
            editor.rows[row].erase(col, length);
        }

        void unapply() override {
            editor.rows[row].insert(col, removed);
        }

    private:
        TextEditor& editor;
        int row;
        int col;
        int length;
        string removed;
    };
};
