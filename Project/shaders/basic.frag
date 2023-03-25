#version 410 core

in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoords;
in vec4 fragPosLightSpace;

out vec4 fColor;

//matrices
uniform mat4 model;
uniform mat4 view;
uniform mat3 normalMatrix;
//lighting
uniform vec3 lightDir;
uniform vec3 lightColor;
uniform mat3 lightDirMatrix;
// textures
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shadowMap;
// For fog functionality
uniform int activateFog;
// For Spotlight
uniform int spotLight;
uniform vec3 direction;
uniform vec3 position;
uniform float cutOff;
uniform float outerCutOff;

//components
vec3 ambient;
uniform float ambientStrength = 0.2f;
vec3 diffuse;
vec3 specular;
uniform float specularStrength = 0.5f;
uniform float shininess = 30.0f;

// For shadow functionality
float shadow;

void computeDirLight()
{
    vec3 cameraPosEye = vec3(0.0f);	//in eye coordinates, the viewer is situated at the origin

    //compute eye space coordinates
    vec4 fPosEye = view * model * vec4(fPosition, 1.0f);
    vec3 normalEye = normalize(normalMatrix * fNormal);

    //compute light direction
	vec3 lightDirN = normalize(lightDirMatrix * lightDir);	

	//compute view direction 
	vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);
	
	//compute half vector
	vec3 halfVector = normalize(lightDirN + viewDirN);

    //compute ambient light
    ambient = ambientStrength * lightColor;

    //compute diffuse light
    diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;

    //compute specular light
    vec3 reflectDir = reflect(-lightDirN, normalEye);
    float specCoeff = pow(max(dot(normalEye, halfVector), 0.0f), shininess);
    specular = specularStrength * specCoeff * lightColor;
}

float computeShadow()
{
    // perform perspective divide
    vec3 normalizedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    if (normalizedCoords.z > 1.0f) {
        return 0.0f;
    }
    // Transform to [0,1] range
    normalizedCoords = normalizedCoords * 0.5 + 0.5;
    // Get closest depth value from light's perspective
    float closestDepth = texture(shadowMap, normalizedCoords.xy).r;
    // Get depth of current fragment from light's perspective
    float currentDepth = normalizedCoords.z;
    // Check whether current frag pos is in shadow
    float bias = 0.0005f;
    float shadow = currentDepth - bias > closestDepth ? 1.0f : 0.0f;

    return shadow;
}

float computeFog()
{
	float fogDensity = 0.005f;
	float fragmentDistance = length(fPosition);
	float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));

	return clamp(fogFactor, 0.0f, 1.0f);
}

void main() 
{
    computeDirLight();

    //modulate with diffuse map
	ambient *= vec3(texture(diffuseTexture, fTexCoords));
	diffuse *= vec3(texture(diffuseTexture, fTexCoords));
	//modulate woth specular map
	specular *= vec3(texture(diffuseTexture, fTexCoords));

    vec4 colorFromTexture = texture(diffuseTexture, fTexCoords);
    if(colorFromTexture.a < 0.1)
        discard;

    //modulate with shadow
    shadow = computeShadow();

    if (spotLight == 1)
    {
        vec4 fPosEye = view * model * vec4(fPosition, 1.0f);
		vec3 lightDirection = normalize(position - fPosEye.xyz);
		float theta = dot(lightDirection, normalize(-direction));
		float epsilon = (cutOff - outerCutOff);
		float intensity = clamp((theta - outerCutOff) / epsilon, 0.0, 1.0);
		diffuse *= intensity;
		specular *= intensity;
	}

    //compute final vertex color
    vec3 color = min((ambient + (1.0f - shadow) * diffuse) + (1.0f - shadow) * specular, 1.0f);
    
    if (activateFog == 1)
    {
	    float fogFactor = computeFog();
	    vec4 fogColor = vec4(0.7f, 0.7f, 0.7f, 1.0f);
        vec4 colorF = vec4(color, 1.0f);
	    fColor = fogColor * (1 - fogFactor) + colorF * fogFactor;
	}
    else {
	    fColor = vec4(color, 1.0f);
	}
}
