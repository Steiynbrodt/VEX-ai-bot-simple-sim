#include <iostream>
#include <vector>
#include <queue>
#include <cmath>
#include <thread>
#include <chrono>
#include <cstdlib>

const int GRID_SIZE = 37;
const int OFFSET = 18;

int startX = 15, startY = 5;
int goalX = 10, goalY = 10;

bool walkable[GRID_SIZE][GRID_SIZE];
bool isPath[GRID_SIZE][GRID_SIZE];

struct Node {
    int x, y;
    int gCost = 9999, hCost = 9999;
    Node* parent = nullptr;

    int fCost() const {
        return gCost + hCost;
    }
};

Node* nodes[GRID_SIZE][GRID_SIZE];

int heuristic(int x1, int y1, int x2, int y2) {
    return 10 * (abs(x1 - x2) + abs(y1 - y2));  // Manhattan distance (scaled)
}

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void simulateGridPrint() {
    const int step = 1;
    clearScreen();

    const int gridDisplayWidth = (OFFSET * 2 + 1) / step;
    const int gridDisplayHeight = (OFFSET * 2 + 1) / step;

    const int consoleWidth = 80;  // adjust depending on your terminal
    const int consoleHeight = 25; // adjust depending on your terminal

    int paddingLeft = (consoleWidth - gridDisplayWidth) / 2;
    int paddingTop = (consoleHeight - gridDisplayHeight) / 2;

    // Top vertical padding
    for (int i = 0; i < paddingTop; ++i)
        std::cout << std::endl;

    for (int y = -OFFSET; y <= OFFSET; y += step) {
        // Left horizontal padding
        for (int i = 0; i < paddingLeft; ++i)
            std::cout << " ";

        for (int x = -OFFSET; x <= OFFSET; x += step) {
            int rx = x + OFFSET;
            int ry = y + OFFSET;

            if (x == 0 && y == 0) {
                std::cout << "X";
            } else if (x == startX && y == startY) {
                std::cout << "R";
            } else if (x == goalX && y == goalY) {
                std::cout << "Z";
            } else if (!walkable[ry][rx]) {
                std::cout << "#";
            } else if (isPath[ry][rx]) {
                std::cout << "*";
            } else {
                std::cout << ".";
            }
        }
        std::cout << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }}
void calculatePath() {
    // Reset
    for (int y = 0; y < GRID_SIZE; y++) {
        for (int x = 0; x < GRID_SIZE; x++) {
            nodes[y][x] = new Node{x - OFFSET, y - OFFSET};
            nodes[y][x]->gCost = 9999;
            nodes[y][x]->hCost = 9999;
            nodes[y][x]->parent = nullptr;
            isPath[y][x] = false;
        }
    }

    Node* start = nodes[startY + OFFSET][startX + OFFSET];
    Node* goal = nodes[goalY + OFFSET][goalX + OFFSET];
    start->gCost = 0;
    start->hCost = heuristic(startX, startY, goalX, goalY);

    std::vector<Node*> openSet = { start };
    bool closedSet[GRID_SIZE][GRID_SIZE] = { false };

    // Directions: 8-way movement (N, NE, E, SE, S, SW, W, NW)
    int dx[8] = { -1, -1,  0, 1, 1,  1,  0, -1 };
    int dy[8] = {  0, -1, -1, -1, 0,  1,  1,  1 };

    while (!openSet.empty()) {
        // Find node with lowest fCost
        Node* current = openSet[0];
        int currentIndex = 0;
        for (int i = 1; i < openSet.size(); i++) {
            if (openSet[i]->fCost() < current->fCost()) {
                current = openSet[i];
                currentIndex = i;
            }
        }

        openSet.erase(openSet.begin() + currentIndex);
        int cx = current->x + OFFSET;
        int cy = current->y + OFFSET;
        closedSet[cy][cx] = true;

        if (current == goal) {
            Node* p = goal;
            while (p != start && p != nullptr) {
                isPath[p->y + OFFSET][p->x + OFFSET] = true;
                p = p->parent;
            }
            return;
        }

        for (int d = 0; d < 8; d++) {
            int nx = current->x + dx[d];
            int ny = current->y + dy[d];

            if (nx < -OFFSET || nx > OFFSET || ny < -OFFSET || ny > OFFSET) continue;
            int rx = nx + OFFSET;
            int ry = ny + OFFSET;
            if (!walkable[ry][rx] || closedSet[ry][rx]) continue;

            // Prevent corner-cutting through walls
            if (dx[d] != 0 && dy[d] != 0) {
                int adj1x = current->x + dx[d];
                int adj1y = current->y;
                int adj2x = current->x;
                int adj2y = current->y + dy[d];

                if (!walkable[adj1y + OFFSET][adj1x + OFFSET] ||
                    !walkable[adj2y + OFFSET][adj2x + OFFSET]) {
                    continue;
                }
            }

            Node* neighbor = nodes[ry][rx];
            int moveCost = (dx[d] == 0 || dy[d] == 0) ? 10 : 14;
            int tentativeG = current->gCost + moveCost;

            if (tentativeG < neighbor->gCost) {
                neighbor->parent = current;
                neighbor->gCost = tentativeG;
                neighbor->hCost = heuristic(nx, ny, goalX, goalY);
                openSet.push_back(neighbor);
            }
        }
    }
}
int main() {
    // Console input
    std::cout << "Enter START X (-18 to 18): ";
    std::cin >> startX;
    std::cout << "Enter START Y (-18 to 18): ";
    std::cin >> startY;

    std::cout << "Enter GOAL X (-18 to 18): ";
    std::cin >> goalX;
    std::cout << "Enter GOAL Y (-18 to 18): ";
    std::cin >> goalY;

    // Clamp to grid
    startX = std::max(-OFFSET, std::min(OFFSET, startX));
    startY = std::max(-OFFSET, std::min(OFFSET, startY));
    goalX  = std::max(-OFFSET, std::min(OFFSET, goalX));
    goalY  = std::max(-OFFSET, std::min(OFFSET, goalY));

    // Initialize grid
    for (int y = 0; y < GRID_SIZE; ++y) {
        for (int x = 0; x < GRID_SIZE; ++x) {
            walkable[y][x] = true;
            isPath[y][x] = false;
        }
    }

    // Add obstacles
    int obstacles[][2] = {
        {-6, -6}, {6, 6}, {-6, 6}, {6, -6}
    };
    int obstacleCount = sizeof(obstacles) / sizeof(obstacles[0]);
    for (int i = 0; i < obstacleCount; i++) {
        int ox = obstacles[i][0];
        int oy = obstacles[i][1];
        if (ox >= -OFFSET && ox <= OFFSET && oy >= -OFFSET && oy <= OFFSET) {
            walkable[oy + OFFSET][ox + OFFSET] = false;
        }
    }

    simulateGridPrint();
    std::this_thread::sleep_for(std::chrono::seconds(1));

    calculatePath();
    simulateGridPrint();

    std::cout << "\nPfad gefunden. Drücke Enter zum Beenden.\n";
    std::cin.get(); // Wait for final Enter
    std::cin.get(); // One more to exit cleanly
    return 0;
}
