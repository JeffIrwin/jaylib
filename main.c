
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "raylib.h"

// Toggle between 0 (false) and 1 (true)
#define HI_RES \
0

#if HI_RES
	// Somewhere between 1400 and 1440, threads.net compression will kick in and
	// make higher resolutions look like garbage.  1280 is safely below this
	// threshold
	const int screenWidth = 1280;
	const int screenHeight = 1280;
	const int FPS = 60;
#else
	const int screenWidth = 800;
	const int screenHeight = 800;
	const int FPS = 30;
#endif

//****************

#define ESC "\033"
#define GREEN   ESC "[92m"
#define MAGENTA ESC "[95m"
#define RED     ESC "[91;1m"
#define RESET   ESC "[0m"

#define ERROR RED "Error: "

//****************

int main(void)
{
	InitWindow(screenWidth, screenHeight, "jaylib");

	// Load shader file relative to the path of this C source file.  This will
	// break if you compile and then move the shader

	char* this_file = __FILE__;
	char* slash = strrchr(this_file, '/');

	size_t len = slash - this_file;
	printf("len = %ld\n", len);

	char* this_dir = malloc(len + 1);
	*this_dir = '\0';
	strncat(this_dir, this_file, len);

	const char* shader_file = TextFormat("%s/shader.glsl", this_dir);

	printf("this_dir = %s\n", this_dir);
	printf("shader_file = %s\n", shader_file);
	printf("\n");

	const char* shader_text = LoadFileText(shader_file);
	//printf("shader_text = \n%s\n", shader_text);
	//printf("shader_text = \"%s\"\n", shader_text);
	//printf("shader len = %d\n", strlen(shader_text));
	//if (strcmp(shader_text, "(null)") == 0)
	if (shader_text == NULL)
	{
		printf(ERROR "cannot load shader file \"%s\"\n" RESET, shader_file);
		return EXIT_FAILURE;
	}

	//// LoadShader() does not throw an error if shader_file does not exist
	//Shader shader = LoadShader(0, shader_file);
	Shader shader = LoadShaderFromMemory(0, shader_text);

	printf("shader id = %d\n", shader.id);

	// Always true, even if shader file doesn't exist
	bool ready = IsShaderReady(shader);
	printf("shader ready = %d\n", ready);
	if (!ready)
	{
		printf(ERROR "cannot compile shader program from file \"%s\"\n" RESET, shader_file);
		return EXIT_FAILURE;
	}

	//return 0;

	// Create a RenderTexture2D to be used for render to texture
	RenderTexture2D target = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());

	float vtime = 0.f;  // virtual time (not real, ticks up predictably regardless of perf)

	// Get variable (uniform) locations on the shader to connect with the program
	// NOTE: If uniform variable could not be found in the shader, function returns -1
	int vtimeLoc = GetShaderLocation(shader, "vtime");

	// Upload the shader uniform values
	SetShaderValue(shader, vtimeLoc, &vtime, SHADER_UNIFORM_FLOAT);

	//bool showControls = true;
	bool showControls = false;

	SetTargetFPS(FPS);

	float dt = 1.f / FPS;
	printf("dt = %e\n", dt);
	size_t iframe = 0;

	//--------------------------------------------------------------------------------------

	// This is some crazy magic.  Open a pipe into ffmpeg to stream a video
	//
	// Ref:  https://stackoverflow.com/a/25921244/4347028
	//
	FILE *avconv = NULL;

	// Initialize
	//
	// TODO: put videos in subdir
	avconv = popen(TextFormat("ffmpeg -y -f rawvideo -s %dx%d -pix_fmt rgb24 -r %d -i - -an -pix_fmt yuv420p jaylib-3.mp4", screenWidth, screenHeight, FPS), "w");

	//--------------------------------------------------------------------------------------

	// Main loop
	while (!WindowShouldClose())
	{
		if (IsKeyPressed(KEY_SPACE)) iframe = 0;  // re-start animation
		if (IsKeyPressed(KEY_F1)) showControls = !showControls;  // toggle whether or not to show controls

		// Increment time
		vtime = dt * iframe;
		iframe += 1;
		//printf("vtime = %e\n", vtime);
		SetShaderValue(shader, vtimeLoc, &vtime, SHADER_UNIFORM_FLOAT);

		// Draw
		//----------------------------------------------------------------------------------
		BeginTextureMode(target);    // Enable drawing to texture
			ClearBackground(WHITE);

			// Draw a rectangle in shader mode to be used as shader canvas
			// NOTE: Rectangle uses font white character texture coordinates,
			// so shader can not be applied here directly because input vertexTexCoord
			// do not represent full screen coordinates (space where want to apply shader)
			DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), WHITE);
		EndTextureMode();

		BeginDrawing();
			ClearBackground(WHITE);

			// Draw the saved texture
			// NOTE: We do not invert texture on Y, already considered inside shader
			BeginShaderMode(shader);
				// WARNING: If FLAG_WINDOW_HIGHDPI is enabled, HighDPI monitor scaling should be considered
				// when rendering the RenderTexture2D to fit in the HighDPI scaled Window
				DrawTextureEx(target.texture, (Vector2){ 0.0f, 0.0f }, 0.0f, 1.0f, WHITE);
			EndShaderMode();

			if (showControls)
			{
				DrawText("Press F1 to toggle these controls", 10, 30, 10, RAYWHITE);
				DrawText("Press SPACE restart animation", 10, 45, 10, RAYWHITE);
			}
		EndDrawing();
		//----------------------------------------------------------------------------------

		// TODO: add a "dry run" mode that skips ffmpeg export and only renders
		// to screen.  Also skip ffmpeg init and cleanup

		//if (iframe == 30)
		if (true)
		{
			// Save frame in memory -- faster than disk
			Image image = LoadImageFromScreen();

			int w = image.width;
			int h = image.height;

			// Convert pixel format.  TODO: is yuv420 available in raylib?  Then
			// we could skip an extra transcoding step
			ImageFormat(&image, PIXELFORMAT_UNCOMPRESSED_R8G8B8);
			//ImageFormat(&image, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);

			// Write the raw pixels to the ffmpeg pipe
			if (avconv)
			{
				const int PIXEL_BYTES = 3;
				fwrite(image.data, screenWidth * screenHeight * PIXEL_BYTES, 1, avconv);
			}
		}

		const size_t NFRAMES = 10 * FPS;
		if (iframe == NFRAMES) break;
		//if (iframe == NFRAMES - 1) break;

		//if (iframe == NFRAMES) CloseWindow();
	}

	// ffmpeg video cleanup
	if (avconv)
		pclose(avconv);

	// raylib de-initialization
	UnloadShader(shader);
	UnloadRenderTexture(target);
	CloseWindow();

	printf(GREEN "Finished jaylib\n" RESET);
	return 0;
}

