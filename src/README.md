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


**NOTE(MIGUEL): (05/02/2022)**

I'm currently including and compiling freetype on the dll which can get reloaded. The app crashes on dll reload
when calling freetype functions. I'll move this to the exe side but it would be nice to understand better the reason
behind the crash... Just for basics i want to load the ascii range of characters maybe using a temp arena and do the 
mapping into d3d11 2d textures. As long as i initialize free type once when reloading a the dll everything is fine.
I dont want glyhs to be loaded every frame.
I want the Ansi range of charters for now.
I want full control over font loading from the dll.
I don't duplicate bitmaps for chars.

**NOTE(MIGUEL): (05/02/2022)**

Enough of the text layout is done. Now ill focus on other stuff and let any flaws expose themselves and 
fix accordinglly. I have choice between implementing quad trees, a camera system, or whatever. Ill work on
some elementary debug services and UI using hamdmade hero.


**NOTE(MIGUEL): (05/07/2022)**

Found the source of a pretty obvious (but not really crash) caused by macros that im useing for counting cycles
taken by blocks of code. I have a global defined in a dll header with the static keyword. It just holds a pointer 
to appstate but that pointer gets initialized  in the dll which may not even load. lol However my main concern is if
it actually global... The dll is compiled as a seperate unit meant to be linked at run time. When the executable links it
need to fetch pointers to funcitons to be able to use them. That is the only way. Functions are extern by default yet you still
have to go throug that process. So it's kind of dumb to expect that declaring a variable as extern is going to magically make
some memory address that is shared between the dll and the exe. Also Declaring a variable that (extern or not) in a header that 
is viewed by both the exe src and dll src will just result in both the dll and exe haveing their own respective versions of 
that variable. Therefore i can assign the appstate ptr some memory in a dll function but exe wont know because it has it's own
appstate ptr which is completely seperate and still null. hence the crash.


**NOTE(MIGUEL): (05/32/2022)**

I'm going to implement some input into the programa and use HMH and Ryan's UI Articles to get some simple UI going.


**NOTE(MIGUEL): (06/16/2022)**

How is the widget hierarchy being built? Is it relying on the parent stack. How are siblings added? If there is a parent always accessible via parent stack then surley i can use the last child tree link for appending siblings conveinitnly. Of course the widget cache/hashtable need to to be in persitant memory to persist accross frame boundries. I have an idea of doing a lookup into the widget cache to view the state of a widghet on the previous frame but what is the policy on adding a new widget to the cache. Do they all get cached?? Before i could have a single layout object be shared by several buttons which are just funcion calls. layout struct evolved into the widget struct hold crossframe data for each widget as well as layout data. And now seems like there needs to be a widget data struct for each button? Is that understanding correct?