#include <iostream>
#include <chrono>
#include <SDL2/SDL.h>
#include "chip8.hpp"

const char* WINDOW_TITLE = "CHIP-8 Arcade Console";
const int DISPLAY_SCALE = 12;

void HandleInput(uint8_t* keypad, SDL_Event& event) {
    bool down = (event.type == SDL_KEYDOWN);
    switch (event.key.keysym.sym) {
        // Player 1 (Left Paddle): W / S
        case SDLK_w: keypad[0x1] = down; break; 
        case SDLK_s: keypad[0x4] = down; break;
        // Player 2 (Right Paddle): Up / Down Arrow keys
        case SDLK_UP: keypad[0xC] = down; break; 
        case SDLK_DOWN: keypad[0xD] = down; break;
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "[INFO] Usage: chip8.exe <Path to ROM>" << std::endl;
        return 1;
    }

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "[ERROR] SDL Could not initialize." << std::endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        WINDOW_TITLE, 
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
        VIDEO_WIDTH * DISPLAY_SCALE, VIDEO_HEIGHT * DISPLAY_SCALE, 
        SDL_WINDOW_SHOWN
    );

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, VIDEO_WIDTH, VIDEO_HEIGHT);

    Chip8 chip8;
    chip8.LoadROM(argv[1]);

    bool quit = false;
    SDL_Event event;
    auto lastTime = std::chrono::high_resolution_clock::now();

    while (!quit) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) quit = true;
            if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
                HandleInput(chip8.keypad, event);
            }
        }

        auto currentTime = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float, std::chrono::milliseconds::period>(currentTime - lastTime).count();
        lastTime = currentTime;

        // Regulate game CPU execution cycle speed (~800Hz for responsive Pong)
        static float cycleAccumulator = 0;
        cycleAccumulator += dt;
        while (cycleAccumulator >= 1.25f) {
            chip8.Cycle();
            cycleAccumulator -= 1.25f;
        }

        // Hardware Independent Timers decrementing at 60Hz
        static float timerAccumulator = 0;
        timerAccumulator += dt;
        if (timerAccumulator >= 16.66f) {
            chip8.UpdateTimers();
            timerAccumulator = 0;
        }

        // Send matrix data to the GPU via SDL texture
        SDL_UpdateTexture(texture, nullptr, chip8.video, VIDEO_WIDTH * sizeof(uint32_t));
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
