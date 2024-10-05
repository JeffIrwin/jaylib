
#include <stddef.h>
#include <stdio.h>

#include "raylib.h"

// A few good julia sets
const float pointsOfInterest[6][2] =
{
    { -0.348827f, 0.607167f },
    { -0.786268f, 0.169728f },
    { -0.8f, 0.156f },
    { 0.285f, 0.0f },
    { -0.835f, -0.2321f },
    { -0.70176f, -0.3842f },
};

//const int screenWidth = 800;
//const int screenHeight = 450;
//const int screenWidth = 1080;
//const int screenHeight = 1080;
const int screenWidth = 800;
const int screenHeight = 600;

const float zoomSpeed = 1.01f;
const float offsetSpeedMul = 2.0f;

const float startingZoom = 0.75f;

int main(void)
{
    InitWindow(screenWidth, screenHeight, "jaylib");

    Shader shader = LoadShader(0, "shader.glsl");
    //Shader shader = LoadShader(0, TextFormat("resources/shaders/glsl%i/julia_set.fs", GLSL_VERSION));

    // Create a RenderTexture2D to be used for render to texture
    RenderTexture2D target = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());

    // c constant to use in z^2 + c
    float c[2] = { pointsOfInterest[0][0], pointsOfInterest[0][1] };

    // Offset and zoom to draw the julia set at. (centered on screen and default size)
    float offset[2] = { 0.0f, 0.0f };
    float zoom = startingZoom;

	float vtime = 0.f;  // virtual time (not real, ticks up predictably regardless of perf)

    // Get variable (uniform) locations on the shader to connect with the program
    // NOTE: If uniform variable could not be found in the shader, function returns -1
    int cLoc = GetShaderLocation(shader, "c");
    int zoomLoc = GetShaderLocation(shader, "zoom");
    int offsetLoc = GetShaderLocation(shader, "offset");
    int vtimeLoc = GetShaderLocation(shader, "vtime");

    // Upload the shader uniform values!
    SetShaderValue(shader, cLoc, c, SHADER_UNIFORM_VEC2);
    SetShaderValue(shader, zoomLoc, &zoom, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, offsetLoc, offset, SHADER_UNIFORM_VEC2);
    SetShaderValue(shader, vtimeLoc, &vtime, SHADER_UNIFORM_FLOAT);

    int incrementSpeed = 0;             // Multiplier of speed to change c value
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
        // Press [1 - 6] to reset c to a point of interest
        if (IsKeyPressed(KEY_ONE) ||
            IsKeyPressed(KEY_TWO) ||
            IsKeyPressed(KEY_THREE) ||
            IsKeyPressed(KEY_FOUR) ||
            IsKeyPressed(KEY_FIVE) ||
            IsKeyPressed(KEY_SIX))
        {
            if (IsKeyPressed(KEY_ONE)) c[0] = pointsOfInterest[0][0], c[1] = pointsOfInterest[0][1];
            else if (IsKeyPressed(KEY_TWO)) c[0] = pointsOfInterest[1][0], c[1] = pointsOfInterest[1][1];
            else if (IsKeyPressed(KEY_THREE)) c[0] = pointsOfInterest[2][0], c[1] = pointsOfInterest[2][1];
            else if (IsKeyPressed(KEY_FOUR)) c[0] = pointsOfInterest[3][0], c[1] = pointsOfInterest[3][1];
            else if (IsKeyPressed(KEY_FIVE)) c[0] = pointsOfInterest[4][0], c[1] = pointsOfInterest[4][1];
            else if (IsKeyPressed(KEY_SIX)) c[0] = pointsOfInterest[5][0], c[1] = pointsOfInterest[5][1];

            SetShaderValue(shader, cLoc, c, SHADER_UNIFORM_VEC2);
        }

        // If "R" is pressed, reset zoom and offset.
        if (IsKeyPressed(KEY_R))
        {
            zoom = startingZoom;
            offset[0] = 0.0f;
            offset[1] = 0.0f;
            SetShaderValue(shader, zoomLoc, &zoom, SHADER_UNIFORM_FLOAT);
            SetShaderValue(shader, offsetLoc, offset, SHADER_UNIFORM_VEC2);
        }

        //if (IsKeyPressed(KEY_SPACE)) incrementSpeed = 0;         // Pause animation (c change)
        if (IsKeyPressed(KEY_SPACE)) iframe = 0;  // re-start animation

        if (IsKeyPressed(KEY_F1)) showControls = !showControls;  // Toggle whether or not to show controls

        if (IsKeyPressed(KEY_RIGHT)) incrementSpeed++;
        else if (IsKeyPressed(KEY_LEFT)) incrementSpeed--;

        // If either left or right button is pressed, zoom in/out.
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) || IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
        {
            // Change zoom. If Mouse left -> zoom in. Mouse right -> zoom out.
            zoom *= IsMouseButtonDown(MOUSE_BUTTON_LEFT)? zoomSpeed : 1.0f/zoomSpeed;

            const Vector2 mousePos = GetMousePosition();
            Vector2 offsetVelocity;
            // Find the velocity at which to change the camera. Take the distance of the mouse
            // from the center of the screen as the direction, and adjust magnitude based on
            // the current zoom.
            offsetVelocity.x = (mousePos.x/(float)screenWidth - 0.5f)*offsetSpeedMul/zoom;
            offsetVelocity.y = (mousePos.y/(float)screenHeight - 0.5f)*offsetSpeedMul/zoom;

            // Apply move velocity to camera
            offset[0] += GetFrameTime()*offsetVelocity.x;
            offset[1] += GetFrameTime()*offsetVelocity.y;

            // Update the shader uniform values!
            SetShaderValue(shader, zoomLoc, &zoom, SHADER_UNIFORM_FLOAT);
            SetShaderValue(shader, offsetLoc, offset, SHADER_UNIFORM_VEC2);
        }

        // Increment c value with time
        const float dc = GetFrameTime()*(float)incrementSpeed*0.0005f;
        c[0] += dc;
        c[1] += dc;
        SetShaderValue(shader, cLoc, c, SHADER_UNIFORM_VEC2);

		iframe += 1;
		vtime = dt * iframe;
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
                DrawText("Press Mouse buttons right/left to zoom in/out and move", 10, 15, 10, RAYWHITE);
                DrawText("Press KEY_F1 to toggle these controls", 10, 30, 10, RAYWHITE);
                DrawText("Press KEYS [1 - 6] to change point of interest", 10, 45, 10, RAYWHITE);
                DrawText("Press KEY_LEFT | KEY_RIGHT to change speed", 10, 60, 10, RAYWHITE);
                DrawText("Press KEY_SPACE to stop movement animation", 10, 75, 10, RAYWHITE);
                DrawText("Press KEY_R to recenter the camera", 10, 90, 10, RAYWHITE);
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
