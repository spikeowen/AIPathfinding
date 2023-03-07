#include "AIManager.h"
#include "Vehicle.h"
#include "DrawableGameObject.h"
#include "PickupItem.h"
#include "Waypoint.h"
#include "main.h"
#include "constants.h"

AIManager::AIManager()
{
	m_pCar = nullptr;
    m_pCar2 = nullptr;
    currentTarget = nullptr;
    currentTarget2 = nullptr;
}

AIManager::~AIManager()
{
	release();
}

void AIManager::release()
{
	clearDrawList();

	for (PickupItem* pu : m_pickups)
	{
		delete pu;
	}
	m_pickups.clear();

	delete m_pCar;
	m_pCar = nullptr;
    delete m_pCar2;
    m_pCar2 = nullptr;

}

HRESULT AIManager::initialise(ID3D11Device* pd3dDevice)
{
    // create the vehicle 
    float xPos = -500; // an abtrirary start point
    float yPos = 300;

    m_pCar = new Vehicle();
    HRESULT hr = m_pCar->initMesh(pd3dDevice, carColour::redCar);
    m_pCar->setVehiclePosition(Vector2D(xPos, yPos));
    if (FAILED(hr))
        return hr;

    m_pCar2 = new Vehicle();
    HRESULT hr2 = m_pCar2->initMesh(pd3dDevice, carColour::blueCar);
    m_pCar2->setVehiclePosition(Vector2D(xPos + 30.0f, yPos + 10.0f));
    if (FAILED(hr2))
        return hr2;

    // setup the waypoints
    m_waypointManager.createWaypoints(pd3dDevice);
    m_pCar->setWaypointManager(&m_waypointManager);
    currentTarget = m_waypointManager.getNearestWaypoint(m_pCar->getPosition());
    m_pCar->setPositionTo(currentTarget->getPosition());

    m_pCar2->setWaypointManager(&m_waypointManager);
    currentTarget2 = m_waypointManager.getRandomWaypoint();
    m_pCar2->setPositionTo(currentTarget2->getPosition());

    // create a passenger pickup item
    PickupItem* pPickupPassenger = new PickupItem();
    hr = pPickupPassenger->initMesh(pd3dDevice, pickuptype::Passenger);
    m_pickups.push_back(pPickupPassenger);

    // NOTE!! for fuel and speedboost - you will need to create these here yourself

    // (needs to be done after waypoint setup)
    setRandomPickupPosition(pPickupPassenger);

    return hr;
}


void AIManager::update(const float fDeltaTime)
{
    for (unsigned int i = 0; i < m_waypointManager.getWaypointCount(); i++) {
        m_waypointManager.getWaypoint(i)->update(fDeltaTime);
        AddItemToDrawList(m_waypointManager.getWaypoint(i)); // if you uncomment this, it will display the waypoints
    }

    for (int i = 0; i < m_waypointManager.getQuadpointCount(); i++)
    {
        Waypoint* qp = m_waypointManager.getQuadpoint(i);
        qp->update(fDeltaTime);
        //AddItemToDrawList(qp); // if you uncomment this, it will display the quad waypoints
    }

    // update and display the pickups
    for (unsigned int i = 0; i < m_pickups.size(); i++) {
        m_pickups[i]->update(fDeltaTime);
        AddItemToDrawList(m_pickups[i]);
    }

	// draw the waypoints nearest to the car
	/*
    Waypoint* wp = m_waypointManager.getNearestWaypoint(m_pCar->getPosition());
	if (wp != nullptr)
	{
		vecWaypoints vwps = m_waypointManager.getNeighbouringWaypoints(wp);
		for (Waypoint* wp : vwps)
		{
			AddItemToDrawList(wp);
		}
	}
    */

    // update and draw the car (and check for pickup collisions)
	if (m_pCar != nullptr)
	{
		m_pCar->update(fDeltaTime);
        m_pCarPos = m_pCar->getPosition();

        switch (currentState) {
            //regular states
        case 1:
            if (m_pCarPos.Distance(m_pCar2Pos) < 30)
            {
                //commented out to show obstacle avoidance
                fleeing = true;
            }

            if (seeking == true)
            {
                if (inPursuit == false)
                {
                    seek();
                }
            }
            else if (arrived == false)
            {
                if (inPursuit == false)
                {
                    arrive();
                }
            }
            else
            {
                if (fleeing == true)
                {
                    if (inPursuit == false)
                    {
                        flee();
                        //avoid();
                    }

                }
            }
            break;
        //pathhfind
        case 2:
            if (activateSearch == true)
            {
                findWaypoint();
            }
            if (pathFound == true)
            {
                seekWaypoint();
            }
            break;
            //Decision making
        case 3:
            if (passenger == true)
            {
                m_pCar->setPositionTo(m_pCar2->getPosition());
                m_pCar2->slower();
                if (m_pCar2Pos.Distance(m_pCarPos) < 15.0f)
                {
                    passenger = false;
                }
            }
            else
            {
                m_pCar->setPositionTo(passPickup->getPosition());
            }
            break;
        }
        


        wander();
		checkForCollisions();
		AddItemToDrawList(m_pCar);
	}

    if (m_pCar2 != nullptr )
    {
        m_pCar2->update(fDeltaTime);
        m_pCar2Pos = m_pCar2->getPosition();

        if (wandering1 == true)
        {
            if (inPursuit == true)
            {
                pursuit();
            }
            else
            {
                wander();
                m_pCar2->speedReset();
            }
        }
        else
        {
            wander();
            m_pCar2->speedReset();
        }
       
        //checkForCollisions();
        AddItemToDrawList(m_pCar2);
    }
}

