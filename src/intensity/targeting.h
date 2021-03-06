
// Copyright 2010 Alon Zakai ('kripken'). All rights reserved.
// This file is part of Syntensity/the Intensity Engine, an open source project. See COPYING.txt for licensing.

//! Manages relating mouse positions, world positions, and logic entities

struct TargetingControl
{
#ifdef CLIENT
    static void setupOrientation();

    //! Contains the latest and current information about what world geometry (cube) the mouse cursor is hovering over
    static vec           worldPosition;       // Position in cube geometry

    //! Contains the position where the mouse cursor is aiming. Equal to worldposition in general, unless hovering
    //! on an entity
    static vec targetPosition;

    //! Contains the latest and current information about what logic entity the mouse cursor is hovering over
    static CLogicEntity *targetLogicEntity;
#endif

    //! Utility that wraps around sauer's complex system for intersecting a ray (from->to) with a dynamic entity
    static void intersectClosestDynamicEntity(vec &from, vec &to, physent *targeter, float &dist, dynent*& target);

    //! Utility that wraps around sauer's complex system for intersecting a ray (from->to) with a mapmodel
    static void intersectClosestMapmodel(vec &from, vec &to, float& dist, extentity*& target);

    //! Find the logic entity that the ray from->to intersects, and is not 'targeter' (the entity casting the ray, typically)
    static void intersectClosest(vec &from, vec &to, physent *targeter, float& dist, CLogicEntity *&entity);

#ifdef CLIENT
    //! Sets or unsets the state of letting the mouse 'target' entities, i.e., mark them
    //! in a visual manner and let clicking affect that entity
    static void setMouseTargeting(bool on);

    //! Called per-frame, sets worldPosition and targetLogicEntity to their appropriate values
    //! @param forceEntityCheck Set to true to find target entities even if default mouse targeting (hover targeting) is off
    static void determineMouseTarget(bool forceEntityCheck=false);
#endif

    //! Calculate the movement of a physical entity. Takes into account both
    //! actual movement since the last calculation here, and the velocity
    static float calculateMovement(physent* entity);

    //! Calculates how long a physics frame should be for a particular physics entity. On
    //! the client, this depends on how visible the other entity is (no need for 200fps of
    //! physics for something that is only a few pixels large), the screen resolution, etc.
    //! On the server, this will (TODO) depend on how the entity is visible by any of
    //! the players.
    static void calcPhysicsFrames(physent *entity);
};

