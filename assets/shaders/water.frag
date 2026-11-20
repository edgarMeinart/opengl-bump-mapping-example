#version 110

// Water surface over a stone floor, viewed from straight above.
// uSlopes holds the height-field slope (x in R, y in G) encoded as 0.5 + 0.5 * slope / range.

uniform sampler2D uFloor;
uniform sampler2D uSlopes;
uniform float uSlopeScale;      // decodes the texture and exaggerates the waves
uniform float uRefraction;      // how far the floor image shifts per unit of slope
uniform vec3 uWaterColor;
uniform float uDepthTint;
uniform float uSpecular;
uniform vec3 uLightDirection;   // normalized, pointing toward the light

varying vec2 vSurfaceUv;
varying vec2 vFloorUv;

void main()
{
    vec2 slope = (texture2D(uSlopes, vSurfaceUv).rg * 2.0 - 1.0) * uSlopeScale;
    vec3 normal = normalize(vec3(slope, 1.0));

    // Refraction: the floor appears shifted where the surface is tilted.
    vec3 floorColor = texture2D(uFloor, vFloorUv + normal.xy * uRefraction).rgb;

    // Cheap caustics: surface tilted toward the light focuses more light onto the floor.
    float focus = 1.0 + 2.5 * (dot(normal, uLightDirection) - uLightDirection.z);
    vec3 color = mix(floorColor * focus, uWaterColor, uDepthTint);

    // Specular highlight of the light, seen by a viewer looking down the z axis.
    vec3 reflected = reflect(-uLightDirection, normal);
    color += pow(max(reflected.z, 0.0), 120.0) * uSpecular;

    // Steep slopes reflect more of the sky (a rough Fresnel term).
    float fresnel = pow(1.0 - normal.z, 2.0);
    color += fresnel * vec3(0.55, 0.7, 0.85) * 1.5;

    gl_FragColor = vec4(color, 1.0);
}
