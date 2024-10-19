
#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Output fragment color
out vec4 finalColor;

uniform float vtime;  // virtual time

const float pi = 3.14159265359;

float normal_dist(float x, float mu, float sigma)
{
	float xmu = x - mu;
	float sigma2 = sigma * sigma;
	return 1.f / sqrt(2 * pi * sigma2) * exp(-xmu * xmu / (2 * sigma2));
}

float triangle_dist(float x, float mu, float sigma)
{
	float xmu = x - mu;
	return max((1.f - abs(xmu)) / sigma, 0.f);
}

vec4 get_color(int hex)
{
	int r = (hex / 256 / 256) % 256;
	int g = (hex / 256      ) % 256;
	int b = (hex            ) % 256;
	return vec4(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);
}

float wrap(float x, float xmin, float xmax)
{
	if (x < xmin)
	    x = xmax - mod(xmin - x, xmax - xmin);
	else
	    x = xmin + mod(x - xmin, xmax - xmin);
	return x;
}

void main()
{
	// Transform to range [-1, 1] ** 2 with y up
	float x =  (fragTexCoord.x - 0.5f) * 2.f;
	float y = -(fragTexCoord.y - 0.5f) * 2.f;

	const int NLAYERS = 10;

	//int palette[NLAYERS+1] = int[](0xCAF0F8, 0x90E0EF, 0x00B4D8, 0x0077B6, 0x03045E, 0xCAF0F8);
	//int palette[NLAYERS+1] = int[](0x03045E, 0x0077B6, 0x00B4D8, 0x90E0EF, 0xCAF0F8, 0x03045E);
	//int palette[NLAYERS+1] = int[](0xD8E2DC, 0xFFFFFF, 0xFFCAD4, 0xF4ACB7, 0x9D8189, 0xD8E2DC);
	//int palette[NLAYERS+1] = int[](0x0A9396, 0x94D2BD, 0xE9D8A6, 0xEE9B00, 0xCA6702, 0xBB3E03);

	//int palette[NLAYERS+1] = int[](0xCDB4DB, 0xFFC8DD, 0xFFAFCC, 0xBDE0FE, 0xA2D2FF, 0xCDB4DB);
	//int palette[NLAYERS+1] = int[](0xA2D2FF, 0xCDB4DB, 0xFFC8DD, 0xFFAFCC, 0xBDE0FE, 0xA2D2FF);
	//int palette[NLAYERS+1] = int[](0xCDB4DB, 0xFFC8DD, 0xFFAFCC, 0xBDE0FE, 0xA2D2FF);

	//int palette[NLAYERS] = int[](0xCAF0F8, 0x90E0EF, 0x00B4D8, 0x0077B6, 0x03045E);
	//int palette[NLAYERS] = int[](0xA2D2FF, 0xCDB4DB, 0xFFC8DD, 0xFFAFCC, 0xBDE0FE);
	//int palette[NLAYERS] = int[](0x03071E, 0x370617, 0x6A040F, 0x9D0208, 0xD00000, 0xDC2F02, 0xE85D04, 0xF48C06, 0xFAA307, 0xFFBA08);
	int palette[NLAYERS] = int[](0x012A4A, 0x013A63, 0x01497C, 0x014F86, 0x2A6F97, 0x2C7DA0, 0x468FAF, 0x61A5C2, 0x89C2D9, 0xA9D6E5);

	float z = 10 * y + 2 * sin(4 * x + x*x - 2*pi*vtime/2.5 + sin(y + 2*pi*vtime/5));

	//int iz = int(floor(z)) % NLAYERS;
	//int iz = int(floor(z % NLAYERS));
	int iz = int(wrap(z, 0, NLAYERS));

	finalColor = get_color(palette[iz]);
}

