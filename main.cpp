#include <math.h>
#include <vector>
#include <random>
#include <ctime>

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

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
    virtual float sdf(vec2 p) {
        return 0;
    }
    Drawable(vec2 pos) : pos{ pos } {}
};

class Circle : public Drawable {
public:
    float radius;
    float sdf(vec2 p) override {
        return (pos - p).magnitude() - radius;
    }
    Circle(vec2 pos, float radius) : Drawable(pos), radius(radius) {}
};

// important rendering stuff

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

#define DEF_COL_R 0
#define DEF_COL_G 0
#define DEF_COL_B 0
#define DEF_COL_A 255

#define DEF_BG_COL_R 255
#define DEF_BG_COL_G 150
#define DEF_BG_COL_B 31
#define DEF_BG_COL_A 255

SDL_Event event;
SDL_Renderer *renderer;
SDL_Window *window;

void draw_pixel(vec2 pos) {
    SDL_RenderDrawPoint(renderer, pos.x, pos.y);
}

#define RANDOM_CIRCLE_COUNT 5
#define RANDOM_CIRCLE_MIN_SIZE 20
#define RANDOM_CIRCLE_MAX_SIZE 50

std::vector<Drawable*> drawables;
void create_drawables() {
    srand(time(0));

    for (int i = 0; i < RANDOM_CIRCLE_COUNT; i++) {
        int radius = rand() % (RANDOM_CIRCLE_MAX_SIZE - RANDOM_CIRCLE_MIN_SIZE) + RANDOM_CIRCLE_MIN_SIZE;
        vec2 pos = { rand() % (WINDOW_WIDTH - 2 * radius) + radius, rand() % (WINDOW_HEIGHT - 2 * radius) + radius };
        drawables.emplace_back(new Circle{ pos, radius });
    }
}
void destroy_drawables() {
    for (auto d : drawables)
        delete d;
}

void draw() {
    vec2 it;
    for (it.x = 0; it.x <= WINDOW_WIDTH; it.x++) {
        for (it.y = 0; it.y <= WINDOW_HEIGHT; it.y++) {
            float min = 10000;
            for (auto d : drawables) {
                float newDist = d->sdf(it);
                if (newDist < min) {
                    min = newDist;
                    if (min <= 0)
                        break;
                }
            }
            if (min <= 0) {
                draw_pixel(it);
            }
        }
    }
}

int main() {
    // initialize sdl
    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(WINDOW_WIDTH, WINDOW_HEIGHT, 0, &window, &renderer);

    create_drawables();

    // render loop
    while (1) {
        // poll window and quit if needed
        if (SDL_PollEvent(&event) && event.type == SDL_QUIT)
            break;
        
        SDL_SetRenderDrawColor(renderer, DEF_BG_COL_R, DEF_BG_COL_G, DEF_BG_COL_B, DEF_BG_COL_A);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, DEF_COL_R, DEF_COL_G, DEF_COL_B, DEF_COL_A);

        // you may guess 3 times
        draw();

        SDL_RenderPresent(renderer);
    }

    destroy_drawables();

    // tidy up sdl
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_SUCCESS;
}