void AIManager::mouseUp(int x, int y)
{
    if (currentState == 1)
    {
        // get a waypoint near the mouse click, then set the car to move to the this waypoint
        Waypoint* wp = m_waypointManager.getNearestWaypoint(Vector2D(x, y));
        if (wp == nullptr)
            return;
        currentTarget = wp;
        // steering mode
        m_pCar->setPositionTo(currentTarget->getPosition());
    }
    else if (currentState == 2)
    {
        pathfind(x,y);
    }
}

void AIManager::keyUp(WPARAM param)
{
    const WPARAM key_a = 65;
    switch (param)
    {
        case key_a:
        {
            OutputDebugStringA("a Up \n");
            break;
        }
    }
}

void AIManager::keyDown(WPARAM param)
{
    const WPARAM key_1 = 49;
    const WPARAM key_2 = 50;
    const WPARAM key_3 = 51;
	// hint 65-90 are a-z
	const WPARAM key_a = 65;
    const WPARAM key_p = 80;
    const WPARAM key_r = 82;
	const WPARAM key_s = 83;
    const WPARAM key_t = 84;
    const WPARAM key_w = 87;
    const WPARAM key_space = 32;

    switch (param)
    {
        //Add state toggle, check the logic of pathfinding, then test to see what happens
        case VK_NUMPAD0:
        {
            OutputDebugStringA("0 pressed \n");
            break;
        }
        case key_1:
        {
            OutputDebugStringA("1 pressed \n");
            currentState = 1;
            break;
        }
        case key_2:
        {
            OutputDebugStringA("2 pressed \n");
            currentState = 2;
            break;
        }
        case key_3:
        {
            OutputDebugStringA("3 pressed \n");
            currentState = 3;
            break;
        }
        case key_a:
        {
            OutputDebugStringA("a Down \n");
            arrived = false;
            break;
        }
        case key_p:
        {
            inPursuit = !inPursuit;
            if (inPursuit == false)
            {
                currentTarget2 = m_waypointManager.getNearestWaypoint(m_pCar2->getPosition());
                m_pCar2->setPositionTo(currentTarget2->getPosition());
                m_pCar2->speedReset();
            }
            break;
        }
        case key_r:
        {
            currentTarget = m_waypointManager.getRandomWaypoint();
            m_pCar->setPositionTo(currentTarget->getPosition());
            break;
        }
		case key_s:
		{
            seeking = true;
			break;
		}
        case key_t:
		{
            if (wandering2 == true)
                wandering2 = false;
            else
                wandering2 = true;
            break;
        }
        case key_w:
        {
            if (wandering1 == true)
                wandering1 = false;
            else
                wandering1 = true;
            break;
        }
        case key_space:
        {
            m_pCar->setPositionTo(Vector2D(12,8));
            break;
        }
        // etc
        default:
            break;
    }
}

void AIManager::setRandomPickupPosition(PickupItem* pickup)
{
    if (pickup == nullptr)
        return;

    int x = (rand() % SCREEN_WIDTH) - (SCREEN_WIDTH / 2);
    int y = (rand() % SCREEN_HEIGHT) - (SCREEN_HEIGHT / 2);

    Waypoint* wp = m_waypointManager.getNearestWaypoint(Vector2D(x, y));
    if (wp) {
        pickup->setPosition(wp->getPosition());
    }

    passPickup = wp;
}

