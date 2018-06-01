SD1 C27 A6 - Incursion Gold
- Author: Andrew Chase

-----Known Issues-----

There are no serious issues that I know of. All required mechanics specified in the project
description were implemented correctly, with some added features. 

There are some rare issues with an exception begin thrown from the Map::GetTileFromPosition()
function, as it tries to index into my Tile* vector with what ends up being an invalid index.
I haven't found an efficient way to fix this that doesn't involve adding null checks throughout my entire project
(plus I'm not sure what to do if I ask for a Tile* and it ends up being null), so instead I added a 3-tile thick
solid wall around the map, which seems to stop out of bounds/index issues. I also put in a failsafe where it will return
the first tile of the map should such a case arise.

Otherwise, if for any reason a bullet (or entity for that matter) gets outside of the map, an exception may be thrown.
But during normal gameplay, this issue is non-existant.

I may have has another exception issue, though I haven't seen it happen lately. It might have been to do with
not cleaning up the world/maps correctly, so a function of world/map may be called on a nullptr, but these issues
arised when the app wasn't terminated correctly I believe, and these issues do not appear during normal play.
But I haven't seen this issue in a while, so it may have been addressed/fixed when I fixed a different bug.


-----How to Use-----

This program works exactly as specified in the assignment documentation for the required items. 
There is a pre-built Incursion.exe in the Win_32 folder, or you can compile/build the project 
in Visual Studio and test it there. Both the debug builds and the release builds have been tested and work.

The player tank spawns in the bottom left area of an NxM map of tiles, surrounded by a brick patch of tiles where
no enemies will spawn. The green grass tiles represent grass, which the tank can drive over, and the gray 
rock-looking tiles are stone, which act as solid walls. There are also water tiles, which can be seen/shoot through
but not driven over. Enemies are randomly spawned on valid spawn tiles, which are specified in the TileDefinition class.

You pick up PickUps by driving into them, and move to the next map by driving over the Purple/White teleport square.

Npcs work with a more-advanced AI than what was specified in the assignment document. All NpcTanks will pursue their hostile
target if in line of sight, turning and shooting appropriately. If the target falls out of line of sight, they purse the last target
location, unless a new hostile target is nearby and is in line of sight, to which they switch to that target. Ally NPC tanks will
also default to following the player if no enemy tanks are present.



-----Game Controls-----

    Keyboard input:

        ESC Key - closes the game

        F1 - Render Debug information on all Entities

        F2 - Draws a black grid over the map, to clearly define tile locations

	F3 - While held, displays a bird's eye view of the entire map in realtime

	F4 - Toggles the player's physics flag, allowing them to no clip

        F5 - Toggles the player's god mode flag, making them invincible

	F6 - Sets time to run at 5x the speed

        'P' - Pauses/unpauses the game, and respawns the player if the game over screen is shown

        'T' - Slows the game time to 1/10th the speed when held

	'R' - Loads the next map in the world, or goes to the victory screen if pressed on the last map

	'H' - Spawns an ally NPC tank at the bottom left of the map


    Controller input:

        Left Stick - Moves the tank
                     The tank moves in its forward direction, and turns towards the orientation
		     of the stick at a fixed rate.

	Right Stick - Turns the turret
		      The Turret rotates to the orientation of the right stick (faster than the left stick tank rotation)
		      independent of the tank base. That is, if the base of the tank turns, the turret remains facing the
		      same world direction.

	Left/Right trigger - When pulled over half way, the tank will fire bullets out of the front of the turret at a rate of
			     10 bullets per second. The gun continuously fires as long as the trigger is held, with no other
			     restrictions.

	Start Button - Pauses/unpauses the game, and respawns the player if the game over screen is shown

	Back Button - Pauses the game if the game isn't paused, and quits to main menu if the game is paused. At the main menu screen,
		      it closes the application.


-----Juice Features-----


	- Small game feel effects, such as:
		- Gun recoil when fired
		- Tanks shake when they move
		- Screen shake when the player takes damage
		- Explosions are feathered and layered to make them more dynamic
		- Flashing/tint on the player and healthbar when invincible
		- Pause/GameOver screen fades in and out
		- Maps load by fading out and in
		- Item PickUps move in a circular fashion on the map, to draw attention (and each have their own acquire sound effect)
		- Maps have hard coded parameters, but otherwise are randomly generated each game start
		- Sound Effects
			- Randomized explosion sounds
			- Player tank engine rumble, that changes when the player is moving
			- Sounds effects for pausing, resuming, respawning, picking up pickups, etc
			- Separate music for Main Menu, gameplay, and the victory screen

	- Redefined Tile Definitions, to account for tiles being driven over, seen through, and are a spawnable location
		- Now water can be seen through but can't be driven over

	- PickUps, which are spawned on map creation and are dropped by turrets and heavy tanks (the big ones) with a random change
	  Types include
		- Health replenish (Red Plus) 	- refills the player's health to full
		- Invincibility (Yellow Sun) 	- Makes the player invincible for 5 seconds, stacks with other invincibility pickups
		- Reinforcements (Gray Shield)	- Summons 5 NPC allies to fight alongside the player
		- Speed Boost (White Wind)	- Increases the player's speed for 5 seconds, stacks with other speed boost pickups

	- Advanced AI (well at least more advanced)
		- Entities now look for the nearest hostile Entity to them in line of sight, and always prioritize them first
		- If they lose line of sight, they pursue their last seen position, unless a new target is found
		- Hostile NPCs then wander after they reach the last target position, or if they time out
		- Friendly NPCs, instead of wandering, follow a trail of "breadcrumb" positions to regroup back with the player
		- Hostile turrets turn back and forth on the last target orientation they had, as if they were searching
	

-----Deep Learning-----

The most prominent insight I had from this project is learning to program ahead. I realized when it came to juicing that there were so many ideas
I wanted to implement that I would need to ensure the game was built correctly from the ground up to accommodate these changes. Keeping functions
generic and open be used in a variety of situations helps with this, as well as taking an extra 30 seconds then to add a little tweak to allow
changes to be made later.

Thus, I began to realize that programming is an incredibly dynamic medium, in that old code that is finished being written...isn't. I often find myself
going back to add things, refactor, or even clarify something I didn't before, such as "this won't work in this case." Finished code remains as just an
active member of the development process as the new code being added as of now, and it's important to balance working in both areas.


More specifically, I found myself struggling to add certain features to the game, such as pickups and their movement animations. I was asking questions
like "Should they inherit from Entity? Would they work with the current Map implementation? What about with my TileDefinitions, would they spawn correctly?"
I naturally found myself wanted to make the PickUp work with the existing system, when in the end I found it much easier to just rework the Map class a bit.
It took less time, and not only solved the problem at hand, but opens up different feature possibilities I can add in the future.

The Map wasn't quite done yet....and I don't think it ever will be.


-----Closing-----

If you have any questions or concerns email me at amchase@smu.edu.

Thanks!