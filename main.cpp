#include <math.h>
#include <stdio.h>
#include <ctime>
#include <vector>
#include <random>
#include <array>
#include <algorithm>
#include <stdexcept>

#define PI 3.14159265
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

float clip(float n, float lower, float upper) {
    return std::max(lower, std::min(n, upper));
}

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#include <SDL2_gfxPrimitives.h>

typedef struct vec2 {
    float x, y;

    vec2() : x{0}, y{0} {}
    vec2(float x, float y) : x{x}, y{y} {}
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
    vec2 operator/ (float v) {
        return { x / v, y / v };
    }
    float magnitude() {
        return sqrt(*this * *this);
    }
    float sqr_mag() {
        return *this * *this;
    }
    vec2 normalized() {
        if (*this * *this == 1)
            return *this;
        else
            return *this / magnitude();
    }
    void normalize() {
        if (*this * *this == 1)
            return;
        *this = *this / magnitude();
    }
    vec2 abs() {
        return { std::abs(x), std::abs(y) };
    }
    float max() {
        return std::max(x, y);
    }
    float min() {
        return std::min(x, y);
    }
    static vec2 max(vec2 a, vec2 b) {
        return { std::max(a.x, b.x), std::max(a.y, b.y) };
    }
} vec2;

class Drawable {
public:
    vec2 pos;
    virtual float sdf(vec2 p) {
        return 0;
    }
    Drawable(vec2 pos) : pos{ pos } {}
    ~Drawable() {}
};

class Circle : public Drawable {
public:
    float radius;
    float sdf(vec2 p) override {
        return (pos - p).magnitude() - radius;
    }
    Circle(vec2 pos, float radius) : Drawable{ pos }, radius{ radius } {}
};

class Rectangle : public Drawable {
public:
    vec2 size;
    float sdf(vec2 p) override {
        p = pos - p;
        vec2 d = p.abs() - size / 2;
        return vec2::max(d, { 0,0 }).magnitude() + std::min(std::max(d.x,d.y),0.f);
    }
    Rectangle(vec2 pos, vec2 size) : Drawable{ pos }, size{ size } {}
};

class Light : public Drawable {
public:
    float brightness;
    float sdf(vec2 p) override {
        return (pos - p).magnitude();
    }
    Light(vec2 pos, float brightness) : Drawable{ pos }, brightness{ brightness } {}
};


/* -------------------------
 *      Rendering Stuff
 * -------------------------
*/

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
void draw_circle(int32_t centreX, int32_t centreY, int32_t radius)
{
   const int32_t diameter = (radius * 2);

   int32_t x = (radius - 1);
   int32_t y = 0;
   int32_t tx = 1;
   int32_t ty = 1;
   int32_t error = (tx - diameter);

   while (x >= y)
   {
      //  Each of the following renders an octant of the circle
      SDL_RenderDrawPoint(renderer, centreX + x, centreY - y);
      SDL_RenderDrawPoint(renderer, centreX + x, centreY + y);
      SDL_RenderDrawPoint(renderer, centreX - x, centreY - y);
      SDL_RenderDrawPoint(renderer, centreX - x, centreY + y);
      SDL_RenderDrawPoint(renderer, centreX + y, centreY - x);
      SDL_RenderDrawPoint(renderer, centreX + y, centreY + x);
      SDL_RenderDrawPoint(renderer, centreX - y, centreY - x);
      SDL_RenderDrawPoint(renderer, centreX - y, centreY + x);

      if (error <= 0)
      {
         ++y;
         error += ty;
         ty += 2;
      }

      if (error > 0)
      {
         --x;
         tx += 2;
         error += (tx - diameter);
      }
   }
}

/* -------------------------
 *       Drawable Stuff
 * -------------------------
*/

#define RANDOM_CIRCLE_COUNT 50
#define RANDOM_CIRCLE_MIN_SIZE 10
#define RANDOM_CIRCLE_MAX_SIZE 30

