#include <math.h>
#include <vector>

#include <SDL.h>

typedef struct vec2 {
    float x, y;

    vec2 operator+ (vec2 o) {
        return { x + o.x, y + o.y };
    }
    vec2 operator- (vec2 o) {
        return { x - o.x, y - o.y };
    }
    float operator* (vec2 o) {
        return x * o.x + y * o.y;
    }
    vec2 operator* (float v) {
        return { x * v, y * v };
    }
    float magnitude() {
        return sqrt(*this * *this);
    }
} vec2;

class Drawable {
public:
    vec2 pos;
    virtual float sdf(vec2 p);
    Drawable(vec2 pos) : pos{ pos } {}
};

class Circle : Drawable {
public:
    float radius;
    float sdf(vec2 p) override {
        return (pos - p).magnitude();
    }
    Circle(vec2 pos, float radius) : Drawable(pos), radius(radius) {}
};

std::vector<Drawable*> drawables;
void create_drawables() {
    drawables.emplace_back(new Circle({200, 300}, 100));
    drawables.emplace_back(new Circle({293, 239}, 70));
    drawables.emplace_back(new Circle({193, 892}, 150));
    drawables.emplace_back(new Circle({590, 938}, 90));
}
void destroy_drawables() {
    for (auto d : drawables)
        delete d;
}


void main() {
    create_drawables();

    destroy_drawables();
}