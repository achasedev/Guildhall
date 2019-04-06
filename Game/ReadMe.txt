C27 SD4 - SuperMiner
A4 - Camera, Physics and Collision
Author: Andrew Chase

-----Known Issues/Changes-----

Ensure the startup project is the game project, and not the engine project. Otherwise it will not startup.

-----How to Use-----

Run the .exe or build the solution to start the game.

-----Game Controls-----

Use WASD to move around on the horizontal XY plane.
Use QE to move up and down along the vertical z axis.
Hold Shift to move faster, or hold space to move more slowly.

Rotate the camera using mouse look.

Press Left Click when looking at a block to dig the block.
Press Right Click when looking at a block face to place a stone block along that face.
Press and hold Control and Right click to place a glowstone block.

Press 'U' to immediate deactivate all chunks (they will immediately reactivate in activation order)
Press 'R' to lock the raycast and draw a debug raycast for testing. Press 'R' again to unlock the raycast.
Press 'Y' to toggle the raycast method from Step-and-Sample (slow) to Intercepts (fast)
Press and hold 'T' to increase the scale to 10000x real time.

Press 'F2' to toggle the camera modes:
 1. Third person - You will control the entity from an over-the-shoulder view
  - WASD moves the entity relative to their own transfrom
  - If Right click is *NOT* held:
   - The camera will orbit around the player, not affecting the player
  - If right click is held:
   - The player will rotate and face the direction the camera is facing. Continuing to hold right click keeps them looking in the same direction
  - Mouse wheel will zoom the camera in and out

 2. First-Person
  - Controls the player from a first-person view with standard controls

 3. Fixed-angle
  - Player will still be controlled, moving relative to its transform with WASD
  - Moving the mouse will change the direction the entity is facing
  - Mouse wheel will zoom the camera in and out
  - The camera angle is fixed

 4. Detached
  - The camera will detach from the player and fly freely with no physics or collision

Press 'F3' to toggle physics modes on the player:
 1. WALKING - the player will fall from gravity, and have collision with the blocks in the chunk
 2. FLYING - the player can fly freely with no gravity, but will still have collision
 3. NO CLIP - the player can fly freely with no gravity or collision with the blocks in the chunk

-----Extras-----

-----Closing-----

If you have any questions or concerns email me at amchase@smu.edu.

Thanks!