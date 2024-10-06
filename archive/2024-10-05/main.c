
#include <stddef.h>
#include <stdio.h>

#include "raylib.h"

//const int screenWidth = 1920;
//const int screenHeight = 1920;
//const int screenWidth = 1440;
//const int screenHeight = 1440;
//const int screenWidth = 1400;
//const int screenHeight = 1400;
//const int screenWidth = 1360;
//const int screenHeight = 1360;
//const int screenWidth = 1320;
//const int screenHeight = 1320;

const int screenWidth = 1280;
const int screenHeight = 1280;
//const int screenWidth = 800;
//const int screenHeight = 800;

//const int screenWidth = 1200;
//const int screenHeight = 1200;
//const int screenWidth = 1080;
//const int screenHeight = 1080;
//const int screenWidth = 900;
//const int screenHeight = 700;

int main(void)
{
	InitWindow(screenWidth, screenHeight, "jaylib");

	// Load shader file relative to the path of this C source file.  This will
	// break if you compile and then move the shader

	//char* this_dir = __FILE__;
	char* this_file = __FILE__;
	char* slash = strrchr(this_file, '/');

	size_t len = slash - this_file;
	printf("len = %d\n", len);

	char* this_dir = malloc(len);
	strncpy(this_dir, this_file, len);

	char* shader_file = TextFormat("%s/shader.glsl", this_dir);

	printf("this_dir = %s\n", this_dir);
	printf("shader_file = %s\n", shader_file);
	printf("\n");

	Shader shader = LoadShader(0, shader_file);

	// TODO: how to catch shader compiler error? Happens at runtime
	printf("shader id = %d\n", shader.id);
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

	const int FPS = 60;
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

	// TODO: dry up frame rate

	/* initialize */
	avconv = popen(TextFormat("ffmpeg -y -f rawvideo -s %dx%d -pix_fmt rgb24 -r 60 -i - -an -pix_fmt yuv420p jaylib-1.mp4", screenWidth, screenHeight), "w");

	//--------------------------------------------------------------------------------------

	// Main loop
	while (!WindowShouldClose())
	{
		if (IsKeyPressed(KEY_SPACE)) iframe = 0;  // re-start animation
		if (IsKeyPressed(KEY_F1)) showControls = !showControls;  // Toggle whether or not to show controls

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

		//if (iframe == 30)
		if (true)
		{
			// Save frame in memory -- faster than disk
			Image image = LoadImageFromScreen();

			int w = image.width;
			int h = image.height;

			// Convert pixel format
			ImageFormat(&image, PIXELFORMAT_UNCOMPRESSED_R8G8B8);
			//ImageFormat(&image, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);

			// Write the raw pixels to the ffmpeg pipe
			if (avconv)
			{
				const int PIXEL_BYTES = 3;
				fwrite(image.data, screenWidth * screenHeight * PIXEL_BYTES, 1, avconv);
			}
		}

		if (iframe == 600) break;
		//if (iframe == 600) CloseWindow();
	}

	/* ffmpeg video cleanup */
	if (avconv)
		pclose(avconv);

	// Raylib de-initialization
	UnloadShader(shader);
	UnloadRenderTexture(target);

	CloseWindow();

	return 0;
}

