
#version 330


// Input vertex attributes (from vertex shader)
in vec3 fragPosition;
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;

// Input uniform values
uniform sampler2D texture0;
//uniform vec4 colDiffuse;

// Output fragment color
out vec4 finalColor;

// NOTE: Add here your custom variables

#define     MAX_LIGHTS              4
#define     LIGHT_DIRECTIONAL       0
#define     LIGHT_POINT             1

struct Light {
    int enabled;
    int type;
    vec3 position;
    vec3 target;
    vec4 color;
};

// Input lighting values
uniform Light lights[MAX_LIGHTS];
uniform vec4 ambient;
uniform vec3 viewPos;

void main()
{
    // Texel color fetching from texture sampler
    vec4 texelColor = texture(texture0, fragTexCoord);
    vec3 lightDot = vec3(0.0);
    vec3 normal = normalize(fragNormal);
    vec3 viewD = normalize(viewPos - fragPosition);

    //vec3 specular = vec3(0.0);
    vec3 specular = vec3(0.1);

    // NOTE: Implement here your fragment shader code

    for (int i = 0; i < MAX_LIGHTS; i++)
    {
        if (lights[i].enabled == 1)
        {
            vec3 light = vec3(0.0);

            if (lights[i].type == LIGHT_DIRECTIONAL)
            {
                light = -normalize(lights[i].target - lights[i].position);
            }

            if (lights[i].type == LIGHT_POINT)
            {
                light = normalize(lights[i].position - fragPosition);
            }

            float NdotL = max(dot(normal, light), 0.0);
            lightDot += lights[i].color.rgb*NdotL;

            float specCo = 0.0;
            float shine = 80.0;
            if (NdotL > 0.0) specCo = pow(max(0.0, dot(viewD, reflect(-(light), normal))), shine);
            specular += specCo;
        }
    }

    //finalColor = (texelColor*((colDiffuse + vec4(specular, 1.0))*vec4(lightDot, 1.0)));
    finalColor = (texelColor*((fragColor + vec4(specular, 1.0))*vec4(lightDot, 1.0)));

    //finalColor += texelColor*(ambient/10.0)*colDiffuse;
    finalColor += texelColor*(ambient/10.0)*fragColor;

    //// Gamma correction
    ////finalColor = pow(finalColor, vec4(1.0/2.2));
    //finalColor = pow(finalColor, vec4(1.0/1.2));
}



//in vec3 fragNormal;
//in vec3 fragPosition;
//in vec4 fragColor;
//
//in float fragTexCoord;
//
//out vec4 finalColor;
//
////uniform vec3 u_light;
////uniform sampler1D tex;
//uniform vec3 viewPos;
//
//struct Light {
//	int enabled;
//	int type;
//	vec3 position;
//	vec3 target;
//	vec4 color;
//};
//#define     MAX_LIGHTS              4
//
//uniform Light lights[MAX_LIGHTS];
//
//// Some of these parameters, like specular color or shininess, could be
//// moved into uniforms, or they're probably fine as defaults
//
//const vec4 specular_color = vec4(0.1, 0.1, 0.1, 1.0);
//
////vec4 diffuse_color = texture(tex, v_tex_coord);
////vec4 ambient_color = diffuse_color * 0.1;
//
//void main()
//{
//	vec4 diffuse_color = fragColor;
//	vec4 ambient_color = diffuse_color * 0.1;
//
//	// Point-light only
//	vec3 light = normalize(lights[0].position - fragPosition);
//
//	float diffuse =
//			max(dot(normalize(fragNormal), light), 0.0);
//
//	//vec3 camera_dir = normalize(-fragPosition);
//	vec3 camera_dir = normalize(viewPos - fragPosition);
//	vec3 half_dir = normalize(light + camera_dir);
//
//	float specular =
//			pow(max(dot(half_dir, normalize(fragNormal)), 0.0), 20.0);
//
//	vec4 color = ambient_color +  diffuse *  diffuse_color
//	                      + specular * specular_color;
//
//	finalColor = color;
//}
//
