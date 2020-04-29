#include "Floor.h"

Floor::Floor(int floor_num, int rows, int cols)
{
    setFloorNum(floor_num);
    setFloorRow(rows);
    setFloorCol(cols);
    initializeFloor();
}

Floor::Floor(const Floor &f)
{
    setFloorNum(f.floor_num);
    setFloorRow(f.rows);
    setFloorCol(f.cols);
    initializeFloor(f.getUnavailableSpots());
}

void Floor::initializeFloor()
{
    // Initialize all spots
    this->floor_map = new Spot **[this->rows];
    for (int i = 0; i < rows; ++i)
    {
        this->floor_map[i] = new Spot *[this->cols];
        for (int j = 0; j < cols; ++j)
        {
            this->floor_map[i][j] = new Spot(i, j, true, this->floor_num);
        }
    }
}

void Floor::initializeFloor(const set<pair<int, int>> &unavailable_spots)
{
    // Initialize all spots
    this->floor_map = new Spot **[this->rows];
    for (int i = 0; i < rows; ++i)
    {
        this->floor_map[i] = new Spot *[this->cols];
        for (int j = 0; j < cols; ++j)
        {
            this->floor_map[i][j] = new Spot(i, j, true, this->floor_num);
        }
    }
    // Mark specific spots as unavailable
    for (auto spot : unavailable_spots)
    {
        this->floor_map[spot.first][spot.second]->setAvailable(false);
    }
}

Floor::~Floor()
{
    for (int i = 0; i < rows; ++i)
    {
        for(int j = 0; j < cols; j++){
            delete this->floor_map[i][j];
        }
        delete[] this->floor_map[i];
    }
    delete this->floor_map;
}

/* Prints the floor map:
	0- spot is un available, 
	1- spot is available but empty,
	Container ID- spot is available and full.
*/
ostream &operator<<(ostream &out, const Floor &f)
{
    out << "Floor " << f.floor_num << " Map is:" << endl;
    for (int i = 0; i < f.rows; i++)
    {
        for (int j = 0; j < f.cols; j++)
        {
            if (!f.floor_map[i][j]->getAvailable())
            { // If Spot is unavailable
                out << "___________0_________";
            }
            else
            {
                //else, Spot is available
                if (f.floor_map[i][j]->getContainer() == nullptr)
                    out << "___________1_________";
                else
                    out << "___" << f.floor_map[i][j]->getContainer()->getID() <<"->"
                        << f.floor_map[i][j]->getContainer()->getDestPort() << "__";
            }
        }
        out << "\n";
    }
    return out << "\n";
}

set<pair<int, int>> Floor::getUnavailableSpots() const
{
    set<pair<int, int>> unavail_spots = {};
    for (int i = 0; i < rows; ++i)
    {
        for (int j = 0; j < cols; ++j)
        {
            if (!this->floor_map[i][j]->getAvailable()) //found an unavailable spot
                unavail_spots.emplace(i, j);
        }
    }
    return unavail_spots;
}