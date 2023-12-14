# Hyacinth Labyrinth

## Idea
Procedurally generated maze with procedurally generated innards with ball physically moving around it

## Base goals
#### Procedurally generate a maze
- Start with square tiling that are either open or closed
  - Open spaces can be moved through
  - Closed spaces have decoration
- Second level generation to place objects in appropriate spaces
  - E.g. 2x2 pond
  - Water for reflections
- Outputs a string of some sort that represents the tiling

#### Procedurally generate decoration in the maze
- L-system trees and foliage (trimeshes)
- Walls

#### Move a ball
- Collision detection with wall

#### Real-time ray tracing for reflections and shadows on decorating surfaces
- Only selectively used for specific objects

## Stretch Goals
- Fancy maze generation
- Seasonal trees


####  Notes
- Third person view of ball from ~45˚ up
- Add minimap?
- How to generate the “walls” of the maze
  - Bump mapping runs into problems of ugliness
  - Generate actual geometry
    - Makes collisions harder
    - Maybe add a “fence” (real or fake)
  - Different kinds of wall
     - Thick bushes
     - Fence with trees behind it
     - Wall
- Generate maze based on ball position
  - Keep the stuff near the ball the same, but regenerate things farther away
  - Fog to hide stuff
- Stuff in the maze
  - Water? Pond?
  - Trees
    - 4 season theme with different variations
    - Start+end cycle 
Ball movement
- Instead of ball moving around, move the whole maze plane (e.g. camera and gravity direction)


## Submission notes:
### External dependencies
- The base Vulkan system was repurposed from https://github.com/blurrypiano/littleVulkanEngine, on top of which we added support for texture mapping.
- For loading obj files and textures, we use https://github.com/tinyobjloader/tinyobjloader
- For loading texture images we use https://github.com/nothings/stb

### Existing features
-  Random (but solvable) maze generation using Wilson's algorithm
-  Lsystems for offline generation of foliage geometry and textures
-  Physics simulation of a ball within the maze
-  Real-time rendering using Vulkan
