/*

 The goal of this program is to iterate through photos from a
 folder of images on a Raspberry Pi, then display each photo on a
 attached monitor for a duration of time (ex. 3 Seconds), 
 then transition to randomly to another photo in the image folder 
 without repeating the photos displayed previously. The program 
 is designed in a way that the user can turn off the functionality
 whenever they wish.

 This code is written in C language.

*/

#include <stdio.h> // Standard input-output library
#include <stdlib.h> // Standard library
#include <string.h> // String library (used for working with strings)
#include <strings.h> // String library (used for case-insensitive string comparison functions and bit-processing utilities)
#include <stddef.h> // Holds definitions for fundamental types and macros (ex. "NULL")
#include <dirent.h> // For opening, reading, closing, and other functions related to directories
#include <time.h> // This library is imported for rand() functions to work and look truly "random"

// These two libraries important are Simple DirectMedia Layer (SDL) libraries used for low-level access to standard input-output and graphics hardware
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

/* 

Typedef helps to make sure that 
ImageList is the only name that 
can be associated with the photos
struct.

*/

typedef struct {

    char **filenames;
    int count;
    int capacity;

} ImageList;

/* 

The "init_list" function takes ImageList 
pointer, and initializes values for "list" via 
that pointer, using direct initlization and
dynamic memory allocation through "malloc."

Since we pass the pointer into the parameter rather 
than the struct itself, this is effectively pass by 
reference in C. The function can manage and modify the 
ImageList declaration in main() function, instead of just
making a local copy of it for the function, thus allowing
for the usage of the list throughout the program isntead
of just in the function itself. 

Since no value is needed to be returned,
function is cast as void.

*/

void init_list(ImageList *list){
    list->capacity = 4;
    list->count = 0;
    list->filenames = malloc(list->capacity * sizeof(char *));
}

/* 

The "add_image" function takes the Imagelist pointer,
and a const character "filename" pointer. The purpose 
of this function is to check whether or not there is 
enough space for image files to be stored, and if not,
the capacity is increased for that list. In the end
the goal of this function is just to add and allocate
memory for the respective number of image files in the 
ImageList list declaration.

*/

void add_image(ImageList *list, const char *filename){
    
    if (list->capacity == list->count)
    {
    list->capacity *= 2;
    list->filenames = realloc(list->filenames, list->capacity * sizeof(char *));
    }

    char *name_copy = strdup(filename);

    list->filenames[list->count] = name_copy;
    list->count++;

}

/*

The purpose of the shuffle_list function is to swap the list
of files in the folder which contains images. The
reason for this is to allow for a random, new shuffle
of photos everytime, and so that the same photos do not display 
more than once in one cycle iteration.

It is cast as void because it's functionaility mainly deals
with the struct, and switching around the order of the filenames
that "ImageList *list" struct pointer refers.

*/

void shuffle_list(ImageList *list){

    for(int i = (list->count - 1); i > 0; i--){
        int j = rand() % (i+1);

        char *temp = list->filenames[j];
        list->filenames[j] = list->filenames[i];
        list->filenames[i] = temp;
    }

}

/*

The scan_directory walks through the entire folder of images, 
and then adds them to the "Imagelist *list" via add_image 
function built. In other words,this function walks 
through all entries in the given folder, filters the image
files specifically, and adds them to list via add_image.

*/

void scan_directory(const char* dir_path, ImageList *list){

    printf("Trying to open: %s\n", dir_path);
    DIR *dir = opendir(dir_path);

    if (dir == NULL){
        printf("Could not open directory\n");
        return;
    }

    struct dirent *entry;

    while((entry = readdir(dir)) != NULL){

        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0){
            continue;
        }

        char *dot = strrchr(entry->d_name, '.');

        if (dot == NULL) {
            continue;
        }

        if (strcasecmp(dot, ".jpg") != 0 && strcasecmp(dot, ".jpeg") != 0 && strcasecmp(dot, ".png") != 0) {
            continue;
        }

        char full_path[512];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);

        add_image(list, full_path);

    }

    closedir(dir);
}

/*

The "free_list" function frees the data
allocated to the filenames double pointer
in the init_list function. Freeing the
list allows for the prevrention of memory
leaks when the code runs in a different time,
and prevents any system crashes as well.

*/

void free_list(ImageList *list){
	
	for (int i = 0; i < list->count; i++){
		free(list->filenames[i]);
	}

	free(list->filenames);
}

/*

The "load_image_as_texture" function simply loads the images from 
the folder of images to the screen, directly using Video RAM (VRAM).
This function shines because it allows acceleratted rendering of
the photos, and the processor used is the GPU, not the CPU. What
this function also does is resize the pixels of the maximum pixels
a Raspberry Pi can display (which is 2048x2048). At the end it makes
sure to free up any temporary space and it returns a texture which 
is used in "main" function for rendering the images in the folder.

*/

