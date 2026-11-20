#version 110

// Wall in the z = 0 plane of the 2D world. Texture coordinates come from the world position,
// so the stone texture tiles at a fixed world size regardless of the window aspect ratio.

uniform float uTileSize;

varying vec2 vTexcoord;
varying vec3 vWorldPosition;

void main()
{
    vWorldPosition = gl_Vertex.xyz;
    vTexcoord = gl_Vertex.xy / uTileSize;
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
