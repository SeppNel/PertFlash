#include <SDL.h>
#include <stdio.h>
#include <pthread.h> 
#include <time.h>
#include "serial.h"

#define SERIAL_CLICK -1.0
#define SERIAL_READ_ERROR -69.0
#define AVG_UPPER_LIMIT 1000.0
#define AVG_LOWER_LIMIT 1.0

int mainRunning = 1;
int comIndex;

// Initial color
SDL_Color currentColor = {0, 0, 0, 255}; // Black

void printUsageMessage(){
    printf("Usage: PertFlash.exe ComPort [-v] [-f]\n");
    printf("\n");
    printf("ComPort: Serial port of the arduino.\n");
    printf("-v: Enable Vsync.\n");
    printf("-f: Enable Fullscreen.\n");
}

void argumentParser(int argc, char* argv[], int* vsync, int* full){
    *vsync = 0;
    *full = 0;
    for (size_t i = 0; i < argc; i++){
        if (strcmp(argv[i], "-v") == 0) {
            *vsync = 1;
        }
        else if (strcmp(argv[i], "-f") == 0) {
            *full = SDL_WINDOW_FULLSCREEN;
        }

    }
}

void cleanup(SDL_Renderer* r, SDL_Window* w){
    SDL_DestroyRenderer(r);
    SDL_DestroyWindow(w);
    SDL_Quit();
}

void *serialThread(){
    //Initialize Serial
    while(!serial_setup(comIndex)){
        sleep(1);
    }

    double sum = 0.0;
    int runs = 0;
    while(1){
        float f = serial_read();
        if(f == SERIAL_CLICK){
            if (currentColor.r == 255){
                currentColor = (SDL_Color){0, 0, 0, 255}; // Black
            } else {
                currentColor = (SDL_Color){255, 255, 255, 255}; // White
            }
        }
        else if(f > 0.0){
            printf("Response Time: %f \n", f);
            if(f < AVG_UPPER_LIMIT && f > AVG_LOWER_LIMIT){
                sum += f;
                runs++;
            }

            if(runs % 50 == 0 && runs > 0){
                printf("Avg in %i runs = %f\n", runs, sum/runs);
            }
        }
        else if(f == SERIAL_READ_ERROR){
            printf("Error reading serial port\n");
            mainRunning = 0;
        }
    }
    
} 

int main(int argc, char* argv[]){
    if(argc < 2){
        printUsageMessage();
        return 1;
    }

    comIndex = atoi(argv[1]);
    int vsync = 0;
    int full = 0;
    int* p_vsync = &vsync;
    int* p_full = &full;

    argumentParser(argc, argv, p_vsync, p_full);
    
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0){
        fprintf(stderr, "SDL initialization failed: %s\n", SDL_GetError());
        return 1;
    }

    // Create window
    SDL_Window* window = SDL_CreateWindow("Color Changer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 600, 600, full);
    if (!window){
        fprintf(stderr, "Window creation failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Create renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer){
        fprintf(stderr, "Renderer creation failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Set Vsync
    SDL_RenderSetVSync(renderer, vsync);

    // Set the color and clear the screen
    SDL_SetRenderDrawColor(renderer, currentColor.r, currentColor.g, currentColor.b, currentColor.a);
    SDL_RenderClear(renderer);
    // Render the color rectangle
    SDL_RenderPresent(renderer);

    pthread_t thread_id; 
    pthread_create(&thread_id, NULL, serialThread, NULL); 

    // Main loop
    SDL_Event event;
    while (mainRunning){
        //Poll SDL Events
        if(SDL_PollEvent(&event)){
            if (event.type == SDL_QUIT){
                mainRunning = 0;
            }
        }

        // Set the color and clear the screen
        SDL_SetRenderDrawColor(renderer, currentColor.r, currentColor.g, currentColor.b, currentColor.a);
        SDL_RenderClear(renderer);

        // Render the color rectangle
        SDL_RenderPresent(renderer);
    }

    cleanup(renderer, window);

    return 0;
}
