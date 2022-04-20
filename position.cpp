//
// Created by Saiyu Wang on 2022/4/19.
//

#include "position.h"
#include "error.h"

void File::AddLine(int offset) {
    mux_.lock();

    int line_number = (int)lines_.size();
    if ((line_number == 0 || lines_.back() < offset) && offset < size) {
        lines_.push_back(offset);
    }

    mux_.unlock();
}

Position File::GetPositionByOffset(int offset) {
    mux_.lock();

    int line_id = 0;
    int line_offset = 0;
    for(const int& current_line_offset: lines_) {
        line_id ++;
        if (current_line_offset <= offset) {
            line_offset = current_line_offset;
        } else {
            break;
        }
    }

    mux_.unlock();

    Position pos;

    pos.filename = name;
    pos.offset = offset;
    pos.line = line_id + 1;
    pos.column = offset - line_offset + 1;

    return pos;
}
