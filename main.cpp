#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <locale>
#include <optional>
#include <type_traits>
#include <vector>
#define WIDTH 100
#define HEIGHT 50
#define FRAMES 1.0 / 30.0
#define PI 3.141592653589793

struct Vector2 {
    float x, y = 0;

    bool visalbe = true;
    Vector2(float x, float y, bool visalbe) : x(x), y(y), visalbe(visalbe) {};
    Vector2(float x, float y) : x(x), y(y), visalbe(true) {};
};

struct Vector3 {
    float x, y, z;

    Vector3(float x, float y, float z) : x(x), y(y), z(z) {};

    Vector3 operator+(Vector3 vec) {
        return Vector3{x + vec.x, y + vec.y, z + vec.z};
    };

    Vector3 operator+=(Vector3 vec) {
        return Vector3{x + vec.x, y + vec.y, z + vec.z};
    };
};

struct Triangle3D {
    Vector3 a, b, c;

    Triangle3D(Vector3 a, Vector3 b, Vector3 c) : a(a), b(b), c(c) {};
};

struct Triangle2D {
    Vector2 a, b, c;

    Triangle2D(Vector2 a, Vector2 b, Vector2 c) : a(a), b(b), c(c) {};
    std::vector<Vector2> get_as_vector() {
        std::vector<Vector2> out = {a, b, c};
        return out;
    }
};

float dist(Vector2 a, Vector2 b) {
    return std::sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
}
bool is_point_on_screen(Vector2 a) {

    if (a.x > 0 && a.x < WIDTH && a.y > 0 && a.y < HEIGHT) {
        return true;
    }
    return false;
}

void draw_pixel(int x, int y, char a, std::vector<char> *buffer) {

    (*buffer)[y * WIDTH + x] = a;
}

Vector2 project2D(Vector3 p) {
    float FOV = (PI / 2.0);
    float f = 1.0f / std::tan(FOV / 2.0);

    if (p.z <= 0) {

        return Vector2{((p.x / p.z)) * f, ((p.y / p.z)) * f, false};
    };
    return Vector2{((p.x / p.z)) * f, ((p.y / p.z)) * f, true};
}
void drawLine(Vector2 a, Vector2 b, std::vector<char> *display) {

    if (!(a.visalbe && b.visalbe)) {
        return;
    }
    float dx = b.x - a.x;
    float dy = b.y - a.y;

    int steps = (int)std::max(std::abs(dx), std::abs(dy));

    float xInc = dx / steps;
    float yInc = dy / steps;

    float x = a.x;
    float y = a.y;

    for (int i = 0; i < steps; i++) {
        int ix = (int)std::round(x);
        int iy = (int)std::round(y);

        if (is_point_on_screen(Vector2{static_cast<float>(ix),
                                       static_cast<float>(iy), true})) {

            draw_pixel(ix, iy, '.', display);
        }
        if (is_point_on_screen(Vector2{static_cast<float>(ix),
                                       static_cast<float>(iy), true}) &&
            (i == 0 || i == steps)) {
            draw_pixel(ix, iy, 'X', display);
        }
        x += xInc;
        y += yInc;
    }
}

Vector2 drawPoint(Vector3 p) {
    Vector2 new_p = project2D(p);
    Vector2 screen{0, 0, new_p.visalbe};
    screen.x = static_cast<int>(((new_p.x + 1) / 2.0) * WIDTH - 1);
    screen.y = static_cast<int>((1 - ((new_p.y + 1) / 2.0)) * HEIGHT);

    return screen;
}

void connectPolygon(std::vector<Triangle2D> trigs, std::vector<char> *buffer) {

    for (Triangle2D trig : trigs) {
        for (Vector2 vertex : trig.get_as_vector()) {

            if (vertex.visalbe) {
                for (int i{0}; i < trig.get_as_vector().size() - 1; i++) {
                    drawLine(vertex, trig.get_as_vector()[i], buffer);
                }
            }
        }
    }
}

