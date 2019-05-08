Events, Jobs, and NamedProperties
Author: Andrew Chase

-----Known Issues/Changes-----

-----How to Use-----

Run the .exe or build the solution to start the game.
	
-----Game Controls-----

o	Named Properties
	-	This is a test done automatically on Game Initialization – See the function “Game::RunNamedPropertiesTest()” in Game.cpp, line 202.
	-	This test runs various gets and sets with many Asserts
	-	If any assert fails, the game will display an error and pause
	-	If no asserts fail, the message "NamedProperties tests passed!" will be printed to the Developer console (press ~ to open and close the developer console)
o	EventSystem
	-	The event system is tested using 1 event and 3 callback subscriber types:
		•	A Static function callback (Game::EventSystemStaticCallback() in Game.hpp)
		•	An Object Method callback on Game class (Game::EventSystemObjectMethodCallback() in Game.hpp)
		•	A standalone C Function callback in Game.hpp (EventSystemCCallback in Game.hpp)
	-	Controls are as follows:
		•	Press ‘Y’ to fire the test event. Any subscribers that existed at the time of firing will respond by printing text to the screen.
			o	There is a 3 second cooldown before you can fire again – this is done intentionally to allow you to read the message printed, and to avoid misreading the results through spamming events
		•	Press ‘U’, ‘I’, or ‘O’ to toggle whether the static function callback, the object method callback, or the C Function callback is subscribed to the test event respectively. Any event not subscribed will not print text to the screen when the event is fired, as expected.
			o	BIG NOTE: Subscribing/unsubscribing will change the ordering of which the subscribers are called!!! It works as a queue, so the first put in will be the first called. You can manipulate this ordering by removing all callbacks and putting them back in in the order you want them called. THEY ARE NOT NECESSARILY CALLED IN THE ORDER THEY ARE LISTED ON THE SCREEN.
	•	Press ‘J’, ‘K’, or ‘L’ to toggle whether the static function callback, the object method callback, or the C Function callback will consume the event respectively.
		o	i.e. All subscribers after the consumer will not be called and will not print anything to the screen.
	•	All controls are also listed on the screen as well as here.
o	JobSystem
	-	Each job is implemented as a for loop that count to 1,000,000 and then returns, to simulate work.
	-	Jobs can be created and executed in two ways:
		•	By pressing ‘N’, 1000 jobs will be created and added to the job system, where they will be executed asynchronously
		•	By pressing ‘M’, 1000 jobs will be created, executed and finalized sequentially on the main thread to simulate how the work will be done without the job system
			o	This will create a stall until all jobs are finished
		•	In both ‘N’ and ‘M’ cases, you cannot add additional jobs until the previous batch of 1000 is done. This was done intentially game-side to better visualize how the work is done 	without confiscating it by adding jobs to the main thread and job system simultaneously
		•	By default, the Game will create one worker thread for the work.
			o	Press ‘V’ to destroy a thread, reducing the number of working threads by 1
	-	You can reduce the thread count to 0, where you will see the job system do no work.
		o	Press ‘B’ to create a worker thread, increasing the number of working threads by 1
	•	All controls are also listed on the screen as well as here


-----Extras-----

-----Closing-----

If you have any questions or concerns email me at amchase@smu.edu and I'll do my best to accommodate!

Thanks!