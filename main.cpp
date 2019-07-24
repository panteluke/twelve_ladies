#include <iostream>
#include <vector>
#include <memory>
#include <algorithm>

using namespace std;

enum class BodyPart {
    Head,
    Tail
};

enum class Color {
    Red,
    Green,
    Yellow,
    Blue
};

enum class RestrictionPosition {
    Up,
    Right,
    Left,
    Down
};

struct Restriction {
    Color color;
    BodyPart part;
    RestrictionPosition position;
};

class Lady {
    public:
        Color color;
        BodyPart part;
        Lady(Color c, BodyPart p) : color(c), part(p) {}
};

class Tile {
    public:
        int id;
        Lady up;
        Lady right;
        Lady down;
        Lady left;
        void rotate();
};

void Tile::rotate() 
{
    Lady tmp = up;
    up = left;
    left = down;
    down = right;
    right = tmp;
}

typedef shared_ptr<Tile> Board[5][5] ;

void initBoard(Board board) 
{
    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 5; ++j) {
            board[i][j] = nullptr;
        }
    }
}

pair<int, int> getNextPosToFill(Board board)
{
    for (int i = 1; i < 4; ++i) {
        for (int j = 1; j < 4; ++j) {
            if (board[i][j] == nullptr) {
                if ((board[i-1][j] != nullptr) || (board[i+1][j] != nullptr) || (board[i][j-1] != nullptr) || (board[i][j + 1] != nullptr)) {
                    return make_pair(i,j);
                }
            }
        }
    }
}

bool tileMatchesAllRestrictions(Tile* tile, const std::vector<Restriction>& restrictions) {
    bool result = true;

    for (auto &restriction : restrictions) {
        switch (restriction.position) {
            case RestrictionPosition::Up: result = tile->up.color == restriction.color && tile->up.part == restriction.part; break;
            case RestrictionPosition::Left: result = tile->left.color == restriction.color && tile->left.part == restriction.part; break;
            case RestrictionPosition::Down: result = tile->down.color == restriction.color && tile->down.part == restriction.part; break;
            case RestrictionPosition::Right: result = tile->right.color == restriction.color && tile->right.part == restriction.part; break;
        }
        if (!result) return false;
    }
    return result;
}

bool tileWithAnyRotationsCanMatchAllRestrictions(shared_ptr<Tile> tile, const std::vector<Restriction>& restrictions, std::vector<int>& numberOfRotationsForMatch)
{
    bool result = false;
    Tile tmpTile(*tile);

    int i = 0;
    while (i < 4) {
        if (tileMatchesAllRestrictions(&tmpTile, restrictions)) {
            result = true;
            numberOfRotationsForMatch.push_back(i);
        }
        ++i;
        tmpTile.rotate();
    }

    return result;
}

void createRestrictionDueToNeighbor(Board board, int row, int col, RestrictionPosition position, vector<Restriction>& restrictions)
{
    if (board[row][col]) {
        Lady* ladyToUse = NULL;
        switch (position) {
            case RestrictionPosition::Up: ladyToUse = &board[row][col]->down; break;
            case RestrictionPosition::Down: ladyToUse = &board[row][col]->up; break;
            case RestrictionPosition::Right: ladyToUse = &board[row][col]->left; break;
            case RestrictionPosition::Left: ladyToUse = &board[row][col]->right; break;
        }

        Restriction restriction;
        restriction.color = ladyToUse->color;
        restriction.part = ladyToUse->part == BodyPart::Head ? BodyPart::Tail : BodyPart::Head;
        restriction.position = position;
        restrictions.push_back(restriction);
    }
}

vector<Restriction> createRestrictionsForPos(Board board, pair<int, int> pos)
{
    vector<Restriction> restrictions;

    createRestrictionDueToNeighbor(board, pos.first - 1, pos.second, RestrictionPosition::Up, restrictions);
    createRestrictionDueToNeighbor(board, pos.first + 1, pos.second, RestrictionPosition::Down, restrictions);
    createRestrictionDueToNeighbor(board, pos.first, pos.second + 1, RestrictionPosition::Right, restrictions);
    createRestrictionDueToNeighbor(board, pos.first, pos.second - 1, RestrictionPosition::Left, restrictions);

    return restrictions;
}