std::vector<Drawable*> drawables;
void create_drawables() {
    srand(time(0));

    for (int i = 0; i < RANDOM_CIRCLE_COUNT; i++) {
        int radius = rand() % (RANDOM_CIRCLE_MAX_SIZE - RANDOM_CIRCLE_MIN_SIZE) + RANDOM_CIRCLE_MIN_SIZE;
        vec2 pos = { rand() % (WINDOW_WIDTH - 2 * radius) + radius, rand() % (WINDOW_HEIGHT - 2 * radius) + radius };
        drawables.emplace_back(new Circle{ pos, radius });
    }
    
    drawables.emplace_back(new Rectangle{ {WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2}, {200, 100} });
}
void destroy_drawables() {
    for (auto d : drawables)
        delete d;
}

float get_min_dist(vec2 pos, Drawable **drawable) {
    float min { 10000.f };
    for (auto d : drawables) {
        float newDist { d->sdf(pos) };
        if (newDist < min) {
            min = newDist;
            *drawable = d;
            if (min <= 0) {
                break;
            }
        }
    }
    return min;
}

float get_min_dist(vec2 pos) {
    return get_min_dist(pos, nullptr);
}

/* -------------------------
 *        Light Stuff
 * -------------------------
*/

#define LIGHT_RAY_MAX_DEPTH 50

std::vector<Light*> lights;
void create_lights() {
    lights.emplace_back(new Light({ WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 }, 100.f));
}
#define LIGHT_DIR_COUNT 3600
vec2 light_directions[LIGHT_DIR_COUNT];

typedef struct RayHitInfo {
    vec2 pos;
    Drawable* drawable;
    float distance;
    bool hit;
} RayHitInfo;

bool march_ray(vec2 pos, vec2 delta, RayHitInfo* hit, float maxDepth = 100.f, float threshold = 0.01f, uint16_t maxSteps = 50) {
    float min;
    int depth { 0 };
    delta.normalize();
    hit->hit = false;
    do {
        min = get_min_dist(pos, &hit->drawable);
        if (min <= threshold) {
            hit->hit = true;
            break;
        }
        pos = pos + delta * min;
        hit->distance += min;
        depth++;
    } while (depth < maxSteps && clip(pos.x, 0, WINDOW_WIDTH) == pos.x && clip(pos.y, 0, WINDOW_HEIGHT) == pos.y && min >= threshold);
    hit->pos = pos;
    return hit->hit;
}

float interpolate(float a, float b, float t) {
    return a * (1 - t) + b * t;
}
float four_point_ip(float a, float b, float c, float d, vec2 delta, vec2 pointDelta) {
    return interpolate(
        interpolate(a, b, delta.x / pointDelta.x),
        interpolate(d, c, delta.x / pointDelta.x),
        delta.y / pointDelta.y
    );
}
vec2 abs_point_around(vec2 pos, int index) {
    switch(index) {
        case 0:
            return vec2( floorf(pos.x), floorf(pos.y) );
        case 1:
            return vec2( ceilf (pos.x), floorf(pos.y) );
        case 2:
            return vec2( ceilf (pos.x), ceilf (pos.y) );
        case 3:
            return vec2( floorf(pos.x), ceilf (pos.y) );
        default:
            return {};
    }
}

#define PM_CACHE_PRECISION 1
#define PM_CACHE_WIDTH PM_CACHE_PRECISION * WINDOW_WIDTH
#define PM_CACHE_HEIGHT PM_CACHE_PRECISION * WINDOW_HEIGHT
#define PM_CACHE_SIZE PM_CACHE_HEIGHT * PM_CACHE_WIDTH

uint64_t get_pm_index(vec2 pos) {
    return static_cast<uint64_t>(pos.x + pos.y * PM_CACHE_WIDTH);
}