SDL_Texture *load_image_as_texture(SDL_Renderer *renderer, const char *path){
		
	SDL_Surface *loaded_surface = IMG_Load(path);
	
	if(!loaded_surface){
		printf("Failure to load image %s\n", IMG_GetError());
		return NULL;
	}
	
	// --- Scale image down if it exceeds the GPU's max texture size ---
	int max_dimension = 2000;
	SDL_Surface *surface = loaded_surface;
	
	if (loaded_surface->w > max_dimension || loaded_surface->h > max_dimension) {
		float scale = (float)max_dimension / (loaded_surface->w > loaded_surface->h ? loaded_surface->w : loaded_surface->h);
		int new_w = (int)(loaded_surface->w * scale);
		int new_h = (int)(loaded_surface->h * scale);
		
		surface = SDL_CreateRGBSurface(0, new_w, new_h, 32, 0, 0, 0, 0);
		
		if (surface == NULL){
			printf("Failure to create resized surface %s\n", SDL_GetError());
			SDL_FreeSurface(loaded_surface);
			return NULL;
		}
		
		SDL_BlitScaled(loaded_surface, NULL, surface, NULL);
		SDL_FreeSurface(loaded_surface);
	}
	
	SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_FreeSurface(surface);
		
	if(!texture){
		printf("Failure to load image %s\n", IMG_GetError());
		return NULL;
	}
	
	return texture;
}


// The main() function

int main(int argc, char *argv[]){
	
	// Setting time to NULL with srand so that the random functions work and don't deliver same results every time program runs
	srand(time(NULL));

	// Determine which folder to scan: use the path passed on the command
	// line if provided, otherwise fall back to a default folder.
	const char *image_folder;

	if (argc >= 2) {
		image_folder = argv[1];
	} else {
		image_folder = "./images"; // default folder if none is specified
		printf("No folder specified, defaulting to \"%s\"\n", image_folder);
		printf("Usage: %s <path_to_image_folder>\n", argv[0]);
	}
	
	// Declaring instance of struct ImageList
	ImageList list;

	// Initializing and shuffling list
	init_list(&list);
	scan_directory(image_folder, &list);
	shuffle_list(&list);

	// NULL check
	if (list.count == 0){
		printf("No images found");
		return 1;
	}
	
	int sdl_initialization = SDL_Init(SDL_INIT_VIDEO);

	if (sdl_initialization < 0){
		printf("Failure to initialize %s\n", SDL_GetError());
		SDL_Quit();
		return 1;
	}
	
	// JPG and PNG are prompted to be scanned for in the folder explicitly

	int flags = IMG_INIT_JPG | IMG_INIT_PNG; 
	int initted = IMG_Init(flags);
		
	if((initted & flags) != flags){
		printf("Failure to initialize SDL_image %s\n", IMG_GetError());
		IMG_Quit();
		SDL_Quit();
		return 1;
	}

	// Creating a usable graphical window on the respective user's screen
	
	SDL_Window *window = SDL_CreateWindow("My Test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP);
	
	if(window == NULL){
		printf("Failure to create window %s\n", SDL_GetError());
		IMG_Quit();
		SDL_Quit();
		return 1;
	}
	
	// SDL renderer creates a render by referencing the created graphical window previously, creating a 2D rendering context linked to the specific window created

	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	
	if(!renderer){
		printf("Failure to render %s\n", SDL_GetError());
		IMG_Quit();
		SDL_Quit();
		return 1;
	}
	
	// Initialization of while-loop specific variables for optimal display

	int index = 0;
	int running = 1;
	
	// "while (running)" acts as a infinite loop, and helps for the displaying of multiple photos without crowding too much code

	while (running){
		
		// Check for quit input (Space key or window close) 

		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				running = 0;
			}
			if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE) {
				running = 0;
			}
		}
		
		if (!running) {
			break;
		}
		
		// Creating SDL texture, which is the image data stored on the graphics 

		SDL_Texture *texture = load_image_as_texture(renderer, list.filenames[index]);
	
		if (!texture){
			printf("Failed to load image: %s\n", list.filenames[index]);
			index = (index + 1) % list.count;
			continue;
		}
		
		// Calculate aspect-ratio-preserving destination rectangle 

		int screen_w, screen_h;
		SDL_GetWindowSize(window, &screen_w, &screen_h);
		
		int img_w, img_h;
		SDL_QueryTexture(texture, NULL, NULL, &img_w, &img_h);
		
		float scale_w = (float)screen_w / img_w;
		float scale_h = (float)screen_h / img_h;
		float scale = (scale_w < scale_h) ? scale_w : scale_h;
		
		int dest_w = (int)(img_w * scale);
		int dest_h = (int)(img_h * scale);
		
		SDL_Rect dest_rect;
		dest_rect.w = dest_w;
		dest_rect.h = dest_h;
		dest_rect.x = (screen_w - dest_w) / 2;
		dest_rect.y = (screen_h - dest_h) / 2;
		
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, texture, NULL, &dest_rect);
		SDL_RenderPresent(renderer);
		
		// Allows time for 3000 milliseconds (3 seconds) for every photo displayed
		SDL_Delay(3000);
		SDL_DestroyTexture(texture);
		
		index = (index + 1) % list.count;
		
		if (index == 0) {

			// Reshuffle for the next cycle. Note: this doesn't guarantee the
    		// last image of this cycle differs from the first image of the
    		// next cycle — Fisher-Yates shuffles the list independently each
    		// time, so there's a small chance the same photo displays twice
    		// in a row across the cycle boundary.

			shuffle_list(&list);
		}
	}
	

	// Making sure to free the memory space to prevent memory leaks and bugs in future runs

	free_list(&list);
	
	// Destorying SDL Render and Window in reverse chronological order (since the renderer holds dependency on the window)

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	// De-initializing the SDL_image library and releasing dynamic memory used for loading JPG/PNG images 
	IMG_Quit();

	// Shuts down all SDL subsystems and frees the memory tied to them	
	SDL_Quit();
	
	return 0;

}
