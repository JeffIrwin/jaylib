
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

	////float sigma = 1.f;
	//float sigma = sqrt(0.2f);

	//float mu0 = -1.9f;
	float mu0 = 0.f;

	//float mu = 0.f;
	//float mu = mu0 + vtime;
	float mu = mu0;

	//float y_thresh = normal_dist(x, mu, sigma) - 0.5f;
	//float y_thresh = normal_dist(x, mu, sigma) - 0.99f;

	const int NLAYERS = 4;

	//float y_trans[NLAYERS] = float[](-0.99f, -0.8f, -0.6f, -0.4f);
	float y_trans[NLAYERS] = float[](-0.9f, -0.6333f, -0.3667f, -0.1f);

	float x_trans[NLAYERS] = float[](0.f, 2.0f, 0.0f, 1.f);
	float speeds [NLAYERS] = float[](1.f, -0.8f ,  0.6f, -0.4f);
	float sigmas [NLAYERS] = float[](0.45f, 0.35f, 0.55f, 0.425f);

	//int palette[NLAYERS] = int[](0x222244, 0x666688, 0xaaaacc);
	int palette[NLAYERS+1] = int[](0xCDB4DB, 0xFFC8DD, 0xFFAFCC, 0xBDE0FE, 0xA2D2FF);

	//bool above = y > x;
	//bool above = y > y_thresh;

	for (int i = 0; i < NLAYERS; i++)
	{
		float y_thresh = normal_dist(
				//x - x_trans[i] - speeds[i] * vtime,
				//x - speeds[i] * vtime - x_trans[i],
				//mod(x - speeds[i] * vtime - 3.f, 6.f) - x_trans[i] + 3.f,
				//mod(x - x_trans[i] - speeds[i] * vtime - 3.f, 6.f) + 3.f,
				//wrap(x - speeds[i] * vtime, -3.f, 3.f) - x_trans[i],
				wrap(x - speeds[i] * vtime - x_trans[i], -2.f, 2.f),
				mu,
				sigmas[i])
			+ y_trans[i];

		bool below = y < y_thresh;
		if (below)
		{
			finalColor = get_color(palette[i]);
			return;
		}
	}
	finalColor = get_color(palette[NLAYERS]);

	//if (above)
	//{
	//	//finalColor = vec4(0.5f, 0.5f, 0.9f, 1.f);
	//	finalColor = get_color(0x8888ee);
	//}
	//else
	//{
	//	//finalColor = vec4(0.1f, 0.1f, 0.2f, 1.f);
	//	finalColor = get_color(0x222244);
	//}
}

