#include <iomanip>
#include <iostream>
#include <vector>

using namespace std;

static vector<vector<vector<int> > > allCompleteMoves();

void printBoard(const vector<vector<int> > &board) {
    string builder = "";
    for (int i = 0; i < board.size(); i++) {
        for (int j = 0; j < board[i].size(); j++) {
            cout << setw(3) << to_string(board[i][j]);
        }
        cout << "\n";
    }
}

int main() {
    auto moves = allCompleteMoves();
    for (int i = 0; i < moves.size(); i++) {
        cout << "Solution #" << i + 1 << ":\n";
        printBoard(moves.at(i));
        cout << "\n";
    }
    flush(cout);
    return 0;
}

struct Point {
    int row;
    int col;
    // Point(int r, int c) : row(r), col(c) {}
};

static vector<vector<vector<int> > > possibleSolutions;
static int board[5][6];

static vector<Point> offsets = {
    {-2, -1},
    {-2, 1},
    {-1, -2},
    {-1, 2},
    {1, -2},
    {1, 2},
    {2, -1},
    {2, 1}
};

inline vector<Point> possibleMoves(const int row, const int col) {
    vector<Point> result = vector<Point>();
    result.reserve(6);
    for (auto &offset: offsets) {
        int potentialRow = offset.row + row;
        int potentialCol = offset.col + col;
        if (potentialRow >= 0 && potentialRow < 5 && potentialCol >= 0 && potentialCol < 6 && board[potentialRow][
                potentialCol] == 0) {
            result.push_back(Point{potentialRow, potentialCol});
            // cout << potentialRow << ", " << potentialCol << endl;
        }
    }
    return result;
}

inline vector<vector<int> > copy() {
    vector<vector<int> > result = vector<vector<int> >(5, vector<int>(6));
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 6; j++) {
            result[i][j] = board[i][j];
        }
    }
    return result;
}

void simulate(const int row, const int col, const int num) {
    board[row][col] = num;
    if (num == 30 && ((row == 1 && col == 2) || (col == 1 && row == 2))) {
        possibleSolutions.emplace_back(copy());
        cout << "Found one!" << endl;
    } else {
        for (auto &possibleMove: possibleMoves(row, col)) {
            simulate(possibleMove.row, possibleMove.col, num + 1);
        }
    }
    board[row][col] = 0;
}

vector<vector<vector<int> > > allCompleteMoves() {
    possibleSolutions = vector<vector<vector<int> > >();
    possibleSolutions.reserve(16);
    for (auto &row: board) {
        for (auto &col: row) {
            col = 0;
        }
    };
    simulate(0, 0, 1);


    return possibleSolutions;
}