void clear(std::vector<char> *buffer) {

    std::fill((*buffer).begin(), buffer->end(), ' ');
    // floor
    for (int i = (*buffer).size() / 2; i < (*buffer).size(); i++) {
        (*buffer)[i] = '#';
    };

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

std::vector<Vector2> plot3D(std::vector<std::optional<Vector3>> vertexes3D,
                            std::vector<char> *buffer) {

    std::vector<Vector2> screenPoints;

    for (std::optional<Vector3> vertex : vertexes3D) {
        auto tmp = drawPoint(vertex.value());
        screenPoints.push_back(tmp);
    }
    return screenPoints;
}

Vector3 get_input() {

    Vector3 out(0, 0, 0);

    system("stty raw");
    char input = getchar();

    system("stty cooked");
    switch (tolower(input, std::locale())) {
    case 'a':
        out.x += 0.3;
        break;
    case 'd':
        out.x -= 0.3;
        break;
    case 'w':
        out.z -= 0.3;
        break;
    case 's':
        out.z += 0.3;
        break;
    case 'q':
        system("stty cooked");
        exit(1);
    default:
        return Vector3(0, 0, 0);
    }
    return out;
}
void move_vertexes(std::vector<Triangle3D> *triangles, Vector3 offset) {

    for (int i{0}; i < triangles->size(); i++) {
        (*triangles)[i].a = (*triangles)[i].a + offset;
        (*triangles)[i].b = (*triangles)[i].b + offset;
        (*triangles)[i].c = (*triangles)[i].c + offset;
    }
}

Triangle2D project_triangle(Triangle3D triangle) {
    Triangle2D out{
        drawPoint(triangle.a),
        drawPoint(triangle.b),
        drawPoint(triangle.c),
    };
    return out;
}

std::vector<Triangle2D> project_cube(std::vector<Triangle3D> triangles) {

    std::vector<Triangle2D> out;
    for (Triangle3D triangle : triangles) {
        out.push_back(project_triangle(triangle));
    }
    return out;
};

float edge_detection(Vector2 a, Vector2 b, Vector2 p) {
    return (b.x - a.x) * (p.y - a.y) - (b.y - a.y) * (p.x - a.x);
}
bool is_point_in_triangle(Triangle2D t, Vector2 p) {
    float e1 = edge_detection(t.a, t.b, p);
    float e2 = edge_detection(t.b, t.c, p);
    float e3 = edge_detection(t.c, t.a, p);

    bool has_neg = (e1 < 0) || (e2 < 0) || (e3 < 0);
    bool has_pos = (e1 > 0) || (e2 > 0) || (e3 > 0);

    return !(has_neg && has_pos);
}

void rastorize_triangle(Triangle2D trig, std::vector<char> *buffer) {

    if (!trig.a.visalbe || !trig.b.visalbe || !trig.c.visalbe)
        return;
    for (int y{0}; y < HEIGHT; y++) {
        for (int x{0}; x < WIDTH; x++) {
            if (is_point_in_triangle(trig, Vector2{static_cast<float>(x),
                                                   static_cast<float>(y)})) {
                if (is_point_on_screen(Vector2{static_cast<float>(x),
                                               static_cast<float>(y)})) {
                    draw_pixel(x, y, '.', buffer);
                }
            }
        }
    }
};

void rastorize_shape(std::vector<Triangle2D> triangles,
                     std::vector<char> *buffer) {
    for (Triangle2D trig : triangles) {
        rastorize_triangle(trig, buffer);
    }
};

int main() {

    auto last_time = std::chrono::steady_clock::now();
    bool running = true;
    std::vector<char> buffer(WIDTH * HEIGHT, ' ');
    std::vector<Vector2> vertexes2D;
    std::vector<Triangle2D> projectedTrignales;
    std::vector<Triangle3D> cube = {

        // front face
        Triangle3D{Vector3{-1, 1, 2}, Vector3{1, 1, 2}, Vector3{-1, -1, 2}},
        Triangle3D{Vector3{1, -1, 2}, Vector3{1, 1, 2}, Vector3{-1, -1, 2}},
        // back face
        Triangle3D{Vector3{-1, 1, 4}, Vector3{1, 1, 4}, Vector3{-1, -1, 4}},
        Triangle3D{Vector3{1, -1, 4}, Vector3{1, 1, 4}, Vector3{-1, -1, 4}},

        // left face
        Triangle3D{Vector3{-1, 1, 2}, Vector3{-1, 1, 4}, Vector3{-1, -1, 2}},
        Triangle3D{Vector3{-1, -1, 4}, Vector3{-1, 1, 4}, Vector3{-1, -1, 2}},

        // right face
        Triangle3D{Vector3{1, 1, 2}, Vector3{1, 1, 4}, Vector3{1, -1, 2}},
        Triangle3D{Vector3{1, -1, 4}, Vector3{1, 1, 4}, Vector3{1, -1, 2}},

    };

    std::vector<Triangle3D> cube1 = {
        // front face
        Triangle3D{Vector3{3, 1, 2}, Vector3{5, 1, 2}, Vector3{3, -1, 2}},
        Triangle3D{Vector3{5, -1, 2}, Vector3{5, 1, 2}, Vector3{3, -1, 2}},
        // back face
        Triangle3D{Vector3{3, 1, 4}, Vector3{5, 1, 4}, Vector3{3, -1, 4}},
        Triangle3D{Vector3{5, -1, 4}, Vector3{5, 1, 4}, Vector3{3, -1, 4}},

        // left face
        Triangle3D{Vector3{3, 1, 2}, Vector3{3, 1, 4}, Vector3{3, -1, 2}},
        Triangle3D{Vector3{3, -1, 4}, Vector3{3, 1, 4}, Vector3{3, -1, 2}},

        // right face
        Triangle3D{Vector3{5, 1, 2}, Vector3{5, 1, 4}, Vector3{5, -1, 2}},
        Triangle3D{Vector3{5, -1, 4}, Vector3{5, 1, 4}, Vector3{5, -1, 2}},
    };
    Vector3 input(0, 0, 0);

    while (running) {

        auto now = std::chrono::steady_clock::now();
        std::chrono::duration<float> delta = now - last_time;
        float dt = delta.count();
        if (dt < FRAMES) {
            continue;
        }
        // clear screen
        //
        clear(&buffer);

        move_vertexes(&cube, input);
        rastorize_shape(project_cube(cube), &buffer);

        move_vertexes(&cube1, input);
        rastorize_shape(project_cube(cube1), &buffer);

        bufferDraw(buffer);

        last_time = now;
        input = get_input();
    }

    return 1;
}
