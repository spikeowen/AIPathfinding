# AIPathfinding
 Year 2 module, using a given simple framework to make a car navigate a map via waypoints

AI Program controls
1-	State 1
P- Red chooses random waypoint to go to
S- Red seeks blue until they ‘collide’
W- Toggles Red to wander or not
T- Toggles Blue to wander or not
A- Red will approach blue and slow down when close (Supposed to simulate Pursuit)
Space- moves red to center (can wait for blue there to see flee better)
Mouse left click- moves red to close waypoint of mouse
2-	State 2
Mouse left click- moves red to close waypoint of mouse, tries to choose path around buildings in an attempt at A* pathfinding
3-	State 3
Decisions are automatic, red seems to seek, flee then do so again


-Fleeing is done automatically after successful seeking/arrival
-Blue is set to initially wander, red is not, red just goes to the nearest starting waypoint
-If red and blue do get close naturally, red should flee, but forgets original destination so not quite obstacle avoidance 