/*
// IMPORTANT
// hello. This is hopefully the only time you are exposed to directx code 
// you shouldn't need to edit this, but marked in the code below is a section where you may need to add code to handle pickup collisions (speed boost / fuel)
// the code below does the following:
// gets the *first* pickup item "m_pickups[0]"
// does a collision test with it and the car
// creates a new random pickup position for that pickup

// the relevant #includes are already in place, but if you create your own collision class (or use this code anywhere else) 
// make sure you have the following:
#include <d3d11_1.h> // this has the appropriate directx structures / objects
#include <DirectXCollision.h> // this is the dx collision class helper
using namespace DirectX; // this means you don't need to put DirectX:: in front of objects like XMVECTOR and so on. 
*/

bool AIManager::checkForCollisions()
{
    if (m_pickups.size() == 0)
        return false;

    XMVECTOR dummy;

    // get the position and scale of the car and store in dx friendly xmvectors
    XMVECTOR carPos;
    XMVECTOR carScale;
    XMMatrixDecompose(
        &carScale,
        &dummy,
        &carPos,
        XMLoadFloat4x4(m_pCar->getTransform())
    );

    // create a bounding sphere for the car
    XMFLOAT3 scale;
    XMStoreFloat3(&scale, carScale);
    BoundingSphere boundingSphereCar;
    XMStoreFloat3(&boundingSphereCar.Center, carPos);
    boundingSphereCar.Radius = scale.x;

    // do the same for a pickup item
    // a pickup - !! NOTE it is only referring the first one in the list !!
    // to get the passenger, fuel or speedboost specifically you will need to iterate the pickups and test their type (getType()) - see the pickup class
    XMVECTOR puPos;
    XMVECTOR puScale;
    XMMatrixDecompose(
        &puScale,
        &dummy,
        &puPos,
        XMLoadFloat4x4(m_pickups[0]->getTransform())
    );

    // bounding sphere for pickup item
    XMStoreFloat3(&scale, puScale);
    BoundingSphere boundingSpherePU;
    XMStoreFloat3(&boundingSpherePU.Center, puPos);
    boundingSpherePU.Radius = scale.x;

	// THIS IS generally where you enter code to test each type of pickup
    // does the car bounding sphere collide with the pickup bounding sphere?
    if (boundingSphereCar.Intersects(boundingSpherePU))
    {
        OutputDebugStringA("A pickup collision has occurred!\n");
        m_pickups[0]->hasCollided();
        setRandomPickupPosition(m_pickups[0]);
        passenger = true;

        // you will need to test the type of the pickup to decide on the behaviour
        // m_pCar->dosomething(); ...

        return true;
    }

    return false;
}



void AIManager::seek()
{
    m_pCar->setPositionTo(m_pCar2->getPosition());
    if (m_pCarPos.Distance(m_pCar2Pos) < 30.0f)
    {
        seeking = false;
    }
}


void AIManager::flee()
{
    //if (m_pCarPos.Distance(m_pCar2Pos) > 60)
    //{
    //    m_pCar->setPositionTo(m_waypointManager.getNearestWaypoint(m_pCarPos)->getPosition());
    //    fleeing = false;
    //}
    //else
    //{
    //    m_pCar->getDirection(m_pCar2Pos);

    //    fleeing = true;
    //}

    m_pCar->setPositionTo(m_pCar2->getPosition() * -1);
    if (m_pCarPos.Distance(m_pCar2Pos) > 60.0f)
    {
        fleeing = false;
        currentTarget = m_waypointManager.getNearestWaypoint(m_pCar->getPosition());
        m_pCar->setPositionTo(currentTarget->getPosition());
    }
}


void AIManager::arrive()
{
    m_pCar->setPositionTo(m_pCar2->getPosition());
    if (m_pCarPos.Distance(m_pCar2Pos) < 100.0f)
    {
        //maxSpeed = m_pCar->getSpeed();
        //speed = maxSpeed;
        //m_pCar->setCurrentSpeed(speed);
        m_pCar->slow();
        m_pCar2->slower();
    }

    if (m_pCarPos.Distance(m_pCar2Pos) < 30.0f)
    {
        arrived = true;
        /*m_pCar->setCurrentSpeed(maxSpeed);*/
        m_pCar->speedReset();
        m_pCar2->speedReset();
    }
}


