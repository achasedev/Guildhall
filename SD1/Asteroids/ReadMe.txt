SD1 C27 A3 - Asteroids Gold
Author: Andrew Chase

-----Known Issues-----

I added homing missiles as "juice" mechanic, however due to the wrap around feature of the
game they tend to take a while to reach their mark, usually by circling their target a few
times. This isn't distinctive, but may be noticeable if you shoot a few missiles and see
which target they go for. I found that disabling wraparound on the missiles helps them
to track their target better, so at the moment missiles do not wrap around.

I tested all other features and mechanics of my game, and found no serious issues. I was
worried about the consequences of unplugging or plugging in the controller while the game was
running, so I tweaked my InputSystem to ensure that there would be no side effects, and so
far I haven't observed any.

One minor issue that does exist is that you cannot toggle the debug rendering while the game is
paused. This is because I chose to not call g_theGame->Update() at all when the game is paused,
instead of calling g_theGame->Update(deltaTime) with deltaTime == 0. This may restrict some
implementation later in the game, and I could switch it over, but for now I didn't want to
introduce any issues this late in the assignment, and instead focus on what needed to be completed.

Another minor issue is that the flame render still flickers when the game is paused, both 
on the missiles and on the ship. Furthermore, pressing the accelerate input when the game is paused
will still render the flame on the ship.


-----How to Use-----

This program works exactly as specified in the assignment documentation for the required items. 
There is a pre-built Asteroids.exe in the Win_32 folder, or you can compile/build the project 
in Visual Studio and test it there. Both the debug builds and the release builds have been tested and work

The ship still spawns in the center of the screen, and waves of asteroids spawn off screen and move
randomly in the space. In addition, a blackhole will spawn on wave 1 (and one additional one for every
wave afterward), which will pull any asteroid, bullet, missile, or the player when near it.

The player dies when they touch an asteroid, but they can respawn with the 'N' key. Whenever a player
respawns, they have three seconds of invincibility, which is indicated with the ship flashing.


-----Game Controls-----

    Keyboard input:

        ESC Key - closes the game

        F1 - Render Debug information on all Entities

        Up Arrow/'E' - Thrust the ship forward in the direction it is facing

        Left Arrow/'S' - Rotate the ship counter-clockwise

        Right Arrow/'F' - Rotate the ship clockwise

        Spacebar - Fires a bullet out of the ship

        'I' - Spawns a large asteroid off screen

        'O' - Deletes a random asteroid

        'P' - Pauses/unpauses the game

        'T' - Slows the game time to 1/10th the speed

        'N' - Respawns the ship if the player died

        'B' - Spawns a Black Hole off screen

        'M' - Fires a homing missile out of the ship

    Controller input: (For realism I only put gameplay controls on the controller, and no developer functionality)

        Left Stick - Moves the ship
                     The ship automatically faces the direction the stick is pressed, and
                     accelerates based on the corrected magnitude of the stick

        'A' button - Fires a bullet out of the ship

        'X' button - Fires a homing missile out of the ship

        Start button - Respawns the ship if the player has died

        Back button - Pauses/unpauses the game

 
-----"Juice" features-----

In addition to the required mechanics I added:

    - Colored Entities, and altered bullet shape

    - Ship invincibility when it respawns, indicated by the ship flashing. This makes the ship
      pass through asteroids and black holes without any effect.

    - Black holes (large purple spirals), which pull all Entities near them if they get close. One
      black holes is spawned at the beginning, and an additional one is spawned every wave afterward.
      Black holes cannot be destroyed.

    - Homing missiles, which choose a random asteroid as a target and home in on them. Asteroids are automatically
      destroyed by missiles, and do not break into smaller asteroids. Missiles do not wrap around the screen.

    - Ship recoil when firing missiles, slightly pushing the ship backwards

    - Screen shake, which plays when the player dies and when a missile makes impact with an asteroid



-----Deep Learning-----

The biggest insight I had while working on this project is understanding when is best to abstract, and when not to.
I tend to wrap computationally-dense code in functions to make higher level functions more readable, but consequently 
it makes lower level level functions long and overcomplicated. Eventually, I start to hesitate whether to modularize
more code or keep it contained in one area, and thus consistency suffers.

This eventually lead to the insight of how difficult it is to maintain readable code on a large project, and 
how important it is to start the codebase off strong with a good organized foundation. Building habits such as 
commenting as you code as you work, using TODO tags, or even just making the code readable though verbose
function and data member names makes a world of a difference. Ultimately, I'd say that even after one month of 
programming this project, I read my earlier code and understand it as if I was an outside programmer. Sure, there were some 
things I did recall, but I would not have understood why I did what I did without comments and verbose 
variable names to explain. Despite this being a single-person project, it was as if more than one person worked
on it - the programmer I was four weeks ago, and the programmer I was this week. This made me realize
the importance of readability and standards, and how it benefits the developer himself just as much as others.

With that being said, however, I found that I was focusing far too much on the structure, and not on the project requirements.
I often brushed aside the game specifics as "being the easy part" and continued to throw hours into just reworking
the code to be efficient and organized, such as when I was working on my App class and trying to integrate windows.h into the project. 
At the end of the day, however, I didn't have a working project, just one that looked good in the file, while most 
others had a functioning game. Consequently, I found myself cramming towards the end to get the game implementation 
completed, which albeit was easy to do with a good foundation, but nonetheless took me too much time to finish. 
In a world full of deadlines, my approach would always have me pushing to the end.

All in all, it is important to keep a balance between making the structure organized and readable, but at the same time
finishing the requirements and having a finished project.


-----Closing-----

If you have any questions or concerns email me at amchase@smu.edu.

Thanks!