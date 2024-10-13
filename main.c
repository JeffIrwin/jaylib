
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>  // mkdir
#include <time.h>

#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

#define RLIGHTS_IMPLEMENTATION
#include "rlights.h"

// Toggle between 0 (false) and 1 (true)
#define HI_RES \
1

#if HI_RES
	// Somewhere between 1400 and 1440, threads.net compression will kick in and
	// make higher resolutions look like garbage.  1280 is safely below this
	// threshold
	const int WIDTH  = 1280;
	const int HEIGHT = 1280;
	const int FPS = 60;
#else
	const int WIDTH  = 800;
	const int HEIGHT = 800;
	const int FPS = 30;
#endif

#define ME "jaylib"

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#define JAYLIB_MAJOR 0
#define JAYLIB_MINOR 2
#define JAYLIB_PATCH 0
#define JAYLIB_VERS STR(JAYLIB_MAJOR) "." STR(JAYLIB_MINOR) "." STR(JAYLIB_PATCH)

//****************

// These are suffixed with "_ANSI" to avoid clashing with raylib macros
#define ESC "\033"
#define GREEN_ANSI   ESC "[92m"
#define MAGENTA_ANSI ESC "[95m"
#define RED_ANSI     ESC "[91;1m"
#define YELLOW_ANSI  ESC "[93;1m"
#define RESET_ANSI   ESC "[0m"

#define WARNING YELLOW_ANSI "Warning: "
#define ERROR RED_ANSI "Error: "

//==============================================================================

char* get_date()
{
	// Return date in format "yyyy-mm-dd" (%F)
	const size_t len = 16;//32;
	char* buffer = malloc(len);
	time_t rawtime;
	struct tm *timeinfo;
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	strftime(buffer, len, "%F", timeinfo);
	return buffer;
}

//==============================================================================
void DrawPlaneXY(Vector3 centerPos, Vector2 size, Color color)
{
	// Draw a plane
	//
	// Alternatively, this could be implementing by pushing mat, rlRotatef, and
	// then calling raylib's DrawPlane (XZ)
	rlPushMatrix();
	rlTranslatef(centerPos.x, centerPos.y, centerPos.z);
	rlScalef(size.x, size.y, 1.0f);

	rlBegin(RL_QUADS);
		rlColor4ub(color.r, color.g, color.b, color.a);
		rlNormal3f(0.0f, 0.0f, 1.0f);

		rlVertex3f(-0.5f,  0.5f, 0.0f);
		rlVertex3f(-0.5f, -0.5f, 0.0f);
		rlVertex3f( 0.5f, -0.5f, 0.0f);
		rlVertex3f( 0.5f,  0.5f, 0.0f);
	rlEnd();
	rlPopMatrix();
}

void DrawGridXY(int slices, float spacing)
{
	// Draw a grid centered at (0, 0, 0)
	int halfSlices = slices/2;

	rlBegin(RL_LINES);
	for (int i = -halfSlices; i <= halfSlices; i++)
	{
		if (i == 0)
			rlColor3f(0.5f, 0.5f, 0.5f);
		else
			rlColor3f(0.75f, 0.75f, 0.75f);

		rlVertex3f((float)i*spacing, (float)-halfSlices*spacing, 0.0f);
		rlVertex3f((float)i*spacing, (float) halfSlices*spacing, 0.0f);

		rlVertex3f((float)-halfSlices*spacing, (float)i*spacing, 0.0f);
		rlVertex3f((float) halfSlices*spacing, (float)i*spacing, 0.0f);
	}
	rlEnd();
}

//==============================================================================

Color IntToColor(int hex)
{
	// c.f. raylib's ColorToInt()
	int r = (hex / 256 / 256) % 256;
	int g = (hex / 256      ) % 256;
	int b = (hex            ) % 256;
	return (Color) {r, g, b, 0xff};
}

//==============================================================================

