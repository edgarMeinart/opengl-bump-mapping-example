#version 110

// Full-view quad in World2D coordinates.

uniform vec2 uWorldSize;
uniform float uTileSize;

varying vec2 vSurfaceUv;  // 0..1 across the simulation grid
varying vec2 vFloorUv;    // tiled floor texture

void main()
{
    vSurfaceUv = gl_Vertex.xy / uWorldSize;
    vFloorUv = gl_Vertex.xy / uTileSize;
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
