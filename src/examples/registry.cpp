#include "example.h"

// Factories, one per file in src/examples/.
std::unique_ptr<Example> createBumpMapping();
std::unique_ptr<Example> createDynamicLights();
std::unique_ptr<Example> createSpotlight();
std::unique_ptr<Example> createFire();
std::unique_ptr<Example> createWater();

const std::vector<ExampleInfo>& allExamples()
{
    static const std::vector<ExampleInfo> examples = {
        {"bump", "Bump mapping",
         "The original demo: one quad, three static point lights, tangent-space normal mapping "
         "with a GLSL 1.10 shader. Each light is a separate pass, added with additive blending.",
         createBumpMapping},
        {"lights", "Dynamic lights",
         "Up to seven colored point lights fly over a normal-mapped stone wall, plus one light that follows "
         "the mouse. All lights are evaluated in a single shader pass with uniform arrays.",
         createDynamicLights},
        {"spotlight", "Spotlight",
         "A flashlight in front of a dark wall, aimed at the mouse cursor. Shows spot cone falloff "
         "and how the beam stretches into an ellipse at grazing angles.",
         createSpotlight},
        {"fire", "Fire",
         "A CPU particle system: flames, smoke and sparks drawn as additive and alpha-blended sprites, "
         "with a flickering light that illuminates the wall behind. Drag to move the fire.",
         createFire},
        {"water", "Water ripples",
         "A height-field wave simulation over the stone floor. Normals from the height field refract the "
         "floor texture and add specular highlights. Click or drag to make waves.",
         createWater},
    };
    return examples;
}
