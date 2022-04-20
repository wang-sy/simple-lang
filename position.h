#pragma once

#include <string>
#include <mutex>
#include <vector>

using namespace std;

class Position {
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
