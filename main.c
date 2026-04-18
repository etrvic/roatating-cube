#include <stdlib.h>
#define SDL_MAIN_USE_CALLBACKS 1 /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <math.h>

/* We will use this renderer to draw into this window every frame. */
static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static Uint64 last_time = 0;

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

#define NUM_POINTS 500
#define MIN_PIXELS_PER_SECOND 30
#define MAX_PIXELS_PER_SECOND 60
static SDL_FPoint *points = NULL;
static float *point_speeds = NULL;
int p_size = 0;

int window_w = WINDOW_WIDTH;
int window_h = WINDOW_HEIGHT;
// clang-format off
short map[] = {
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 0, 1, 0, 0, 0, 0, 1,
    1, 0, 1, 1, 0, 0, 1, 1,
    1, 0, 0, 1, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 1, 0, 1,
    1, 0, 0, 1, 1, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1
};
// clang-format on
int size = (int)sqrt(sizeof(map));

short bonds[][2] = {{1, 2}, {1, 3}, {2, 4}, {3, 4}, {3, 7}, {4, 8},
                    {1, 5}, {2, 6}, {5, 6}, {5, 7}, {7, 8}, {6, 8}};

SDL_FPoint cube[8];
void point_initialize(SDL_FPoint *points, float *points_speed, int size) {
  int i;
  for (i = 0; i < size; i++) {
    points[i].x = SDL_randf() * ((float)window_w);
    points[i].y = SDL_randf() * ((float)window_h);
    point_speeds[i] =
        MIN_PIXELS_PER_SECOND +
        (SDL_randf() * (MAX_PIXELS_PER_SECOND - MIN_PIXELS_PER_SECOND));
  }
}

/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {

  SDL_SetAppMetadata("Raycaster", "1.0", "com.example.raycaster");

  if (!SDL_Init(SDL_INIT_VIDEO)) {
    SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }
  if (!SDL_CreateWindowAndRenderer("raycasting", window_w, window_h,
                                   SDL_WINDOW_RESIZABLE, &window, &renderer)) {
    SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }
  SDL_SetRenderLogicalPresentation(renderer, window_w, window_h,
                                   SDL_LOGICAL_PRESENTATION_LETTERBOX);

  /* set up the data for a bunch of points. */
  points = (SDL_FPoint *)malloc(sizeof(SDL_FPoint) * NUM_POINTS);
  point_speeds = (float *)malloc(sizeof(float) * NUM_POINTS);
  p_size = NUM_POINTS;
  point_initialize(points, point_speeds, p_size);

  last_time = SDL_GetTicks();

  return SDL_APP_CONTINUE;
}

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
  if (event->type == SDL_EVENT_QUIT) {
    return SDL_APP_SUCCESS;
  } else if (event->type == SDL_EVENT_WINDOW_RESIZED) {
    window_w = event->window.data1;
    window_h = event->window.data2;
    SDL_SetRenderLogicalPresentation(renderer, window_w, window_h,
                                     SDL_LOGICAL_PRESENTATION_DISABLED);
  }
  return SDL_APP_CONTINUE;
}

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void *appstate) {
  const Uint64 now = SDL_GetTicks();
  const float elapsed =
      ((float)(now - last_time)) / 1000.0f; /* seconds since last iteration */
  int i;

  /* let's move all our points a little for a new frame. */
  for (i = 0; i < p_size; i++) {
    const float distance = elapsed * point_speeds[i];
    points[i].x += distance;
    points[i].y += distance;
    if ((points[i].x >= window_w) || (points[i].y >= window_h)) {
      /* off the screen; restart it elsewhere! */
      if (SDL_rand(2)) {
        points[i].x = SDL_randf() * ((float)window_w);
        points[i].y = 0.0f;
      } else {
        points[i].x = 0.0f;
        points[i].y = SDL_randf() * ((float)window_h);
      }
      point_speeds[i] =
          MIN_PIXELS_PER_SECOND +
          (SDL_randf() * (MAX_PIXELS_PER_SECOND - MIN_PIXELS_PER_SECOND));
    }
  }

  last_time = now;

  SDL_SetRenderDrawColor(renderer, 0, 0, 0,
                         SDL_ALPHA_OPAQUE); /* black, full alpha */
  SDL_RenderClear(renderer);                /* start with a blank canvas. */
  SDL_SetRenderDrawColor(renderer, 255, 255, 255,
                         SDL_ALPHA_OPAQUE);   /* white, full alpha */
  SDL_RenderPoints(renderer, points, p_size); /* draw all the points! */

  /* You can also draw single points with SDL_RenderPoint(), but it's
     cheaper (sometimes significantly so) to do them all at once. */

  SDL_RenderPresent(renderer); /* put it all on the screen */

  return SDL_APP_CONTINUE;
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result) {}
