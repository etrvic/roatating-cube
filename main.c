#include <stdio.h>
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
#define DISTANCE 300
#define DEGREE_RAD 0.01745

#define NUM_POINTS 500
#define MIN_PIXELS_PER_SECOND 30
#define MAX_PIXELS_PER_SECOND 60
static SDL_FPoint *points = NULL;
static float *point_speeds = NULL;
int p_size = 0;
int c = 1000;
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
int bonds[][2] = {{1, 2}, {1, 3}, {2, 4}, {3, 4}, {3, 7}, {4, 8},
                  {1, 5}, {2, 6}, {5, 6}, {5, 7}, {7, 8}, {6, 8}};

SDL_FPoint cube[8];
double cube_pos[8][3];

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
  for (int j = 0; j < 8; j++) {
    cube_pos[j][1] = cube_pos[j][0] = cube_pos[j][2] = DISTANCE / 2;
  }
  cube_pos[0][0] = cube_pos[2][0] = cube_pos[4][0] = cube_pos[6][0] =
      -DISTANCE / 2;
  cube_pos[2][1] = cube_pos[6][1] = cube_pos[3][1] = cube_pos[7][1] =
      -DISTANCE / 2;
  cube_pos[0][2] = cube_pos[1][2] = cube_pos[2][2] = cube_pos[3][2] =
      DISTANCE / 2;
  cube_pos[4][2] = cube_pos[5][2] = cube_pos[6][2] = cube_pos[7][2] =
      -DISTANCE / 2;
  last_time = SDL_GetTicks();
  SDL_SetRenderVSync(renderer, 1);
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
double dz = 0;
/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void *appstate) {
  const Uint64 now = SDL_GetTicks();
  const float elapsed =
      ((float)(now - last_time)) / 1000.0f; /* seconds since last iteration */
  int i;
  for (i = 0; i < 8; i++) {
    double x = cube_pos[i][0];
    double y = cube_pos[i][1];
    double z = cube_pos[i][2];
    double a = 1 * DEGREE_RAD;
    double dx = cos(a) * x + sin(a) * z;
    double dz = -sin(a) * x + cos(a) * z;
    cube_pos[i][0] = dx;
    cube_pos[i][2] = dz;
  }
  for (i = 0; i < 8; i++) {
    double z = cube_pos[i][2] / DISTANCE / 2 + 1.2;
    cube[i].x = cube_pos[i][0] / z;
    cube[i].y = cube_pos[i][1] / z;
  }

  last_time = now;

  SDL_SetRenderDrawColor(renderer, 0, 0, 0,
                         SDL_ALPHA_OPAQUE); /* black, full alpha */
  SDL_RenderClear(renderer);                /* start with a blank canvas. */
  SDL_SetRenderDrawColor(renderer, 255, 255, 255,
                         SDL_ALPHA_OPAQUE); /* white, full alpha */
  for (i = 0; i < 12; i++) {
    int x1 = cube[bonds[i][0] - 1].x + window_w / 2;
    int x2 = cube[bonds[i][1] - 1].x + window_w / 2;
    int y1 = cube[bonds[i][0] - 1].y + window_h / 2;
    int y2 = cube[bonds[i][1] - 1].y + window_h / 2;
    SDL_RenderLine(renderer, x1, y1, x2, y2);
  }
  /* You can also draw single points with SDL_RenderPoint(), but it's
     cheaper (sometimes significantly so) to do them all at once. */

  SDL_RenderPresent(renderer); /* put it all on the screen */

  return SDL_APP_CONTINUE;
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result) {}