void AIManager::wander()
{
    if (wandering1 == true)
    {
        if (m_pCarPos == currentTarget->getPosition())
        {
            currentTarget = m_waypointManager.getRandomWaypoint();
            m_pCar->setPositionTo(currentTarget->getPosition());
        }
    }

    if (wandering2 == true)
    {
        if (m_pCar2Pos == currentTarget2->getPosition())
        {
            currentTarget2 = m_waypointManager.getRandomWaypoint();
            m_pCar2->setPositionTo(currentTarget2->getPosition());
        }
    }
}

void AIManager::avoid()
{
    m_pCar->setPositionTo(m_pCar2->getPosition() * -1);
    if (m_pCarPos.Distance(m_pCar2Pos) > 120.0f)
    {
        fleeing = false;
        m_pCar->setPositionTo(currentTarget->getPosition());
    }
}

void AIManager::pursuit()
{
    m_pCar2->setPositionTo(m_pCar->getPosition());
    if (m_pCar2Pos.Distance(m_pCarPos) < 15.0f)
    {
        m_pCar2->slow();
    }

}

void AIManager::pathfind(int x, int y)
{
    // get a waypoint near the mouse click, then set the car to move to the this waypoint
    Waypoint* wp = m_waypointManager.getNearestWaypoint(Vector2D(x, y));
    if (wp == nullptr)
        return;
    currentTarget = wp;
    
    tempTarget = m_waypointManager.getNearestWaypoint(m_pCarPos);

    originalStart = tempTarget;
    
    activateSearch = true;
}

void AIManager::seekWaypoint()
{
    
    m_pCar->setPositionTo(PathPoints[0]->getPosition());

    if (m_pCarPos == PathPoints[0]->getPosition())
    {
        PathPoints.erase(PathPoints.begin() +0);
    }
    if (m_pCarPos == currentTarget->getPosition())
    {
        pathFound = false;
    }
}

void AIManager::findWaypoint()
{
    startPoint = tempTarget;
    possiblePathPoints = m_waypointManager.getNeighbouringWaypoints(tempTarget);
    duplicates = 0;

    for (int i = 0; i < BadPoints.size(); i++)
    {
        for (int j = 0; j < possiblePathPoints.size(); j++)
        {
            if (BadPoints[i] == possiblePathPoints[j])
            {
                possiblePathPoints.erase(possiblePathPoints.begin() + j);
            }
        }
    }

    nextBest = possiblePathPoints[0];
    for (int i = 0; i < possiblePathPoints.size(); i++)
    {
        if (possiblePathPoints[i]->distanceToWaypoint(currentTarget) < tempTarget->distanceToWaypoint(currentTarget))
        {
            tempTarget = possiblePathPoints[i];
        }

        if (nextBest->distanceToWaypoint(currentTarget) < possiblePathPoints[i]->distanceToWaypoint(currentTarget))
        {
            nextBest = possiblePathPoints[i];
        }
    }
    
    
    if (startPoint == tempTarget)
    {
        BadPoints.push_back(tempTarget);

        //for (int i = 0; i < BadPoints.size(); i++)
        //{
        //    for (int j = 0; j < PathPoints.size(); j++)
        //    {
        //        if (BadPoints[i] == PathPoints[j])
        //        {
        //            PathPoints.erase(PathPoints.begin() + j);
        //        }
        //    }
        //}


        if (PathPoints.size() == 0)
        {
            tempTarget = nextBest;
            PathPoints.push_back(tempTarget);
        }
        else
        {
            tempTarget = PathPoints[PathPoints.size() - 1];
            PathPoints.push_back(tempTarget);
        }
    }
    else
    {
        PathPoints.push_back(tempTarget);
        for (int i = 0; i < PathPoints.size(); i++)
        {
            for (int j = i + 1; j < PathPoints.size(); j++)
            {
                if (PathPoints[i]->getPosition() == PathPoints[j]->getPosition())
                {
                    duplicates++;
                }

                if (duplicates == 1)
                {
                    BadPoints.push_back(PathPoints[j - 1]);
                    //PathPoints.erase(PathPoints.begin() + j);
                    //PathPoints.erase(PathPoints.begin() + i);
                    duplicates = 0;
                }
            }
        }
    }
    

    if (tempTarget == currentTarget)
    {
        BadPoints.clear();
        activateSearch = false;
        pathFound = true;
    }
}