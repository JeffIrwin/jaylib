
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

vec4 get_color(int hex)
{
	int r = (hex / 256 / 256) % 256;
	int g = (hex / 256      ) % 256;
	int b = (hex            ) % 256;
	return vec4(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);
}

void main()
{
	// Transform to range [-1, 1] ** 2 with y up
	float x =  (fragTexCoord.x - 0.5f) * 2.f;
	float y = -(fragTexCoord.y - 0.5f) * 2.f;

	//float sigma = 1.f;
	float sigma = sqrt(0.2f);

	//float mu0 = -1.9f;
	float mu0 = -1.f;

	//float mu = 0.f;
	float mu = mu0 + vtime;

	//float y_thresh = normal_dist(x, mu, sigma) - 0.5f;
	float y_thresh = normal_dist(x, mu, sigma) - 0.99f;

	//bool above = y > x;
	bool above = y > y_thresh;

	if (above)
	{
		//finalColor = vec4(0.5f, 0.5f, 0.9f, 1.f);
		finalColor = get_color(0x8888ee);
	}
	else
	{
		//finalColor = vec4(0.1f, 0.1f, 0.2f, 1.f);
		finalColor = get_color(0x222244);
	}
}