vector<pair<shared_ptr<Tile>, int>> findCandidates(Board board, pair<int, int> pos, vector<shared_ptr<Tile>> availableTiles) {
    vector<pair<shared_ptr<Tile>, int>> result;

    vector<Restriction> restrictions = createRestrictionsForPos(board, pos);

    for (auto& tile : availableTiles) {
        std::vector<int> numberOfRotationsForMatch;
        if (tileWithAnyRotationsCanMatchAllRestrictions(tile, restrictions, numberOfRotationsForMatch)) {
            for (auto num : numberOfRotationsForMatch) {
                result.push_back(std::make_pair(tile, num));
            }
        }
    }

    return result;
}

void printSolution(shared_ptr<Tile> board[5][5])
{
    for (int i = 1; i < 4; ++i) {
        for (int j = 1; j < 4; ++j) {
            cout << board[i][j]->id;
        }
        cout << endl;
    }
    cout << endl;
}

void findSolution(Board board, vector<shared_ptr<Tile>> availableTiles)
{
    if (availableTiles.empty()) {
        printSolution(board); 
    } else {
        pair<int, int> pos = getNextPosToFill(board);
        
        auto candidates = findCandidates(board, pos, availableTiles);

        for (auto candidate : candidates) {
            auto tile = candidate.first;

            board[pos.first][pos.second] = tile;
            for (int i = 0; i < candidate.second; ++i) {
                tile->rotate();
            }

            vector<shared_ptr<Tile>> remainingTiles;
            copy_if(availableTiles.begin(), availableTiles.end(), back_inserter(remainingTiles), [tile](const shared_ptr<Tile>& toAdd){ return toAdd->id != tile->id;} );

            findSolution(board, remainingTiles);

            board[pos.first][pos.second] = nullptr;
            for (int i = 0; i < (4 - candidate.second) % 4; ++i) {
                tile->rotate();
            }
        }
    }
}

vector<shared_ptr<Tile>> createTiles()
{
    vector<shared_ptr<Tile>> result;
    result.push_back(make_shared<Tile>(Tile{1, {Color::Yellow, BodyPart::Tail}, {Color::Green, BodyPart::Tail}, {Color::Green, BodyPart::Head}, {Color::Blue, BodyPart::Head}}));
    result.push_back(make_shared<Tile>(Tile{2, {Color::Red, BodyPart::Head}, {Color::Red, BodyPart::Head}, {Color::Yellow, BodyPart::Tail}, {Color::Green, BodyPart::Head}}));
    result.push_back(make_shared<Tile>(Tile{3, {Color::Red, BodyPart::Tail}, {Color::Green, BodyPart::Tail}, {Color::Blue, BodyPart::Head}, {Color::Blue, BodyPart::Head}}));
    result.push_back(make_shared<Tile>(Tile{4, {Color::Red, BodyPart::Head}, {Color::Green, BodyPart::Tail}, {Color::Red, BodyPart::Head}, {Color::Yellow, BodyPart::Tail}}));
    result.push_back(make_shared<Tile>(Tile{5, {Color::Yellow, BodyPart::Head}, {Color::Blue, BodyPart::Tail}, {Color::Blue, BodyPart::Head}, {Color::Red, BodyPart::Head}}));
    result.push_back(make_shared<Tile>(Tile{6, {Color::Yellow, BodyPart::Head}, {Color::Red, BodyPart::Head}, {Color::Blue, BodyPart::Head}, {Color::Red, BodyPart::Tail}}));
    result.push_back(make_shared<Tile>(Tile{7, {Color::Green, BodyPart::Tail}, {Color::Green, BodyPart::Tail}, {Color::Yellow, BodyPart::Head}, {Color::Red, BodyPart::Head}}));
    result.push_back(make_shared<Tile>(Tile{8, {Color::Red, BodyPart::Tail}, {Color::Yellow, BodyPart::Tail}, {Color::Red, BodyPart::Tail}, {Color::Blue, BodyPart::Tail}}));
    result.push_back(make_shared<Tile>(Tile{9, {Color::Green, BodyPart::Head}, {Color::Blue, BodyPart::Head}, {Color::Yellow, BodyPart::Head}, {Color::Red, BodyPart::Tail}}));

    return result;
}

int main() {
    Board board;

    // 3 8 6 
    // 5 4 2 
    // 1 9 7 

    // 1 4 6 
    // 5 8 7 
    // 3 2 9 

    auto tiles = createTiles();
    for (auto tile : tiles) {
        initBoard(board);
        
        board[2][2] = tile;

        vector<shared_ptr<Tile>> available;

        copy_if(tiles.begin(), tiles.end(), back_inserter(available), [tile](const shared_ptr<Tile>& toAdd){ return toAdd->id != tile->id; });

        findSolution(board, available);
    }

    return 0;
}