float pointmarchingCache[PM_CACHE_SIZE];
Drawable *pointmarchingDrawableCache[PM_CACHE_SIZE];
float pm_cache(vec2 pos) {
    pos = pos * PM_CACHE_PRECISION;
    if (clip(pos.x, 0, PM_CACHE_WIDTH - 1) != pos.x || clip(pos.y, 0, PM_CACHE_HEIGHT - 1) != pos.y) {
        printf("\ncache out of bounds: %f %f\n", pos.x, pos.y);
        fflush(stdout);
        throw std::runtime_error("cache out of bounds");
    }

    // return pointmarchingCache[static_cast<int64_t>(pos.x + pos.y * PM_CACHE_WIDTH)];
    float a = pointmarchingCache[get_pm_index( { floor(pos.x), floor(pos.y)} )];
    float b = pointmarchingCache[get_pm_index( { ceil (pos.x), floor(pos.y)} )];
    float c = pointmarchingCache[get_pm_index( { ceil (pos.x), ceil (pos.y)} )];
    float d = pointmarchingCache[get_pm_index( { floor(pos.x), ceil (pos.y)} )];
    vec2 origin = { floor(pos.x), floor(pos.y) };
    vec2 delta = pos - origin;
    float pointDelta = { 1.f / PM_CACHE_PRECISION };
    return four_point_ip(a, b, c, d, delta, { pointDelta, pointDelta });
}
Drawable* pm_d_cache(vec2 pos) {
    pos = pos * PM_CACHE_PRECISION;

    // std::array<float, 4> all;
    // int nearest = 0;
    // for (int i = 0; i < 4; i++) {
    //     all[i] = (pos - abs_point_around(pos, i)).sqr_mag();
    //     for (int j = 0; j < i; j++) {
    //         if (all[i] < all[j]) {
    //             nearest = i;
    //         }
    //     }
    // }

    return pointmarchingDrawableCache[get_pm_index(abs_point_around(pos, 0))];
}
float& direct_pm_cache(vec2 pos) {
    return pointmarchingCache[get_pm_index(pos)];
}
Drawable** direct_pm_d_cache(vec2 pos) {
    return pointmarchingDrawableCache + get_pm_index(pos);
}
void precalc_pm_cache() {
    vec2 it;
    Drawable* d;
    for (it.x = 0; it.x < PM_CACHE_WIDTH; it.x++) {
        fflush(stdout);
        for (it.y = 0; it.y < PM_CACHE_HEIGHT; it.y++) {
            direct_pm_cache(it) = get_min_dist(it / PM_CACHE_PRECISION, &d);
            *direct_pm_d_cache(it) = d;
        }
        printf("\rto a percentage of %.2f done", it.x / static_cast<float>(PM_CACHE_WIDTH));
    }
    printf("\n");
}

bool march_ray_cache(vec2 pos, vec2 delta, RayHitInfo* hit, float maxDepth = 100.f, float threshold = 0.01f, uint16_t maxSteps = 50) {
    float min;
    int depth { 0 };
    delta.normalize();
    hit->hit = false;
    do {
        min = pm_cache(pos);
        if (min <= 1.5f / PM_CACHE_PRECISION) {
            min = pm_d_cache(pos)->sdf(pos);
            // min = get_min_dist(pos);
        }
        if (min <= threshold) {
            hit->hit = true;
            break;
        }
        pos = pos + delta * min;
        hit->distance += min;
        depth++;
    } while (depth < maxSteps && clip(pos.x, 0, WINDOW_WIDTH - 1) == pos.x && clip(pos.y, 0, WINDOW_HEIGHT - 1) == pos.y && min >= threshold);
    hit->pos = pos;
    return hit->hit;
}

vec2 march_ray_light(vec2 pos, vec2 delta, float threshold = 0.01f) {
    float min;
    int depth { 0 };
    do {
        min = get_min_dist(pos);
        if (min <= threshold) {
            break;
        }
        pos = pos + delta.normalized() * min;
        depth++;
    } while (depth < LIGHT_RAY_MAX_DEPTH && clip(pos.x, 0, WINDOW_WIDTH) == pos.x && clip(pos.y, 0, WINDOW_HEIGHT) == pos.y && min >= threshold);
    //SDL_RenderDrawLine(renderer, (pos.x > WINDOW_WIDTH) ? WINDOW_WIDTH : pos.x, (pos.y > WINDOW_HEIGHT) ? WINDOW_HEIGHT : pos.y, origin.x, origin.y);
    return pos;
}

struct Polygon {
    std::vector<int16_t> posX;
    std::vector<int16_t> posY;
};

