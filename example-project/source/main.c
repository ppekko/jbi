#include <SDL2/SDL.h>
#include <math.h>
#include "otherfile.h"

int main() {
    somefunction();
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("JBI Compiled Program", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Rect rect = {240, 180, 160, 160};

    int angle = 0;
    int is_running = 1;


    while (is_running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                is_running = 0;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

        // Calculate the rotation center
        int centerX = rect.x + rect.w / 2;
        int centerY = rect.y + rect.h / 2;

        // Calculate the new corner positions after rotation
        double radians = angle * M_PI / 180.0;
        double cosAngle = cos(radians);
        double sinAngle = sin(radians);

        int x1 = centerX + (rect.w / 2) * cosAngle - (rect.h / 2) * sinAngle;
        int y1 = centerY + (rect.w / 2) * sinAngle + (rect.h / 2) * cosAngle;

        int x2 = centerX - (rect.w / 2) * cosAngle - (rect.h / 2) * sinAngle;
        int y2 = centerY - (rect.w / 2) * sinAngle + (rect.h / 2) * cosAngle;

        int x3 = 2 * centerX - x1;
        int y3 = 2 * centerY - y1;

        int x4 = 2 * centerX - x2;
        int y4 = 2 * centerY - y2;

        // Draw the rotated square
        SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
        SDL_RenderDrawLine(renderer, x2, y2, x3, y3);
        SDL_RenderDrawLine(renderer, x3, y3, x4, y4);
        SDL_RenderDrawLine(renderer, x4, y4, x1, y1);

        SDL_RenderPresent(renderer);

        SDL_Delay(16);
        angle = (angle + 1) % 360;
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
