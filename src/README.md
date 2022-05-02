# LOGS & RESOURCES

### RESOURCES

**DirectX**
* Beginning DirectX 11 Game Programming
* Physics Modeling for Game Programmers (DirectX 9)
* https://www.3dgep.com/introduction-to-directx-11/

**Orthographic Projection**
* https://docs.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixorthorh?redirectedfrom=MSDN
* https://blog.demofox.org/2017/03/31/orthogonal-projection-matrix-plainly-explained/
* https://docs.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixorthooffcenterrh

**Packing Matrices (Column Vs Row Order)**
* https://en.wikipedia.org/wiki/Row-_and_column-major_order

**Spatial Partitioning**
* https://www.cs.cmu.edu/~mbz/personal/graphics/obj.html
* file:///C:/Users/mAmaro/OneDrive/Books/Computer%20Science/Games%20&%20Simulation/Space-filling%20curves%20an%20introduction%20with%20applications%20in%20scientific%20computing%20(%20PDFDrive%20).pdf
* https://www.youtube.com/c/Mr4thProgramming/videos
* https://en.wikipedia.org/wiki/Binary_space_partitioning
* https://en.wikipedia.org/wiki/Adaptive_mesh_refinement
* https://en.wikipedia.org/wiki/Quadtree

**Text Layout/FreeType**
* https://stackoverflow.com/questions/66265216/how-is-freetype-calculating-advance


### LOG

**NOTE(MIGUEL): (04/08/2022)**

I want to implement a quad tree but theres a few quesetions that i dont know.
Forexamle this is a spatial query so what units should i choose for it? Meters no?
There renderer should a be in charge of conversion to pixels... So the what are the
funtios supposed to do exactly. Ill read about it. What will be the starting grid size be?
I want to see the quad tree work with few entites a so relativelly about the half the size of the 
monitor with a low (about 2) entity cap per grid.


**NOTE(MIGUEL): (04/28/2022)**

I changed the the units of entities to meter with a conversion ratio that i use in the renderer.
Speed and time are not rigourously defined yet. I'm now working on text rendering using freetype.
It should be basic without the use of a cache. The goals is to output basic things like time, delta time,
and later for introspection. I'm reusing code from drone controller project. Also i should be more attentive when
swaping parameters of critical and Heavily used funcitons like MemCopy so that prams in calls are all
updated it could be a huge time sink. Fixed the problem where entities jump over walls when doing a drag and
hold window resize for to long. Dont keep functions that hand os events like window resize in timed sections
of code. 

**NOTE(MIGUEL): (04/29/2022)**

Work on text rendering!!! Buffers are mememory resources that are mean to be used in a a flexible generic way
and textures are memory resources intented to be used with rendering in mind. Texures all have the concept of a
pixel element and they may also have more hardware functionality assoceiated. Im thinking D3DX 2d texture resource containing
an array of 2d texture subresource where each slice is glyp bitmap. Textures are being dispatched to the gpu correctly but 
they are not being rendered on to the screen.

**NOTE(MIGUEL): (05/01/2022)**

Tried to get d3d11 to render text glyph texutures onto my sprite but now quads wont render at all.
Goal is to debug that before moving forward with text glyph stuff. Lookes at renderdoc on why quads wont
render and nothing stook out. Coords seem reasonable but mesh overlay doesnt show up on texture viewer. 
Ill comment out text render calls and first mega quad used of backdrop to try to isolate the problem.