double deltaTimeD;
void draw() {
    // point marching for each pixel on the screen
    /*
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
    */

    // displaying the circles
    //for (auto d : drawables) {
    //    filledCircleRGBA(renderer, d->pos.x, d->pos.y, static_cast<Circle*>(d)->radius, 0, 0, 0, 255);
    //}

    // ray marching for each light
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    for (auto l : lights) {
        std::vector<Polygon> polygons { 1 };
        
        for (int i = 0; i < LIGHT_DIR_COUNT; i++) {
            RayHitInfo hit;
            march_ray_cache(l->pos, light_directions[i], &hit);
            
            polygons.back().posX.push_back(hit.pos.x);
            polygons.back().posY.push_back(hit.pos.y);

            //SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            //SDL_RenderDrawPoint(renderer, hit.pos.x, hit.pos.y);
        }
        for (auto p : polygons) {
            filledPolygonRGBA(renderer, p.posX.data(), p.posY.data(), p.posX.size(), 255, 255, 255, 255);
        }
        filledCircleRGBA(renderer, l->pos.x, l->pos.y, 10, 0, 255, 0, 255);
    }
}

#define PLAYER_SPEED 70
void move_player(Drawable *player) {
    SDL_PumpEvents();
    auto keyboard = SDL_GetKeyboardState(NULL);
    
    if (keyboard[SDL_SCANCODE_W] == SDL_PRESSED)
        player->pos.y -= PLAYER_SPEED * deltaTimeD;
    if (keyboard[SDL_SCANCODE_S] == SDL_PRESSED)
        player->pos.y += PLAYER_SPEED * deltaTimeD;
    if (keyboard[SDL_SCANCODE_D] == SDL_PRESSED)
        player->pos.x += PLAYER_SPEED * deltaTimeD;
    if (keyboard[SDL_SCANCODE_A] == SDL_PRESSED)
        player->pos.x -= PLAYER_SPEED * deltaTimeD;
        
    if (keyboard[SDL_SCANCODE_Q] == SDL_PRESSED)
        exit(0);
}

uint64_t deltaTime;
uint64_t startTime, endTime;
int main() {
    // initialize sdl
    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(WINDOW_WIDTH, WINDOW_HEIGHT, 0, &window, &renderer);

    create_drawables();
    create_lights();
    
    // pre-calculate the light directions
    for (int i = 0; i < LIGHT_DIR_COUNT; i++) {
        light_directions[i] = { static_cast<float>(cos(static_cast<float>(i) / LIGHT_DIR_COUNT * 2 * PI)), static_cast<float>(sin(static_cast<float>(i) / LIGHT_DIR_COUNT * 2 * PI)) };
    }
    // pre-calculate the pointmarching cache for raymarching
    precalc_pm_cache();

    // workaround player
    drawables.emplace_back(new Circle{ { WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 }, 30 });
    Drawable *player = lights.front();

    // render loop
    while (1) {
        startTime = SDL_GetTicks64();
    
        // poll window and quit if needed
        if (SDL_PollEvent(&event) && event.type == SDL_QUIT)
            break;
        
        move_player(player);

        SDL_SetRenderDrawColor(renderer, DEF_BG_COL_R, DEF_BG_COL_G, DEF_BG_COL_B, DEF_BG_COL_A);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, DEF_COL_R, DEF_COL_G, DEF_COL_B, DEF_COL_A);

        // for (int x = 0; x < WINDOW_WIDTH; x++) {
        //     for (int y = 0; y < WINDOW_HEIGHT; y++) {
        //         float c = pm_cache({(float)x, (float)y});
        //         SDL_SetRenderDrawColor(renderer, c, c, c, 255);
        //         SDL_RenderDrawPoint(renderer, x, y);
        //     }
        // }

        // you may guess 3 times
        draw();

        SDL_RenderPresent(renderer);
        
        endTime = SDL_GetTicks64();
        deltaTime = endTime - startTime;
        deltaTimeD = deltaTime / 1000.0;
        printf("\rDelta Time is: %lli     ", deltaTime);
        fflush(stdout);
    }

    destroy_drawables();

    // tidy up sdl
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_SUCCESS;
}