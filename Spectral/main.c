//
//  main.m
//  Spectral TP using FMOD, SDL2, SDL2_image and SDL2_ttf
//
//  Created by Romain GUICHERD on 09/01/2020.
//  Copyright © 2020 Romain GUICHERD. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <SDL2/SDL.h>
#include <SDL2_image/SDL_image.h>
#include <SDL2_ttf/SDL_ttf.h>
#include "fmod.h"
#include "math.h"

#define SPECTRUM_SIZE 1024

// Structure for spectrum
struct spect{
    SDL_Window *window;
    SDL_Surface *screen;
    FMOD_CHANNEL *channel;
    FMOD_DSP *fftDSP;
};
typedef struct spect spect;

// Prototype function setPixel
void setPixel(SDL_Surface *surface, int x, int y, Uint32 pixel);

// Prototype function update spectrum
Uint32 upSpectrum(Uint32 interval, void *parameter);

int main(int argc, char *argv[]){
    
    // Declare pointers and position variables
    SDL_Window *window = NULL;
    SDL_Surface *text = NULL;
    SDL_Rect position;
    SDL_Rect positionTxt;
    SDL_TimerID timer = 0;
    SDL_Color whiteColor = {255, 255, 255};
    position.x = 0;
    position.y = 0;
    positionTxt.x = 750;
    positionTxt.y = 25;
    TTF_Font *font = NULL;
    FMOD_SYSTEM *system = NULL;
    FMOD_CHANNEL *channel1 = NULL;
    FMOD_SOUND *guitar = NULL;
    FMOD_BOOL state = 0;
    FMOD_RESULT result = FMOD_OK;
    FMOD_DSP *fftDSP = NULL;
    
    // Initialize SDL2 and check for issues
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) == -1){
        fprintf(stderr, "Error in opening SDL2: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    
    // Initialize SDL2_ttf and check for issues
    if (TTF_Init() == -1){
        fprintf(stderr, "Error in opening SDL2_ttf: %s\n", TTF_GetError());
        exit(EXIT_FAILURE);
    }
    
    // Spectral system creation, initialization and check for issues
    FMOD_System_Create(&system);
    result = FMOD_System_Init(system, 2, FMOD_INIT_NORMAL, NULL);
    if (result != FMOD_OK){
    fprintf(stderr, "Issue in FMOD system init!\n");
        fprintf(stderr, "Error code: %u\n", result);
        exit(EXIT_FAILURE);
    }
    
    // Check existence of music file
    if (access("Guitar.mp3", F_OK) != -1){
        fprintf(stderr, "Music file found in directory\n");
    }
    else{
        fprintf(stderr, "Music file not found!!!\n");
    }
    
    // FMOD load sound and check for issues
    result = FMOD_System_CreateSound(system, "Guitar.mp3", FMOD_2D | FMOD_CREATESTREAM, 0, &guitar);
    if (result != FMOD_OK){
        fprintf(stderr, "Issue loading sound!\n");
        fprintf(stderr, "Error code: %u\n", result);
        exit(EXIT_FAILURE);
    }
    
    // Create DSP fft type and check for issues
    result = FMOD_System_CreateDSPByType(system, FMOD_DSP_TYPE_FFT, &fftDSP);
    if (result != FMOD_OK){
        fprintf(stderr, "Issue creating fft DSP type!\n");
        fprintf(stderr, "Error code: %u\n", result);
        exit(EXIT_FAILURE);
    }
    
    // Set DSP fft window size
    result = FMOD_DSP_SetParameterInt(fftDSP, FMOD_DSP_FFT_WINDOWSIZE, 2*SPECTRUM_SIZE);
    if (result != FMOD_OK){
        fprintf(stderr, "Issue setting fft DSP size parameters!\n");
        fprintf(stderr, "Error code: %u\n", result);
        exit(EXIT_FAILURE);
    }
    
    // Set DSP fft window type
    result = FMOD_DSP_SetParameterInt(fftDSP, FMOD_DSP_FFT_WINDOWTYPE, FMOD_DSP_FFT_WINDOW_RECT);
    if (result != FMOD_OK){
        fprintf(stderr, "Issue setting fft DSP type parameters!\n");
        fprintf(stderr, "Error code: %u\n", result);
        exit(EXIT_FAILURE);
    }
    
    // Create an application window with the following settings:
    window = SDL_CreateWindow(
        "FMOD: Spectral Analysis",         // window title
        SDL_WINDOWPOS_UNDEFINED,           // initial x position
        SDL_WINDOWPOS_UNDEFINED,           // initial y position
        1024,                              // width, in pixels
        512,                               // height, in pixels
        SDL_WINDOW_OPENGL                  // flags
    );

    // Check that the window was successfully created
    if (window == NULL){
        printf("Could not create window: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    // This makes our buffer swap syncronized with the monitor's vertical refresh
    SDL_GL_SetSwapInterval(1);

    // Set window icon
    SDL_SetWindowIcon(window, SDL_LoadBMP("sdl_icone.bmp"));
    
    // Generate pointer to window surface
    SDL_Surface *screen = SDL_GetWindowSurface(window);

    // Fill SDL window screen with RGB color black
    SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
    
    // Update SDL window
    SDL_UpdateWindowSurface(window);
    
    // Hide mouse cursor
    SDL_ShowCursor(SDL_DISABLE);
    
    // Load font angelina
    font = TTF_OpenFont("angelina.ttf", 65);
    if (!font){
        fprintf(stderr, "TTF_OpenFont: %s\n", TTF_GetError());
    }
    
    // Create text surface with text Guitar
    text = TTF_RenderText_Blended(font, "Guitar", whiteColor);

    // Create SDL event to run
    SDL_Event e;
    int quit = 0;
    while (!quit){
        SDL_PollEvent(&e);
        if (e.type == SDL_QUIT){
            // Remove timer
            SDL_RemoveTimer(timer);
            // Quit application
            quit = 1;
        }
        if (e.type == SDL_KEYDOWN){
            switch(e.key.keysym.sym){
                case SDLK_p:
                    // Get music state on channel 1
                    FMOD_Channel_GetPaused(channel1, &state);
                    // Set pause or unpause
                    if (state == 1){
                        FMOD_Channel_SetPaused(channel1, 0);
                    }
                    else{
                        FMOD_Channel_SetPaused(channel1, 1);
                    }
                    break;
                case SDLK_s:
                    // Stop the music on channel 1
                    FMOD_Channel_Stop(channel1);
                    break;
                case SDLK_m:
                    // FMOD play sound and check for issues
                    result = FMOD_System_PlaySound(system, guitar, NULL, 0, &channel1);
                    if (result != FMOD_OK){
                    fprintf(stderr, "Issue in FMOD system play sound!\n");
                        fprintf(stderr, "Error code: %u\n", result);
                        exit(EXIT_FAILURE);
                    }
                    // Attach DSP fft to channel 1 and check for issues
                    result = FMOD_Channel_AddDSP(channel1, FMOD_CHANNELCONTROL_DSP_HEAD, fftDSP);
                    if (result != FMOD_OK){
                        fprintf(stderr, "Issue adding fft DSP to channel!\n");
                        fprintf(stderr, "Error code: %u\n", result);
                        exit(EXIT_FAILURE);
                    }
                    // Initialization spectre
                    spect Spectre;
                    
                    // Store Spectre structure
                    Spectre.window = window;
                    Spectre.screen = screen;
                    Spectre.channel = channel1;
                    Spectre.fftDSP = fftDSP;
                    
                    // Start timer once
                    if (timer == 0){
                        // Start timer to update spectrum
                        timer = SDL_AddTimer(5, upSpectrum, &Spectre);
                    }
                    break;
            }
        }
        
        // Blit text
        SDL_BlitSurface(text, NULL, screen, &positionTxt);
        
        // FMOD system update
        FMOD_System_Update(system);
    }
    
    // Spectral close and release
    FMOD_Sound_Release(guitar);
    FMOD_System_Close(system);
    FMOD_System_Release(system);
    
    // Free rectangle, image and destroy the window
    SDL_FreeSurface(text);
    SDL_DestroyWindow(window);
    
    // Close the font
    TTF_CloseFont(font);

    // Clean up
    TTF_Quit();
    SDL_Quit();
    
    // Function return
    return EXIT_SUCCESS;
}

// Function to color a pixel
void setPixel(SDL_Surface *surface, int x, int y, Uint32 pixel){
    int bpp = surface->format->BytesPerPixel;

    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch(bpp){
        case 1:
            *p = pixel;
            break;
        case 2:
            *(Uint16 *)p = pixel;
            break;
        case 3:
            if (SDL_BYTEORDER == SDL_BIG_ENDIAN){
                p[0] = (pixel >> 16) & 0xff;
                p[1] = (pixel >> 8) & 0xff;
                p[2] = pixel & 0xff;
            }
            else{
                p[0] = pixel & 0xff;
                p[1] = (pixel >> 8) & 0xff;
                p[2] = (pixel >> 16) & 0xff;
            }
            break;
        case 4:
            *(Uint32 *)p = pixel;
            break;
    }
}

// Function to update the spectrum
Uint32 upSpectrum(Uint32 interval, void *parameter){
    
    // Parameter type
    spect* param = parameter;
    
    // Initialization
    FMOD_RESULT result = FMOD_OK;
    FMOD_DSP_PARAMETER_FFT *fftData = NULL;
    float spectrum_array1[SPECTRUM_SIZE] = {0};
    float spectrum_array2[SPECTRUM_SIZE] = {0};
    float spectrum[SPECTRUM_SIZE] = {0};
    float dominantFreq = 0;
    int height[SPECTRUM_SIZE] = {0};
    
    // Get spectrum and dominant frequency playing on channel 1 and check for issues
    if (param->channel != NULL & param->fftDSP != NULL){
         // Get dominant frequency
         result = FMOD_DSP_GetParameterFloat(param->fftDSP, FMOD_DSP_FFT_DOMINANT_FREQ, &dominantFreq, NULL, 0);
         if (result != FMOD_OK){
           fprintf(stderr, "Issue in FMOD get DSP data!\n");
               fprintf(stderr, "Error code: %u\n", result);
               exit(EXIT_FAILURE);
           }
         // Get spectrum
         result = FMOD_DSP_GetParameterData(param->fftDSP, FMOD_DSP_FFT_SPECTRUMDATA, (void**)&fftData, NULL, NULL, 0);
          if (result != FMOD_OK){
            fprintf(stderr, "Issue in FMOD get DSP data!\n");
                fprintf(stderr, "Error code: %u\n", result);
                exit(EXIT_FAILURE);
            }
     }
    
    // Spectrum extraction for right and left audio
    if (fftData != NULL){
        if (fftData->spectrum[0] != NULL && fftData->spectrum[1] != NULL){
            for (int i = 0; i < SPECTRUM_SIZE; i++){
                spectrum_array1[i] = fftData->spectrum[0][i];
                spectrum_array2[i] = fftData->spectrum[1][i];
                spectrum[i] = (spectrum_array1[i] + spectrum_array2[i])/2;
            }
        }
    }
    
    // Compute bar height
    for (int i = 0; i < SPECTRUM_SIZE; i++){
        height[i] = 511 - (int) (512*pow(spectrum[i], 0.15));
    }
    
    // Fill SDL window screen with black
    SDL_FillRect(param->screen, NULL, SDL_MapRGB(param->screen->format, 0, 0, 0));
    
    // Lock screen
    SDL_LockSurface(param->screen);
    
    // Draw the spectrum bars
    for (int i = 0; i < SPECTRUM_SIZE; i++){
        for (int j = 511; j > height[i]; j--){
            setPixel(param->screen, i, j, SDL_MapRGB(param->screen->format, 255 - j*255/512, 0, j*255/512));
        }
    }
    
    // Unlock screen
    SDL_UnlockSurface(param->screen);
    
    // Update SDL window
    SDL_UpdateWindowSurface(param->window);
    
    // Function return
    return interval;
}
