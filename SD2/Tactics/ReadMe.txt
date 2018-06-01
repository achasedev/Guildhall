SD2 C27 (A3) - Tactics POCG
Author: Andrew Chase

-----Known Issues/Changes-----

I purposely allowed camera rotation around the target of interest (actor or cursor) at all times, to be able
to look around whenever I wanted. This helped with development, and felt better overall.

-----How to Use-----

Run the .exe or build the solution to start the game. Control can be done with keyboard or controller.

	
-----Game Controls-----

    Keyboard input:

        Arrow Keys:	Controls menus if in a menu selection state. 

        Spacebar: Confirms a selection.
        ESC Key: Cancels.  If in a selection phase, returns you to the menu.  If in a menu, moves the selection to WAIT.  

		~ Key - Toggles the Developer Console between open/close

		F8 - Captures the screen and saves it as a png image at the location "Data/Screenshots/screenshot.png"
			 This creates the Screenshots directory if it doesn't already exist.



    Controller Input:

    -	Left Thumbstick:  Controls menus if in a menu selection state, and moves the selection tile cursor during target selection. 
    -   Directional Pad:  Also controls the menu and selection tile, the same as the left thumbstick.

	-	Right Thumbstick:  During target selection, rotates the camera.
	-	A:  Confirms a selection
	-	B:  Cancels.  If in a selection phase, returns you to the menu.  If in a menu, moves the selection to WAIT.  
	-	Right Bumper:  Rotates to the nearest inter-cardinal direction to the right
	-	Left Bumper:  Rotates to the nearest inter-cardinal direction to the left
	-	Right Trigger:  Zoom In
	-	Left Trigger:  Zoom Out



-----Extras-----

I implemented 4 extras for this build:

	E03.10 :: Screenshot functionality - Press F8 to take a screenshot during run time, see Renderer::SaveScreenshotToFile() for implementation (Renderer.cpp, line 761)

	E03.20 :: X-ray Selection Tiles - Notice that the cursor, attack and move tiles are rendered differently when out of view. See GameState_Playing::RenderTiles() for implementation,
									  (GameState_Playing.cpp, line 752) 

	E03.30 :: Fixed Camera Angles - Press LB/RB on the controller to rotate the camera around to the inter-cardinal world angles
									See GameState_Playing::UpdateCameraOnInput() (GameState_Playing.cpp, line 278) for implementation						

	E03.40 :: Block Borders	- 		All blocks are rendererd by mixing two textures, a color texture and a border texture. This is done in the Block.vs/Block.fs shader files.
									See Map::Render() (Map.cpp, line 102) for where it is drawn, and the shader files in Data/Shaders/ for the shader code.


-----Closing-----

If you have any questions or concerns email me at amchase@smu.edu and I'll do my best to accommodate!

Thanks!