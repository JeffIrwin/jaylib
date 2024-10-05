
#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Output fragment color
out vec4 finalColor;

uniform vec2 c;       // c.x = real, c.y = imaginary component. Equation done is z^2 + c
uniform vec2 offset;  // Offset of the scale.
uniform float zoom;   // Zoom of the scale.
uniform float vtime;  // virtual time

const int maxIterations = 255;     // Max iterations to do.
const float colorCycles = 2.0f;    // Number of times the color palette repeats. Can show higher detail for higher iteration numbers.

const float pi = 3.14159265359;

// Square a complex number
vec2 ComplexSquare(vec2 z)
{
    return vec2(
        z.x*z.x - z.y*z.y,
        z.x*z.y*2.0f
    );
}

// Convert Hue Saturation Value (HSV) color into RGB
vec3 Hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0f, 2.0f/3.0f, 1.0f/3.0f, 3.0f);
    vec3 p = abs(fract(c.xxx + K.xyz)*6.0f - K.www);
    return c.z*mix(K.xxx, clamp(p - K.xxx, 0.0f, 1.0f), c.y);
}

float normal_dist(float x, float mu, float sigma)
{
	float xmu = x - mu;
	float sigma2 = sigma * sigma;
	return 1.f / sqrt(2 * pi * sigma2) * exp(-xmu * xmu / (2 * sigma2));
}

void main()
{
    /**********************************************************************************************
      Julia sets use a function z^2 + c, where c is a constant.
      This function is iterated until the nature of the point is determined.

      If the magnitude of the number becomes greater than 2, then from that point onward
      the number will get bigger and bigger, and will never get smaller (tends towards infinity).
      2^2 = 4, 4^2 = 8 and so on.
      So at 2 we stop iterating.

      If the number is below 2, we keep iterating.
      But when do we stop iterating if the number is always below 2 (it converges)?
      That is what maxIterations is for.
      Then we can divide the iterations by the maxIterations value to get a normalized value that we can
      then map to a color.

      We use dot product (z.x * z.x + z.y * z.y) to determine the magnitude (length) squared.
      And once the magnitude squared is > 4, then magnitude > 2 is also true (saves computational power).
    *************************************************************************************************/

    // The pixel coordinates are scaled so they are on the mandelbrot scale
    // NOTE: fragTexCoord already comes as normalized screen coordinates but offset must be normalized before scaling and zoom
    vec2 z = vec2((fragTexCoord.x - 0.5f)*2.5f, (fragTexCoord.y - 0.5f)*1.5f)/zoom;
    z.x += offset.x;
    z.y += offset.y;

	// y up
	z.y *= -1;

	//float sigma = 1.f;
	float sigma = sqrt(0.2f);

	float mu0 = -1.9f;

	//float mu = 0.f;
	float mu = mu0 + vtime;

	//float xmu = z.x - mu;
	//float y_thresh = 1.f / sqrt(2 * pi * sigma * sigma) * exp(-xmu * xmu / (2 * sigma * sigma));

	//float y_thresh = 2 * normal_dist(z.x, mu, sigma) - 0.5f;
	float y_thresh = normal_dist(z.x, mu, sigma) - 0.5f;

	//bool above = z.y > z.x;
	bool above = z.y > y_thresh;

	if (above)
		finalColor = vec4(0.5f, 0.5f, 0.9f, 1.f);
	else
		finalColor = vec4(0.1f, 0.1f, 0.2f, 1.f);
	return;

    int iterations = 0;
    for (iterations = 0; iterations < maxIterations; iterations++)
    {
        z = ComplexSquare(z) + c;  // Iterate function

        if (dot(z, z) > 4.0f) break;
    }

    // Another few iterations decreases errors in the smoothing calculation.
    // See http://linas.org/art-gallery/escape/escape.html for more information.
    z = ComplexSquare(z) + c;
    z = ComplexSquare(z) + c;

    // This last part smooths the color (again see link above).
    float smoothVal = float(iterations) + 1.0f - (log(log(length(z)))/log(2.0f));

    // Normalize the value so it is between 0 and 1.
    float norm = smoothVal/float(maxIterations);

    // If in set, color black. 0.999 allows for some float accuracy error.
    if (norm > 0.999f) finalColor = vec4(1.f, 1.f, 1.f, 1.f);
    //else finalColor = vec4(Hsv2rgb(vec3(norm*colorCycles, 1.0f, 1.0f)), 1.0f);
    else finalColor = vec4(Hsv2rgb(vec3(1.0f - norm*colorCycles, 1.0f, 1.0f)), 1.0f);
}

