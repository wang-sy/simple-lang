#pragma once

#include <string>
#include <mutex>
#include <vector>

using namespace std;

namespace token{
class Position {
public:
    const string ToString() const {return "(" + to_string(line) + ", " + to_string(column) + ")";}
    bool operator<(const Position& other) const {
        if (line == other.line) {
            return column < other.column;
        }

        return  line < other.line;
    }
public:
    string filename;
    int offset; // offset, starting at 0
    int line; // line number, starting at 1
    int column; // column number, starting at 1
};

class File {
public:
    /**
     * @brief AddLine adds the line offset for a new line.
     * The line offset must be larger than the offset for the previous line
     * and smaller than the file size; otherwise the line offset is ignored.
     */
    void AddLine(int offset);

    /**
     * @brief Position returns the Position value for the given file Position p.
     */
    Position GetPositionByOffset(int offset);

    string name{};
    int size{};
private:
    vector<int> lines_{};
    mutex mux_{};
};

const Position npos = Position{
    .filename="",
    .offset=-1,
    .line=-1,
    .column=-1
};

}// namespace token