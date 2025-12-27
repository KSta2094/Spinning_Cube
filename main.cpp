#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <optional>
#include <vector>
#define WIDTH 50
#define HEIGHT 50
#define FRAMES 1.0 / 60.0
#define PI 3.141592653589793

typedef struct {
    float x, y;
} Vector2;

typedef struct {
    float x, y, z;
} Vector3;

std::optional<Vector2> project2D(Vector3 p) {
    if (p.z <= 0) {
        return std::nullopt;
    };
    float FOV = (PI / 2.0);
    float f = 1.0f / std::tan(FOV / 2.0);
    return Vector2{((p.x / p.z)) * f, ((p.y / p.z)) * f};
}
float dist(Vector2 a, Vector2 b) {
    return std::sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
}
void drawLine(Vector2 a, Vector2 b, std::vector<char> *display) {
    float dx = b.x - a.x;
    float dy = b.y - a.y;

    int steps = (int)std::max(std::abs(dx), std::abs(dy));

    float xInc = dx / steps;
    float yInc = dy / steps;

    float x = a.x;
    float y = a.y;

    for (int i = 1; i < steps; i++) {
        int ix = (int)std::round(x);
        int iy = (int)std::round(y);

        if (ix >= 0 && ix < WIDTH && iy >= 0 && iy < HEIGHT &&
            (*display)[iy * WIDTH + ix] != 'X')
            (*display)[iy * WIDTH + ix] = '.';

        x += xInc;
        y += yInc;
    }
}

std::optional<Vector2> drawPoint(Vector3 p, std::vector<char> *buffer) {
    std::optional<Vector2> new_p = project2D(p);
    if (!new_p) {
        return std::nullopt;
    };
    Vector2 screen;
    screen.x = static_cast<int>(((new_p->x + 1) / 2.0) * WIDTH - 1);
    screen.y = static_cast<int>((1 - ((new_p->y + 1) / 2.0)) * HEIGHT);
    if (screen.x > WIDTH) {
        return std::nullopt;
    };
    (*buffer)[screen.y * WIDTH + screen.x] = 'X';
    return screen;
}

void connectPolygon(std::vector<std::optional<Vector2>> vertexes,
                    std::vector<char> *buffer) {
    for (std::optional<Vector2> vertex : vertexes) {
        if (vertex) {
            for (int i{0}; i < vertexes.size() - 1; i++) {
                drawLine(vertex.value(), vertexes[i].value(), buffer);
            }
        }
    }
}

void clear(std::vector<char> *buffer) {

    std::fill((*buffer).begin(), buffer->end(), ' ');
    printf("\033[H\033[J");
}

void bufferDraw(std::vector<char> buffer) {
    for (int i{0}; i < WIDTH * HEIGHT; i++) {

        printf("%c", buffer[i]);
        if (i % WIDTH == 0) {
            printf("\n");
        }
    }
}

void rotatex(std::vector<std::optional<Vector3>> *vertexes, float angle) {

    float c_z = 3;
    float c_y = 0;

    for (int i{0}; i < (*vertexes).size(); i++) {

        Vector3 tmp{0};

        tmp.y = ((*vertexes)[i]->y - c_y) * cos(angle) -
                ((*vertexes)[i]->z - c_z) * sin(angle) + c_y;

        tmp.z = ((*vertexes)[i]->y - c_y) * sin(angle) +
                ((*vertexes)[i]->z - c_z) * cos(angle) + c_z;
        (*vertexes)[i]->y = tmp.y;
        (*vertexes)[i]->z = tmp.z;
    }
}

void rotatey(std::vector<std::optional<Vector3>> *vertexes, float angle) {

    float c_x = 0;
    float c_z = 3;

    for (int i{0}; i < (*vertexes).size(); i++) {
        Vector3 tmp{0};
        // x_rot = c_x + (x - c_x) cos(θ) - (y - c_z) sin(θ);
        // z_rot = c_z - (x - c_x) sin(θ) + (z - c_z) cos(θ);

        tmp.x = c_x + ((*vertexes)[i]->x - c_x) * cos(angle) +
                ((*vertexes)[i]->z - c_z) * sin(angle);

        tmp.z = c_z - ((*vertexes)[i]->x - c_x) * sin(angle) +
                ((*vertexes)[i]->z - c_z) * cos(angle);
        (*vertexes)[i]->x = tmp.x;
        (*vertexes)[i]->z = tmp.z;
    }
}

std::vector<std::optional<Vector2>>
plot3D(std::vector<std::optional<Vector3>> vertexes3D,
       std::vector<char> *buffer) {

    std::vector<std::optional<Vector2>> screenPoints;

    for (std::optional<Vector3> vertex : vertexes3D) {
        screenPoints.push_back(drawPoint(vertex.value(), buffer));
    }
    return screenPoints;
}

int main() {
    auto last_time = std::chrono::steady_clock::now();
    bool running = true;
    std::vector<char> buffer(WIDTH * HEIGHT, ' ');
    std::vector<std::optional<Vector2>> vertexes2D;

    std::vector<std::optional<Vector3>> vertexes3D = {

        // front face
        Vector3{-1, 1, 2},
        Vector3{1, 1, 2},
        Vector3{-1, -1, 2},
        Vector3{1, -1, 2},

        // back face
        Vector3{-1, 1, 4},
        Vector3{1, 1, 4},
        Vector3{-1, -1, 4},
        Vector3{1, -1, 4},

    };

    while (running) {

        auto now = std::chrono::steady_clock::now();
        std::chrono::duration<float> delta = now - last_time;
        float dt = delta.count();
        if (dt < FRAMES) {
            continue;
        }
        // clear screen
        clear(&buffer);

        rotatey(&vertexes3D, 0.02);
        rotatex(&vertexes3D, 0.02);
        vertexes2D = plot3D(vertexes3D, &buffer);
        connectPolygon(vertexes2D, &buffer);

        bufferDraw(buffer);

        last_time = now;
    }

    return 1;
}
