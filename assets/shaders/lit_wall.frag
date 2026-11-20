#version 110

// Normal-mapped wall lit by up to MAX_LIGHTS colored point or spot lights in a single pass.
// The wall faces +z with tangent = +x and binormal = +y, so the normal map is already in world space,
// except that this normal map stores green pointing down the image (DirectX convention), so y is flipped.

#define MAX_LIGHTS 8

uniform sampler2D uBaseMap;
uniform sampler2D uNormalMap;

uniform vec3 uAmbient;
uniform float uSpecular;
uniform float uShininess;

uniform int uLightCount;
uniform vec3 uLightPosition[MAX_LIGHTS];
uniform vec3 uLightColor[MAX_LIGHTS];
uniform float uLightRadius[MAX_LIGHTS];
uniform vec3 uSpotDirection[MAX_LIGHTS];
uniform vec2 uSpotCone[MAX_LIGHTS];  // (cos inner, cos outer); outer <= -1 means point light

varying vec2 vTexcoord;
varying vec3 vWorldPosition;

void main()
{
    vec3 albedo = texture2D(uBaseMap, vTexcoord).rgb;
    vec3 normal = texture2D(uNormalMap, vTexcoord).rgb * 2.0 - 1.0;
    normal = normalize(vec3(normal.x, -normal.y, normal.z));
    vec3 viewDir = vec3(0.0, 0.0, 1.0);  // orthographic camera looking down -z

    vec3 color = uAmbient * albedo;
    for (int i = 0; i < MAX_LIGHTS; ++i) {
        if (i >= uLightCount)
            break;

        vec3 toLight = uLightPosition[i] - vWorldPosition;
        float distance = length(toLight);
        vec3 lightDir = toLight / distance;

        // Smooth falloff that reaches exactly zero at the light radius.
        float attenuation = clamp(1.0 - distance / uLightRadius[i], 0.0, 1.0);
        attenuation *= attenuation;

        if (uSpotCone[i].y > -1.0) {
            float cosAngle = dot(-lightDir, uSpotDirection[i]);
            attenuation *= smoothstep(uSpotCone[i].y, uSpotCone[i].x, cosAngle);
        }

        float diffuse = max(dot(normal, lightDir), 0.0);
        vec3 halfway = normalize(lightDir + viewDir);
        float specular = diffuse > 0.0 ? pow(max(dot(normal, halfway), 0.0), uShininess) * uSpecular : 0.0;

        color += (albedo * diffuse + specular) * uLightColor[i] * attenuation;
    }
    gl_FragColor = vec4(color, 1.0);
}
