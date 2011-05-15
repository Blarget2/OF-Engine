
// Copyright 2010 Alon Zakai ('kripken'). All rights reserved.
// This file is part of Syntensity/the Intensity Engine, an open source project. See COPYING.txt for licensing.

#include "cube.h"
#include "engine.h"
#include "game.h"

#include "message_system.h"

#include "world_system.h"
#include "of_tools.h"

using namespace lua;

// Kripken: These are bounding boxes for positioning in the octree, as opposed to bounding boxes that
// are actually used to check for collisions. These octree-positioning bounding boxes only determine
// in which octants we will sit, i.e., WHEN actual collisions will be checked. If you see a bounding
// box which looks ok but you can walk through it (or part of it) then it might simply be that the
// bounding box is fine, but sauer doesn't know that a certain world area actually contains that
// entity.
///////////////////bool getentboundingbox(extentity &e, ivec &o, ivec &r)


//! Set by load_world when started, unset when finished
bool WorldSystem::loadingWorld = false;

void WorldSystem::triggerCollide(CLogicEntity *mapmodel, physent *d, bool ellipse)
{
    Logging::log(Logging::INFO, "triggerCollide: %lu, %lu\r\n", (unsigned long)mapmodel, (unsigned long)d);

    if (d->type != ENT_PLAYER)
    {
//        Logging::log(Logging::INFO, "Non-player causing collide, so ignore\r\n");
        return; // No need to trigger collisions for cameras, lights, etc. TODO: ENT_AI?
    }

    if (!mapmodel || mapmodel->isNone())
    {
        Logging::log(Logging::ERROR, "Invalid mapmodel to trigger collide for\r\n");
        return; // Invalid or uninialized mapmodel
    }

    CLogicEntity *colliderEntity = LogicSystem::getLogicEntity(d);
    if (!colliderEntity || colliderEntity->isNone())
    {
        Logging::log(Logging::INFO, "Invalid colliding entity to collide with\r\n");
        return; // Most likely a raycasting collision, or camera, etc. - not things we trigger events for
    }

    engine.getref(mapmodel->luaRef);
    #ifdef SERVER
    engine.t_getraw("on_collision");
    #else
    engine.t_getraw("client_on_collision");
    #endif
    engine.push_index(-2).getref(colliderEntity->luaRef).call(2, 0).pop(1);
}

int numExpectedEntities = 0;
int numReceivedEntities = 0;

void WorldSystem::setNumExpectedEntities(int num)
{
    numExpectedEntities = num;
    numReceivedEntities = 0;
}

void WorldSystem::triggerReceivedEntity()
{
    numReceivedEntities += 1;

    if (numExpectedEntities > 0)
    {
        float val = float(numReceivedEntities)/float(numExpectedEntities);
        val = clamp(val, 0.0f, 1.0f);
        char *text = tools::vstrcat(NULL, "sis", "received entity ", numReceivedEntities, "...");
        if (WorldSystem::loadingWorld) // Show message only during map loading, not when new clients log in
            renderprogress(val, text);
        delete[] text;
    }
}
// AreaTrigger collisions

bool WorldSystem::triggeringCollisions = false;

//! Check for triggering collisions, i.e., to run trigger events on AreaTriggers
void WorldSystem::checkTriggeringCollisions(CLogicEntity *entity)
{
    assert(entity->isDynamic());

    WorldSystem::triggeringCollisions = true;
    collide(entity->dynamicEntity, vec(0,0,0));
    WorldSystem::triggeringCollisions = false;
}

