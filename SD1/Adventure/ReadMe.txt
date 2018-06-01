SD1 C27 A9 - Adventure Gold
Author: Andrew Chase

-----Known Issues-----

There are several known issues in this build.

Entities can still clip outside of the map, which can cause problems with null reference exceptions.
I believed I've fixed all that could occur, however everytime I think that another one occurs.
This can be frequent especially when the game is in high-speed mode.

Sometime white lines occur on the map from some sort of rendering/texture coordinate issue on the tiles,
it is more common walking north to south rather than east to west.

Spears and weapons don't render to the map when dropped - since enemies randomly drop an equipped item on death,
this can sometimes drop the weapon and it will not show

RoomsAndPaths will draw paths over walls, completely replacing the wall with a path, making a 3-sided room at times
I think this can be fixed by constructing a shortest path with Dijkstra's algorithm, but for the sake of time I left
it as is.


-----How to Use-----

Run the .exe or build the solution - The different game modes have text at the bottom to state which buttons
do what to control state (pausing, resuming, quitting).

The game has a single adventure where you slay Marcus the bandit on Map 3 to win. The first two maps are randomly
generated, and differ every time the adventure is created.

The game has both ranged attacks and melee attacks - melee attacks do more damage but are more difficult to
use without taking damage.


-----Game Controls-----

    Keyboard input:

        'T' - Slows the game time to 1/10th the speed

	'WASD' keys - Moves the player around up/down/left/right with a fixed speed

	SPACE - Fires an arrow out in the actor's sprite direction

	'Q' - Thrusts the player's spear (melee attack)

	F1 - No clip, making the player move through walls and all entities (don't go outside the map)

	F2 - High-Speed, makes the game update at 10x the normal speed

	'L' - Brings up a test dialogue box, for debugging

	

    Controller input: (For realism I only put gameplay controls on the controller, and no developer functionality)

        Left Stick - Moves the Player object around the map
    
	A Button - Fires an arrow out in the actor's sprite direction

	B Button - Thrusts the player's spear in the player's forward direction

	X Button - When standing on an item on the ground, adds the item to your inventory (cannot be accessed again)

	Y Button - When standing on an item on the ground, equips the item to the appropriate slot, and unequips any item
		   that might already be in that slot


-----Features-----
Below is the list of things I implemented for this assignment:

- Rendering
	- glDrawArrays
	- Map::RenderTiles() optimized (with optimization for Entities too!)
	- Two sprites for tiles (see the walls of the Dungeon map - Map3)
	- Entities sorted by draw order

- Physics Features
	- Actor vs. Actor collision
	- Momentum, velocity, and knock-back (knockback scaled on amount of damage taken, spear attacks knockback a lot)

- Map MetaData Features
	- HeatMap, used for AI behavior

-Stats and Combat Features
	- Stats on all Entities
	- Combat (damage dealt is the (attack of the attacker - defense of defender), speed determines attack speed)

- Items, Equipment, Loot features
	- Items affect stats (all items are equippable in this game)
	- Inventory and Equipment (player can pick up and equip items, cannot drop items unless replacing it with another)

- Dialogue Features
	- Dialogue Class implemented, without Dialogue style param
	- Dialogue game state in Game (Press 'L' while playing to see it working)

- Map Generation 
	- SpawnActor
	- SpawnItem
	- Cellular Automata (Map1 - water is generated using it)
	- RoomsAndPaths (has path-over-walls bug, see top for explanation)

- Audio Features
	- Background music based on game state

- Overlay Features
	- HUD - Actor healthbars and Actor names rendered

- AI Features	
	- Basic pathfinding - used in all 4 Behaviors I implemented
	- Behavior/Utility system
		- 4 Behaviors: Flee, Wander, Melee Attack, Ranged Attack
			- If there is no target, the Actor just wanders
			- Else it attacks the target
			- When health is low, it flees
			- Marcus doesn't flee becaused he's pretty tough

- Custom Features
	- Equipment renders on actors when equipped - Easy to get working, difficult to get it working well in code
	- AI - Entities search for hostile actors, so Players can have allies (see Villagers on Map1, they attack Bandits)


-----Deep Learning-----

The most prominent insight I had from this project is learning to not fixate on a single problem. I spent 10 of the 17 days of this assignment on AI, just
because it wasn't working to my liking - I wanted tier 5 AI and for this project it was too much. It didn't seem that complex of a problem, as I was able to
get the basics working on the first day, but after tweaking and refining more and more I continued to find other edge-cases and issues, leaving me to believe
just leaving it as it was would have both made it the best for the game while also saving me time. As a result of this mistake, I crunched like crazy for the 
last two days of the assignment, which is exactly what I did for Asteroids Playable.

Thus, I also realized that tendencies and habits don't go away easily. Through all of Incursion I felt I practiced good programming habits, 
and didn't fixate on one feature for the length of the project. It's important to be aware of these habits, and to constantly and consciously avoid performing
them, otherwise you may still end up doing it.

A specific example of this insight was when I was working on the RoomsAndPaths MapGenStep. I managed to get it to work on the first test, however it had the issue of paths
running over/along walls, overwriting them completely and creating 2/3-sided rooms, which looked bad. I figured this was a quick fix - just use a HeatMap to path
through walls as little as possible. However, I began to realize that my pathfinding algorithm didn't perform the algorithm exactly, just some variation of it.
In addition, the pathfinding wasn't returning the shortest path, but the greedy minimum path (taking the cheapest neighbor each step), which wouldn't solve this
problem. I eventually found myself on the graph traversal wiki trying to implement an entirely new pathfinding algorithm......just so paths wouldn't go over walls....

I didn't have sounds, UI, balanced combat, or even final maps.

So unlike with the AI, I caught myself early in this instance, CTRL-Z'd any recent changes I made, submitted to perforce and moved on. So yeah, my village looks a bit
funny, but it's better than having a good-looking village and nothing else in the game!


-----Closing-----

If you have any questions or concerns email me ASAP at amchase@smu.edu - I'm unsure if all data assets are in Perforce correctly so just contact me with any issues
and I'll fix them pronto!

Thanks!

-Andrew Chase