int main(void)
{
	printf("\n");
	printf(MAGENTA_ANSI "Starting " ME " " JAYLIB_VERS " main\n" RESET_ANSI);

	char* date = get_date();
	printf("Current date = %s\n", date);
	printf("\n");

	SetConfigFlags(FLAG_MSAA_4X_HINT);  // Enable Multi Sampling Anti Aliasing 4x (if available)
	InitWindow(WIDTH, HEIGHT, ME);

	// Define the camera to look into our 3d world
	//
	// Z-up
	Camera camera = { 0 };
	camera.position = (Vector3){ -2.0f, 6.0, 4.0f };     // Camera position
	camera.target = (Vector3){ 0.0f, 0.0f, 1.5f };      // Camera looking at point
	camera.up = (Vector3){ 0.0f, 0.0f, 1.0f };          // Camera up vector (rotation towards target)
	camera.fovy = 37.0f;                                // Camera field-of-view Y
	camera.projection = CAMERA_PERSPECTIVE;             // Camera projection type

	// Load shader file(s) relative to the path of this C source file.  This
	// will break if you compile and then move the shader

	char* this_file = __FILE__;
	char* slash = strrchr(this_file, '/');

	size_t len = slash - this_file;
	printf("len = %ld\n", len);

	char* this_dir = malloc(len + 1);
	*this_dir = '\0';
	strncat(this_dir, this_file, len);

	const char* fragment_file = TextFormat("%s/fragment.glsl", this_dir);
	const char* vertex___file = TextFormat("%s/vertex.glsl"  , this_dir);

	printf("this_dir = %s\n", this_dir);
	printf("fragment_file = %s\n", fragment_file);
	printf("\n");

	char* fragment_text = LoadFileText(fragment_file);
	if (fragment_text == NULL)
	{
		printf(ERROR "cannot load shader file \"%s\"\n" RESET_ANSI, fragment_file);
		return EXIT_FAILURE;
	}

	char* vertex___text = LoadFileText(vertex___file);
	if (vertex___text == NULL)
	{
		printf(ERROR "cannot load shader file \"%s\"\n" RESET_ANSI, vertex___file);
		return EXIT_FAILURE;
	}

	Shader shader = LoadShaderFromMemory(vertex___text, fragment_text);

	printf("shader id = %d\n", shader.id);

	// Always true, even if shader file doesn't exist
	bool ready = IsShaderReady(shader);
	printf("shader ready = %d\n", ready);
	if (!ready)
	{
		printf(
				ERROR
				"cannot compile shader program from files \"%s\" and \"%s\"\n"
				RESET_ANSI,
				vertex___file, fragment_file
		);
		return EXIT_FAILURE;
	}

	float vtime = 0.f;  // virtual time (not real, ticks up predictably regardless of perf)

	// Get some required shader locations
	shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");
	// NOTE: "matModel" location name is automatically assigned on shader loading,
	// no need to get the location again if using that uniform name
	//shader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocation(shader, "matModel");

	// Ambient light level (some basic lighting)
	int ambientLoc = GetShaderLocation(shader, "ambient");
	SetShaderValue(shader, ambientLoc, (float[4]){ 0.1f, 0.1f, 0.1f, 1.0f }, SHADER_UNIFORM_VEC4);

	// Create lights
	Light lights[MAX_LIGHTS] = { 0 };

	lights[0] = CreateLight(LIGHT_POINT, (Vector3){  3, 1.0, 5 }, Vector3Zero(), BLUE , shader);
	lights[1] = CreateLight(LIGHT_POINT, (Vector3){ -3, 1.5, 5 }, Vector3Zero(), RED  , shader);
	lights[2] = CreateLight(LIGHT_POINT, (Vector3){  3, 1.0, 5 }, Vector3Zero(), RED  , shader);
	lights[3] = CreateLight(LIGHT_POINT, (Vector3){ -3, 1.5, 5 }, Vector3Zero(), RED  , shader);

	//****************

	bool normal_close  = false;

	SetTargetFPS(FPS);
	float dt = 1.f / FPS;
	printf("dt = %e\n", dt);
	size_t iframe = 0;

	const size_t DURATION = 10;  // video length (s)
	const size_t NFRAMES = DURATION * FPS;

	//****************

	const char* outdir     = "videos";
	const char* outdir_img = "screenshots";
	mkdir(outdir    , 0700);
	mkdir(outdir_img, 0700);

	const char* outfile = TextFormat("%s/%s--%s.mp4", outdir    , ME, date);
	const char* outimg  = TextFormat("%s/%s--%s.png", outdir_img, ME, date);

	// This is some crazy magic.  Open a pipe into ffmpeg to stream a video
	//
	// Ref:  https://stackoverflow.com/a/25921244/4347028
	//
	FILE *ffmpeg = NULL;

	// initialize ffmpeg
	ffmpeg = popen(TextFormat(
			"ffmpeg -y -f rawvideo "
			" -s %dx%d "
			" -pix_fmt rgb24 "  // note this relates to raylib PIXELFORMAT ImageFormat below
			" -r %d -i - -an "
			" -pix_fmt yuv420p "
			" %s ",
			WIDTH,
			HEIGHT,
			FPS,
			outfile
	), "w");

	//****************

	// Main game loop
	while (!WindowShouldClose())        // Detect window close button or ESC key
	{
		// Increment time
		vtime = dt * iframe;
		iframe += 1;

		// Update the shader with the camera view vector
		float cameraPos[3] = { camera.position.x, camera.position.y, camera.position.z };
		SetShaderValue(shader, shader.locs[SHADER_LOC_VECTOR_VIEW], cameraPos, SHADER_UNIFORM_VEC3);

		// Draw
		//****************
		BeginDrawing();

			ClearBackground((Color) {50, 50, 80, 0});

			Rectangle rect = (Rectangle) {0, 0, WIDTH, HEIGHT};

			const Color BG_COLOR_1 = IntToColor(0x443377);
			const Color BG_COLOR_2 = IntToColor(0x773344);

			const Color BG_COLOR_MID = ColorLerp(BG_COLOR_1, BG_COLOR_2, 0.5);
			DrawRectangleGradientEx(
					rect,
					BG_COLOR_1, BG_COLOR_MID, BG_COLOR_MID, BG_COLOR_2
			);

			BeginMode3D(camera);
			BeginShaderMode(shader);

				DrawPlaneXY(Vector3Zero(), (Vector2) { 10.0, 10.0 }, WHITE);

				rlPushMatrix();

				rlTranslatef(0, 0, 1.7);

				rlRotatef(360 * vtime / DURATION, -1, 0, 0);
				rlRotatef(360 * vtime / DURATION, 0, 0, 1);

				DrawCube(Vector3Zero(), 2.0, 2.0, 2.0, WHITE);

				rlPopMatrix();

			EndShaderMode();
			DrawGridXY(10, 1.0f);
			EndMode3D();

		EndDrawing();
		//****************

		// TODO: add a "dry run" mode that skips ffmpeg export and only renders
		// to screen.  Also skip ffmpeg init and cleanup

		//if (iframe == 30)
		if (true)
		{
			// Save frame in memory -- faster than disk
			Image image = LoadImageFromScreen();

			//if (true)
			if (iframe == NFRAMES / 2)
			{
				//const char* outimgi  = TextFormat("%s/%s--%s--%d.png", outdir_img, ME, date, iframe);
				//bool io = ExportImage(image, outimgi);
				bool io = ExportImage(image, outimg);
			}

			int w = image.width;
			int h = image.height;

			// Convert pixel format.  TODO: is yuv420 available in raylib?  Then
			// we could skip an extra transcoding step
			ImageFormat(&image, PIXELFORMAT_UNCOMPRESSED_R8G8B8);
			//ImageFormat(&image, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);

			// Write the raw pixels to the ffmpeg pipe
			if (ffmpeg)
			{
				const int PIXEL_BYTES = 3;
				fwrite(image.data, WIDTH * HEIGHT * PIXEL_BYTES, 1, ffmpeg);
			}
		}

		//if (iframe == NFRAMES + 1)
		if (iframe == NFRAMES)
		//if (iframe == NFRAMES - 1)
		{
			normal_close = true;
			break;
		}
	}

	// raylib de-initialization
	UnloadShader(shader);
	//UnloadRenderTexture(target);
	CloseWindow();
	printf("\n");

	// ffmpeg video cleanup
	if (ffmpeg)
	{
		pclose(ffmpeg);
		printf("\n");
		printf(GREEN_ANSI "Successfully created video \"%s\"\n" RESET_ANSI, outfile);
	}

	if (!normal_close)
	{
		printf(WARNING "Graphics window closed abnormally\n" RESET_ANSI);
	}

	printf(GREEN_ANSI "Finished " ME "\n" RESET_ANSI);
	return 0;
}

