#pragma once

#include "WaypointManager.h"

using namespace std;

class Vehicle;
class PickupItem;
typedef vector<PickupItem*> vecPickups;

class AIManager
{
public:
	AIManager();
	virtual  ~AIManager();
	void	release();
	HRESULT initialise(ID3D11Device* pd3dDevice);
	void	update(const float fDeltaTime);
	void	mouseUp(int x, int y);
	void	keyDown(WPARAM param);
	void	keyUp(WPARAM param);
	void    seek();
	void    flee();
	void    arrive();
	void    wander();
	void	avoid();
	void    pursuit();
	void    pathfind(int x, int y);
	void	seekWaypoint();
	void	findWaypoint();


	Vector2D m_pCarPos;
	Vector2D m_pCar2Pos;
	Vector2D waypointCheck;



protected:
	bool	checkForCollisions();
	void	setRandomPickupPosition(PickupItem* pickup);

private:
	vecPickups              m_pickups;
	Vehicle*				m_pCar = nullptr;
	Vehicle*				m_pCar2 = nullptr;
	WaypointManager			m_waypointManager;
	Waypoint*               currentTarget;
	Waypoint*               currentTarget2;
	Waypoint*               tempTarget;
	Waypoint*				originalStart;
	vecWaypoints			possiblePathPoints;
	vecWaypoints			PathPoints;
	vecWaypoints			PreviousPoints;
	vecWaypoints			BadPoints;
	Waypoint*				startPoint;
	Waypoint*				nextBest;
	Waypoint*				passPickup;

	
	bool seeking = false;
	bool fleeing = false;
	bool arrived = true;
	bool inPursuit = false;
	bool wandering1 = false;
	bool wandering2 = true;
	float speed;
	float maxSpeed;
	bool waypointComplete = false;

	int currentState = 1;
	bool pathFound = false;
	int duplicates = 0;
	bool activateSearch = false;

	bool passenger = false;
};

