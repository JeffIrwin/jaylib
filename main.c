
#include <stddef.h>
#include <stdio.h>

#include "raylib.h"

//const int screenWidth = 800;
//const int screenHeight = 450;
//const int screenWidth = 1080;
//const int screenHeight = 1080;
const int screenWidth = 800;
const int screenHeight = 600;

int main(void)
{
    InitWindow(screenWidth, screenHeight, "jaylib");

    Shader shader = LoadShader(0, "shader.glsl");
    //Shader shader = LoadShader(0, TextFormat("resources/shaders/glsl%i/julia_set.fs", GLSL_VERSION));

	// TODO: how to catch shader compiler error? Happens at runtime
	printf("shader id = %d\n", shader.id);
	//return 0;

    // Create a RenderTexture2D to be used for render to texture
    RenderTexture2D target = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());

	float vtime = 0.f;  // virtual time (not real, ticks up predictably regardless of perf)

    // Get variable (uniform) locations on the shader to connect with the program
    // NOTE: If uniform variable could not be found in the shader, function returns -1
    int vtimeLoc = GetShaderLocation(shader, "vtime");

    // Upload the shader uniform values!
    SetShaderValue(shader, vtimeLoc, &vtime, SHADER_UNIFORM_FLOAT);

    bool showControls = true;           // Show controls

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

	// TODO: dry up frame size, frame rate, etc.

	/* initialize */
	//avconv = popen("avconv -y -f rawvideo -s 800x600 -pix_fmt rgb24 -r 25 -i - -vf vflip -an -b:v 1000k test.mp4", "w");
	//avconv = popen("ffmpeg -y -f rawvideo -s 800x600 -pix_fmt rgba32 -r 25 -i - -vf vflip -an -b:v 1000k test.mp4", "w");
	//avconv = popen("ffmpeg -y -f rawvideo -s 800x600 -pix_fmt rgb24 -r 25 -i - -vf vflip -an -b:v 1000k test.mp4", "w");
	//avconv = popen("ffmpeg -y -f rawvideo -s 800x600 -pix_fmt rgb24 -r 60 -i - -an -b:v 1000k test.mp4", "w");
	//avconv = popen("ffmpeg -y -f rawvideo -s 800x600 -pix_fmt rgb24 -r 60 -i - -an test.mp4", "w");

	avconv = popen("ffmpeg -y -f rawvideo -s 800x600 -pix_fmt rgb24 -r 60 -i - -an -pix_fmt yuv420p jaylib-0.mp4", "w");

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

        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        // Using a render texture to draw Julia set
        BeginTextureMode(target);       // Enable drawing to texture
            ClearBackground(WHITE);     // Clear the render texture

            // Draw a rectangle in shader mode to be used as shader canvas
            // NOTE: Rectangle uses font white character texture coordinates,
            // so shader can not be applied here directly because input vertexTexCoord
            // do not represent full screen coordinates (space where want to apply shader)
            DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), WHITE);
        EndTextureMode();
            
        BeginDrawing();
            ClearBackground(WHITE);     // Clear screen background

            // Draw the saved texture and rendered julia set with shader
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
			//// Save frame as individual image to disk -- slow!
			//TakeScreenshot("test.png");

			// Save frame in memory -- faster thank disk
			Image image = LoadImageFromScreen();

			int w = image.width;
			int h = image.height;

			////printf("image data 0 = %x\n", image.data);
			//printf("image data 0 = %x\n", *((int *) image.data));
			//printf("image width  = %d\n", w);
			//printf("image height = %d\n", h);
			//printf("image format = %d\n", image.format);
			//printf("\n");

			// Convert pixel format
			//ImageFormat(&image, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
			ImageFormat(&image, PIXELFORMAT_UNCOMPRESSED_R8G8B8);

			////printf("image data 0 = %x\n", *((int *) image.data));
			//printf("image data 0 = %x\n", *((int *) (&image.data[0])));
			//printf("image data 1 = %x\n", *((int *) (&image.data[3])));
			//printf("image data f = %x\n", *((int *) (&image.data[3*w*h - 3])));
			////printf("image data f = %x\n", *((int *) (&image.data)[w * h - 1]));
			//printf("image format = %d\n", image.format);

			// Write the raw pixels to the ffmpeg pipe

			//glReadPixels(0, 0, 800, 600, GL_RGB, GL_UNSIGNED_BYTE, pixels);
			if (avconv)
			{
			    //fwrite(pixels ,800*600*3 , 1, avconv);
			    fwrite(image.data ,800*600*3 , 1, avconv);
			}
		}
    }

	/* ffmpeg video cleanup */
	if (avconv)
	    pclose(avconv);

    // Raylib de-initialization
    //--------------------------------------------------------------------------------------
    UnloadShader(shader);               // Unload shader
    UnloadRenderTexture(target);        // Unload render texture

    CloseWindow();                      // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

