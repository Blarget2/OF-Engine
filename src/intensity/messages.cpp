

// Copyright 2010 Alon Zakai ('kripken'). All rights reserved.
// This file is part of Syntensity/the Intensity Engine, an open source project. See COPYING.txt for licensing.


#include "cube.h"
#include "engine.h"
#include "game.h"

#ifdef CLIENT
    #include "targeting.h"
#endif

#include "client_system.h"
#include "message_system.h"
#include "editing_system.h"
#include "network_system.h"
#include "of_world.h"
#include "of_tools.h"

int findtype(char *what);

/* Abuse generation from template for now */
void force_network_flush();
namespace server
{
    int& getUniqueId(int clientNumber);
}

namespace MessageSystem
{

// PersonalServerMessage

    void send_PersonalServerMessage(int clientNumber, const char* title, const char* content)
    {
        int exclude = -1; // Set this to clientNumber to not send to

        logger::log(logger::DEBUG, "Sending a message of type PersonalServerMessage (1001)\r\n");
        INDENT_LOG(logger::DEBUG);

         

        int start, finish;
        if (clientNumber == -1)
        {
            // Send to all clients
            start  = 0;
            finish = getnumclients() - 1;
        } else {
            start  = clientNumber;
            finish = clientNumber;
        }

#ifdef SERVER
        int testUniqueId;
#endif
        for (clientNumber = start; clientNumber <= finish; clientNumber++)
        {
            if (clientNumber == exclude) continue;
#ifdef SERVER
            fpsent* fpsEntity = game::getclient(clientNumber);
            bool serverControlled = fpsEntity ? fpsEntity->serverControlled : false;

            testUniqueId = server::getUniqueId(clientNumber);
            if ( (!serverControlled && testUniqueId != DUMMY_SINGLETON_CLIENT_UNIQUE_ID) || // If a remote client, send even if negative (during login process)
                 (false && testUniqueId == DUMMY_SINGLETON_CLIENT_UNIQUE_ID) || // If need to send to dummy server, send there
                 (false && testUniqueId != DUMMY_SINGLETON_CLIENT_UNIQUE_ID && serverControlled) )  // If need to send to npcs, send there
#endif
            {
                #ifdef SERVER
                    logger::log(logger::DEBUG, "Sending to %d (%d) ((%d))\r\n", clientNumber, testUniqueId, serverControlled);
                #endif
                sendf(clientNumber, MAIN_CHANNEL, "riss", 1001, title, content);

            }
        }
    }

#ifdef CLIENT
    void PersonalServerMessage::receive(int receiver, int sender, ucharbuf &p)
    {
        logger::log(logger::DEBUG, "MessageSystem: Receiving a message of type PersonalServerMessage (1001)\r\n");

        types::String title;
        getstring(title, p);
        types::String content;
        getstring(content, p);

        lapi::state.get<lua::Function>("LAPI", "GUI", "show_message")(title, content);
    }
#endif


// RequestServerMessageToAll

    void send_RequestServerMessageToAll(const char* message)
    {
        logger::log(logger::DEBUG, "Sending a message of type RequestServerMessageToAll (1002)\r\n");
        INDENT_LOG(logger::DEBUG);

        game::addmsg(1002, "rs", message);
    }

#ifdef SERVER
    void RequestServerMessageToAll::receive(int receiver, int sender, ucharbuf &p)
    {
        logger::log(logger::DEBUG, "MessageSystem: Receiving a message of type RequestServerMessageToAll (1002)\r\n");

        types::String message;
        getstring(message, p);

        send_PersonalServerMessage(-1, "Message from Client", message.get_buf());
    }
#endif

// LoginRequest

    void send_LoginRequest()
    {
        logger::log(logger::DEBUG, "Sending a message of type LoginRequest (1003)\r\n");
        INDENT_LOG(logger::DEBUG);

        game::addmsg(1003, "r");
    }

#ifdef SERVER
    void LoginRequest::receive(int receiver, int sender, ucharbuf &p)
    {
        logger::log(logger::DEBUG, "MessageSystem: Receiving a message of type LoginRequest (1003)\r\n");


        #ifdef SERVER
            if (world::scenario_code.is_empty())
            {
                send_PersonalServerMessage(
                    sender,
                    "Login failure",
                    "Login failure: instance is not running a map"
                );
                force_network_flush();
                disconnect_client(sender, 3); // DISC_KICK .. most relevant for now
            }
            server::setAdmin(sender, true);
            send_LoginResponse(sender, true, true);
        #else // CLIENT, during a localconnect
            ClientSystem::uniqueId = 9999; // Dummy safe uniqueId value for localconnects. Just set it here, brute force
            // Notify client of results of login
            send_LoginResponse(sender, true, true);
        #endif
    }
#endif

// YourUniqueId

    void send_YourUniqueId(int clientNumber, int uniqueId)
    {
        int exclude = -1; // Set this to clientNumber to not send to

        logger::log(logger::DEBUG, "Sending a message of type YourUniqueId (1004)\r\n");
        INDENT_LOG(logger::DEBUG);

                 // Remember this client's unique ID. Done here so always in sync with the client's belief about its uniqueId.
        server::getUniqueId(clientNumber) = uniqueId;


        int start, finish;
        if (clientNumber == -1)
        {
            // Send to all clients
            start  = 0;
            finish = getnumclients() - 1;
        } else {
            start  = clientNumber;
            finish = clientNumber;
        }

#ifdef SERVER
        int testUniqueId;
#endif
        for (clientNumber = start; clientNumber <= finish; clientNumber++)
        {
            if (clientNumber == exclude) continue;
#ifdef SERVER
            fpsent* fpsEntity = game::getclient(clientNumber);
            bool serverControlled = fpsEntity ? fpsEntity->serverControlled : false;

            testUniqueId = server::getUniqueId(clientNumber);
            if ( (!serverControlled && testUniqueId != DUMMY_SINGLETON_CLIENT_UNIQUE_ID) || // If a remote client, send even if negative (during login process)
                 (false && testUniqueId == DUMMY_SINGLETON_CLIENT_UNIQUE_ID) || // If need to send to dummy server, send there
                 (false && testUniqueId != DUMMY_SINGLETON_CLIENT_UNIQUE_ID && serverControlled) )  // If need to send to npcs, send there
#endif
            {
                #ifdef SERVER
                    logger::log(logger::DEBUG, "Sending to %d (%d) ((%d))\r\n", clientNumber, testUniqueId, serverControlled);
                #endif
                sendf(clientNumber, MAIN_CHANNEL, "rii", 1004, uniqueId);

            }
        }
    }

#ifdef CLIENT
    void YourUniqueId::receive(int receiver, int sender, ucharbuf &p)
    {
        logger::log(logger::DEBUG, "MessageSystem: Receiving a message of type YourUniqueId (1004)\r\n");

        int uniqueId = getint(p);

        logger::log(logger::DEBUG, "Told my unique ID: %d\r\n", uniqueId);
        ClientSystem::uniqueId = uniqueId;
    }
#endif


// LoginResponse

    void send_LoginResponse(int clientNumber, bool success, bool local)
    {
        int exclude = -1; // Set this to clientNumber to not send to

        logger::log(logger::DEBUG, "Sending a message of type LoginResponse (1005)\r\n");
        INDENT_LOG(logger::DEBUG);

                 // If logged in OK, this is the time to create a lua logic entity for the client. Also adds to internal FPSClient
        if (success)
            server::createluaEntity(clientNumber);


        int start, finish;
        if (clientNumber == -1)
        {
            // Send to all clients
            start  = 0;
            finish = getnumclients() - 1;
        } else {
            start  = clientNumber;
            finish = clientNumber;
        }

#ifdef SERVER
        int testUniqueId;
#endif
        for (clientNumber = start; clientNumber <= finish; clientNumber++)
        {
            if (clientNumber == exclude) continue;
#ifdef SERVER
            fpsent* fpsEntity = game::getclient(clientNumber);
            bool serverControlled = fpsEntity ? fpsEntity->serverControlled : false;

            testUniqueId = server::getUniqueId(clientNumber);
            if ( (!serverControlled && testUniqueId != DUMMY_SINGLETON_CLIENT_UNIQUE_ID) || // If a remote client, send even if negative (during login process)
                 (false && testUniqueId == DUMMY_SINGLETON_CLIENT_UNIQUE_ID) || // If need to send to dummy server, send there
                 (false && testUniqueId != DUMMY_SINGLETON_CLIENT_UNIQUE_ID && serverControlled) )  // If need to send to npcs, send there
#endif
            {
                #ifdef SERVER
                    logger::log(logger::DEBUG, "Sending to %d (%d) ((%d))\r\n", clientNumber, testUniqueId, serverControlled);
                #endif
                sendf(clientNumber, MAIN_CHANNEL, "riii", 1005, success, local);

            }
        }
    }

#ifdef CLIENT
    void LoginResponse::receive(int receiver, int sender, ucharbuf &p)
    {
        logger::log(logger::DEBUG, "MessageSystem: Receiving a message of type LoginResponse (1005)\r\n");

        bool success = getint(p);
        bool local = getint(p);

        if (success)
        {
            ClientSystem::finishLogin(local); // This player will be known as 'uniqueID' in the current module
            conoutf("Login was successful.\r\n");
            send_RequestCurrentScenario();
        } else {
            conoutf("Login failure. Please check your username and password.\r\n");
            disconnect();
        }
    }
#endif


// PrepareForNewScenario

    void send_PrepareForNewScenario(int clientNumber, const char* scenarioCode)
    {
        int exclude = -1; // Set this to clientNumber to not send to

        logger::log(logger::DEBUG, "Sending a message of type PrepareForNewScenario (1006)\r\n");
        INDENT_LOG(logger::DEBUG);

         

        int start, finish;
        if (clientNumber == -1)
        {
            // Send to all clients
            start  = 0;
            finish = getnumclients() - 1;
        } else {
            start  = clientNumber;
            finish = clientNumber;
        }

#ifdef SERVER
        int testUniqueId;
#endif
        for (clientNumber = start; clientNumber <= finish; clientNumber++)
        {
            if (clientNumber == exclude) continue;
#ifdef SERVER
            fpsent* fpsEntity = game::getclient(clientNumber);
            bool serverControlled = fpsEntity ? fpsEntity->serverControlled : false;

            testUniqueId = server::getUniqueId(clientNumber);
            if ( (!serverControlled && testUniqueId != DUMMY_SINGLETON_CLIENT_UNIQUE_ID) || // If a remote client, send even if negative (during login process)
                 (false && testUniqueId == DUMMY_SINGLETON_CLIENT_UNIQUE_ID) || // If need to send to dummy server, send there
                 (false && testUniqueId != DUMMY_SINGLETON_CLIENT_UNIQUE_ID && serverControlled) )  // If need to send to npcs, send there
#endif
            {
                #ifdef SERVER
                    logger::log(logger::DEBUG, "Sending to %d (%d) ((%d))\r\n", clientNumber, testUniqueId, serverControlled);
                #endif
                sendf(clientNumber, MAIN_CHANNEL, "ris", 1006, scenarioCode);

            }
        }
    }

#ifdef CLIENT
    void PrepareForNewScenario::receive(int receiver, int sender, ucharbuf &p)
    {
        logger::log(logger::DEBUG, "MessageSystem: Receiving a message of type PrepareForNewScenario (1006)\r\n");

        types::String scenarioCode;
        getstring(scenarioCode, p);

        lapi::state.get<lua::Function>("LAPI", "GUI", "show_message")("Server", "Map being prepared on the server, please wait ..");
        ClientSystem::prepareForNewScenario(scenarioCode);
    }
#endif


// RequestCurrentScenario

    void send_RequestCurrentScenario()
    {
        logger::log(logger::DEBUG, "Sending a message of type RequestCurrentScenario (1007)\r\n");
        INDENT_LOG(logger::DEBUG);

        game::addmsg(1007, "r");
    }

#ifdef SERVER
    void RequestCurrentScenario::receive(int receiver, int sender, ucharbuf &p)
    {
        logger::log(logger::DEBUG, "MessageSystem: Receiving a message of type RequestCurrentScenario (1007)\r\n");


        if (world::scenario_code.is_empty()) return;
        world::send_curr_map(sender);
    }
#endif

// NotifyAboutCurrentScenario

    void send_NotifyAboutCurrentScenario(int clientNumber, const char* mid, const char* sc)
    {
        int exclude = -1; // Set this to clientNumber to not send to

        logger::log(logger::DEBUG, "Sending a message of type NotifyAboutCurrentScenario (1008)\r\n");
        INDENT_LOG(logger::DEBUG);

        int start, finish;
        if (clientNumber == -1)
        {
            // Send to all clients
            start  = 0;
            finish = getnumclients() - 1;
        } else {
            start  = clientNumber;
            finish = clientNumber;
        }

#ifdef SERVER
        int testUniqueId;
#endif
        for (clientNumber = start; clientNumber <= finish; clientNumber++)
        {
            if (clientNumber == exclude) continue;
#ifdef SERVER
            fpsent* fpsEntity = game::getclient(clientNumber);
            bool serverControlled = fpsEntity ? fpsEntity->serverControlled : false;

            testUniqueId = server::getUniqueId(clientNumber);
            if ( (!serverControlled && testUniqueId != DUMMY_SINGLETON_CLIENT_UNIQUE_ID) || // If a remote client, send even if negative (during login process)
                 (false && testUniqueId == DUMMY_SINGLETON_CLIENT_UNIQUE_ID) || // If need to send to dummy server, send there
                 (false && testUniqueId != DUMMY_SINGLETON_CLIENT_UNIQUE_ID && serverControlled) )  // If need to send to npcs, send there
#endif
            {
                #ifdef SERVER
                    logger::log(logger::DEBUG, "Sending to %d (%d) ((%d))\r\n", clientNumber, testUniqueId, serverControlled);
                #endif
                sendf(clientNumber, MAIN_CHANNEL, "riss", 1008, mid, sc);

            }
        }
    }

#ifdef CLIENT
    void NotifyAboutCurrentScenario::receive(int receiver, int sender, ucharbuf &p)
    {
        logger::log(logger::DEBUG, "MessageSystem: Receiving a message of type NotifyAboutCurrentScenario (1008)\r\n");

        types::String mid;
        getstring(mid, p);
        types::String sc;
        getstring(sc, p);

        ClientSystem::currScenarioCode = sc;
        world::set_map(mid);
    }
#endif


// RestartMap

    void send_RestartMap()
    {
        logger::log(logger::DEBUG, "Sending a message of type RestartMap (1009)\r\n");
        INDENT_LOG(logger::DEBUG);

        game::addmsg(1009, "r");
    }

#ifdef SERVER
    void RestartMap::receive(int receiver, int sender, ucharbuf &p)
    {
        logger::log(logger::DEBUG, "MessageSystem: Receiving a message of type RestartMap (1009)\r\n");


        if (world::scenario_code.is_empty()) return;
        if (!server::isAdmin(sender))
        {
            logger::log(logger::WARNING, "Non-admin tried to restart the map\r\n");
            send_PersonalServerMessage(sender, "Server", "You are not an administrator, and cannot restart the map");
            return;
        }
        world::restart_map();
    }
#endif

// NewEntityRequest

    void send_NewEntityRequest(const char* _class, float x, float y, float z, const char* stateData)
    {        EditingSystem::madeChanges = true;

        logger::log(logger::DEBUG, "Sending a message of type NewEntityRequest (1010)\r\n");
        INDENT_LOG(logger::DEBUG);

        game::addmsg(1010, "rsiiis", _class, int(x*DMF), int(y*DMF), int(z*DMF), stateData);
    }

#ifdef SERVER
    void NewEntityRequest::receive(int receiver, int sender, ucharbuf &p)
    {
        logger::log(logger::DEBUG, "MessageSystem: Receiving a message of type NewEntityRequest (1010)\r\n");

        types::String _class;
        getstring(_class, p);
        float x = float(getint(p))/DMF;
        float y = float(getint(p))/DMF;
        float z = float(getint(p))/DMF;
        types::String stateData;
        getstring(stateData, p);

        if (world::scenario_code.is_empty()) return;
        if (!server::isAdmin(sender))
        {
            logger::log(logger::WARNING, "Non-admin tried to add an entity\r\n");
            send_PersonalServerMessage(sender, "Server", "You are not an administrator, and cannot create entities");
            return;
        }
        // Validate class
        if (lapi::state.get<lua::Function>(
            "LAPI", "World", "Entities", "Classes", "get"
        ).call<lua::Object>(_class).is_nil()) return;
        // Add entity
        logger::log(logger::DEBUG, "Creating new entity, %s   %f,%f,%f   %s\r\n", _class.get_buf(), x, y, z, stateData.get_buf());
        if ( !server::isRunningCurrentScenario(sender) ) return; // Silently ignore info from previous scenario
        const char *sauerType = lapi::state.get<lua::Function>("LAPI", "World", "Entities", "Classes", "get_sauer_type").call<const char*>(_class);
        logger::log(logger::DEBUG, "Sauer type: %s\r\n", sauerType);
        // Create
        lua::Table t = lapi::state.new_table(0, 2);
        lua::Table v = lapi::state.new_table(0, 3);
        v["x"] = x; v["y"] = y; v["z"] = z;
        t["position"  ] = v;
        t["state_data"] = stateData;
        int newUniqueId = lapi::state.get<lua::Function>(
            "LAPI", "World", "Entities", "new"
        ).call<lua::Table>(_class, t).get<int>(lapi::state.get<lua::Object>("LAPI", "World", "Entity", "Properties", "id"));
        logger::log(logger::DEBUG, "Created Entity: %d - %s  (%f,%f,%f) \r\n",
                                      newUniqueId, _class.get_buf(), x, y, z);
    }
#endif

// StateDataUpdate

    void send_StateDataUpdate(int clientNumber, int uniqueId, int keyProtocolId, const char* value, int originalClientNumber)
    {
        int exclude = -1; // Set this to clientNumber to not send to

        logger::log(logger::DEBUG, "Sending a message of type StateDataUpdate (1011)\r\n");
        INDENT_LOG(logger::DEBUG);

                 exclude = originalClientNumber;


        int start, finish;
        if (clientNumber == -1)
        {
            // Send to all clients
            start  = 0;
            finish = getnumclients() - 1;
        } else {
            start  = clientNumber;
            finish = clientNumber;
        }

#ifdef SERVER
        int testUniqueId;
#endif
        for (clientNumber = start; clientNumber <= finish; clientNumber++)
        {
            if (clientNumber == exclude) continue;
#ifdef SERVER
            fpsent* fpsEntity = game::getclient(clientNumber);
            bool serverControlled = fpsEntity ? fpsEntity->serverControlled : false;

            testUniqueId = server::getUniqueId(clientNumber);
            if ( (!serverControlled && testUniqueId != DUMMY_SINGLETON_CLIENT_UNIQUE_ID) || // If a remote client, send even if negative (during login process)
                 (false && testUniqueId == DUMMY_SINGLETON_CLIENT_UNIQUE_ID) || // If need to send to dummy server, send there
                 (true && testUniqueId != DUMMY_SINGLETON_CLIENT_UNIQUE_ID && serverControlled) )  // If need to send to npcs, send there
#endif
            {
                #ifdef SERVER
                    logger::log(logger::DEBUG, "Sending to %d (%d) ((%d))\r\n", clientNumber, testUniqueId, serverControlled);
                #endif
                sendf(clientNumber, MAIN_CHANNEL, "riiisi", 1011, uniqueId, keyProtocolId, value, originalClientNumber);

            }
        }
    }

    void StateDataUpdate::receive(int receiver, int sender, ucharbuf &p)
    {
        logger::log(logger::DEBUG, "MessageSystem: Receiving a message of type StateDataUpdate (1011)\r\n");

        int uniqueId = getint(p);
        int keyProtocolId = getint(p);
        types::String value;
        getstring(value, p);
        int originalClientNumber = getint(p);

        #ifdef SERVER
            #define STATE_DATA_UPDATE \
                uniqueId = uniqueId;  /* Prevent warnings */ \
                keyProtocolId = keyProtocolId; \
                originalClientNumber = originalClientNumber; \
                return; /* We do send this to the NPCs sometimes, as it is sent during their creation (before they are fully */ \
                        /* registered even). But we have no need to process it on the server. */
        #else
            #define STATE_DATA_UPDATE \
                assert(originalClientNumber == -1 || ClientSystem::playerNumber != originalClientNumber); /* Can be -1, or else cannot be us */ \
                \
                logger::log(logger::DEBUG, "StateDataUpdate: %d, %d, %s \r\n", uniqueId, keyProtocolId, value.get_buf()); \
                \
                if (!LogicSystem::initialized) \
                    return; \
                \
                lapi::state.get<lua::Function>("LAPI", "World", "Entity", "set_state_data")(uniqueId, keyProtocolId, value);
        #endif
        STATE_DATA_UPDATE
    }


// StateDataChangeRequest

    void send_StateDataChangeRequest(int uniqueId, int keyProtocolId, const char* value)
    {        // This isn't a perfect way to differentiate transient state data changes from permanent ones
        // that justify saying 'changes were made', but for now it will do. Note that even checking
        // for changes to persistent entities is not enough - transient changes on them are generally
        // not expected to count as 'changes'. So this check, of editmode, is the best simple solution
        // there is - if you're in edit mode, the change counts as a 'real change', that you probably
        // want saved.
        // Note: We don't do this with unreliable messages, meaningless anyhow.
        if (editmode)
            EditingSystem::madeChanges = true;

        logger::log(logger::DEBUG, "Sending a message of type StateDataChangeRequest (1012)\r\n");
        INDENT_LOG(logger::DEBUG);

        game::addmsg(1012, "riis", uniqueId, keyProtocolId, value);
    }

#ifdef SERVER
    void StateDataChangeRequest::receive(int receiver, int sender, ucharbuf &p)
    {
        logger::log(logger::DEBUG, "MessageSystem: Receiving a message of type StateDataChangeRequest (1012)\r\n");

        int uniqueId = getint(p);
        int keyProtocolId = getint(p);
        types::String value;
        getstring(value, p);

        if (world::scenario_code.is_empty()) return;
        #define STATE_DATA_REQUEST \
        int actorUniqueId = server::getUniqueId(sender); \
        \
        logger::log(logger::DEBUG, "client %d requests to change %d to value: %s\r\n", actorUniqueId, keyProtocolId, value.get_buf()); \
        \
        if ( !server::isRunningCurrentScenario(sender) ) return; /* Silently ignore info from previous scenario */ \
        \
        lapi::state.get<lua::Function>("LAPI", "World", "Entity", "set_state_data")(uniqueId, keyProtocolId, value, actorUniqueId);
        STATE_DATA_REQUEST
    }
#endif

// UnreliableStateDataUpdate

    void send_UnreliableStateDataUpdate(int clientNumber, int uniqueId, int keyProtocolId, const char* value, int originalClientNumber)
    {
        int exclude = -1; // Set this to clientNumber to not send to

        logger::log(logger::DEBUG, "Sending a message of type UnreliableStateDataUpdate (1013)\r\n");
        INDENT_LOG(logger::DEBUG);

                 exclude = originalClientNumber;


        int start, finish;
        if (clientNumber == -1)
        {
            // Send to all clients
            start  = 0;
            finish = getnumclients() - 1;
        } else {
            start  = clientNumber;
            finish = clientNumber;
        }

#ifdef SERVER
        int testUniqueId;
#endif
        for (clientNumber = start; clientNumber <= finish; clientNumber++)
        {
            if (clientNumber == exclude) continue;
#ifdef SERVER
            fpsent* fpsEntity = game::getclient(clientNumber);
            bool serverControlled = fpsEntity ? fpsEntity->serverControlled : false;

            testUniqueId = server::getUniqueId(clientNumber);
            if ( (!serverControlled && testUniqueId != DUMMY_SINGLETON_CLIENT_UNIQUE_ID) || // If a remote client, send even if negative (during login process)
                 (false && testUniqueId == DUMMY_SINGLETON_CLIENT_UNIQUE_ID) || // If need to send to dummy server, send there
                 (true && testUniqueId != DUMMY_SINGLETON_CLIENT_UNIQUE_ID && serverControlled) )  // If need to send to npcs, send there
#endif
            {
                #ifdef SERVER
                    logger::log(logger::DEBUG, "Sending to %d (%d) ((%d))\r\n", clientNumber, testUniqueId, serverControlled);
                #endif
                sendf(clientNumber, MAIN_CHANNEL, "iiisi", 1013, uniqueId, keyProtocolId, value, originalClientNumber);

            }
        }
    }

    void UnreliableStateDataUpdate::receive(int receiver, int sender, ucharbuf &p)
    {
        logger::log(logger::DEBUG, "MessageSystem: Receiving a message of type UnreliableStateDataUpdate (1013)\r\n");

        int uniqueId = getint(p);
        int keyProtocolId = getint(p);
        types::String value;
        getstring(value, p);
        int originalClientNumber = getint(p);

        STATE_DATA_UPDATE
    }


// UnreliableStateDataChangeRequest

    void send_UnreliableStateDataChangeRequest(int uniqueId, int keyProtocolId, const char* value)
    {
        logger::log(logger::DEBUG, "Sending a message of type UnreliableStateDataChangeRequest (1014)\r\n");
        INDENT_LOG(logger::DEBUG);

        game::addmsg(1014, "iis", uniqueId, keyProtocolId, value);
    }

#ifdef SERVER
    void UnreliableStateDataChangeRequest::receive(int receiver, int sender, ucharbuf &p)
    {
        logger::log(logger::DEBUG, "MessageSystem: Receiving a message of type UnreliableStateDataChangeRequest (1014)\r\n");

        int uniqueId = getint(p);
        int keyProtocolId = getint(p);
        types::String value;
        getstring(value, p);

        if (world::scenario_code.is_empty()) return;
        STATE_DATA_REQUEST
    }
#endif

// NotifyNumEntities

    void send_NotifyNumEntities(int clientNumber, int num)
    {
        int exclude = -1; // Set this to clientNumber to not send to

        logger::log(logger::DEBUG, "Sending a message of type NotifyNumEntities (1015)\r\n");
        INDENT_LOG(logger::DEBUG);

         

        int start, finish;
        if (clientNumber == -1)
        {
            // Send to all clients
            start  = 0;
            finish = getnumclients() - 1;
        } else {
            start  = clientNumber;
            finish = clientNumber;
        }

#ifdef SERVER
        int testUniqueId;
#endif
        for (clientNumber = start; clientNumber <= finish; clientNumber++)
        {
            if (clientNumber == exclude) continue;
#ifdef SERVER
            fpsent* fpsEntity = game::getclient(clientNumber);
            bool serverControlled = fpsEntity ? fpsEntity->serverControlled : false;

            testUniqueId = server::getUniqueId(clientNumber);
            if ( (!serverControlled && testUniqueId != DUMMY_SINGLETON_CLIENT_UNIQUE_ID) || // If a remote client, send even if negative (during login process)
                 (false && testUniqueId == DUMMY_SINGLETON_CLIENT_UNIQUE_ID) || // If need to send to dummy server, send there
                 (false && testUniqueId != DUMMY_SINGLETON_CLIENT_UNIQUE_ID && serverControlled) )  // If need to send to npcs, send there
#endif
            {
                #ifdef SERVER
                    logger::log(logger::DEBUG, "Sending to %d (%d) ((%d))\r\n", clientNumber, testUniqueId, serverControlled);
                #endif
                sendf(clientNumber, MAIN_CHANNEL, "rii", 1015, num);

            }
        }
    }

#ifdef CLIENT
    void NotifyNumEntities::receive(int receiver, int sender, ucharbuf &p)
    {
        logger::log(logger::DEBUG, "MessageSystem: Receiving a message of type NotifyNumEntities (1015)\r\n");

        int num = getint(p);

        world::set_num_expected_entities(num);
    }
#endif


// AllActiveEntitiesSent

    void send_AllActiveEntitiesSent(int clientNumber)
    {
        int exclude = -1; // Set this to clientNumber to not send to

        logger::log(logger::DEBUG, "Sending a message of type AllActiveEntitiesSent (1016)\r\n");
        INDENT_LOG(logger::DEBUG);

         

        int start, finish;
        if (clientNumber == -1)
        {
            // Send to all clients
            start  = 0;
            finish = getnumclients() - 1;
        } else {
            start  = clientNumber;
            finish = clientNumber;
        }

#ifdef SERVER
        int testUniqueId;
#endif
        for (clientNumber = start; clientNumber <= finish; clientNumber++)
        {
            if (clientNumber == exclude) continue;
#ifdef SERVER
            fpsent* fpsEntity = game::getclient(clientNumber);
            bool serverControlled = fpsEntity ? fpsEntity->serverControlled : false;

            testUniqueId = server::getUniqueId(clientNumber);
            if ( (!serverControlled && testUniqueId != DUMMY_SINGLETON_CLIENT_UNIQUE_ID) || // If a remote client, send even if negative (during login process)
                 (false && testUniqueId == DUMMY_SINGLETON_CLIENT_UNIQUE_ID) || // If need to send to dummy server, send there
                 (false && testUniqueId != DUMMY_SINGLETON_CLIENT_UNIQUE_ID && serverControlled) )  // If need to send to npcs, send there
#endif
            {
                #ifdef SERVER
                    logger::log(logger::DEBUG, "Sending to %d (%d) ((%d))\r\n", clientNumber, testUniqueId, serverControlled);
                #endif
                sendf(clientNumber, MAIN_CHANNEL, "ri", 1016);

            }
        }
    }

#ifdef CLIENT
    void AllActiveEntitiesSent::receive(int receiver, int sender, ucharbuf &p)
    {
        logger::log(logger::DEBUG, "MessageSystem: Receiving a message of type AllActiveEntitiesSent (1016)\r\n");
        ClientSystem::finishLoadWorld();
    }
#endif


// ActiveEntitiesRequest

    void send_ActiveEntitiesRequest(const char* scenarioCode)
    {
        logger::log(logger::DEBUG, "Sending a message of type ActiveEntitiesRequest (1017)\r\n");
        INDENT_LOG(logger::DEBUG);

        game::addmsg(1017, "rs", scenarioCode);
    }

#ifdef SERVER
    void ActiveEntitiesRequest::receive(int receiver, int sender, ucharbuf &p)
    {
        logger::log(logger::DEBUG, "MessageSystem: Receiving a message of type ActiveEntitiesRequest (1017)\r\n");

        types::String scenarioCode;
        getstring(scenarioCode, p);

        #ifdef SERVER
            if (world::scenario_code.is_empty()) return;
            // Mark the client as running the current scenario, if indeed doing so
            server::setClientScenario(sender, scenarioCode.get_buf());
            if ( !server::isRunningCurrentScenario(sender) )
            {
                logger::log(logger::WARNING, "Client %d requested active entities for an invalid scenario: %s\r\n",
                    sender, scenarioCode.get_buf()
                );
                send_PersonalServerMessage(sender, "Invalid scenario", "An error occured in synchronizing scenarios");
                return;
            }
            lapi::state.get<lua::Function>("LAPI", "World", "Entities", "send")(sender);
            MessageSystem::send_AllActiveEntitiesSent(sender);
            lapi::state.get<lua::Function>("LAPI", "World", "Events", "Server", "player_login")(
                lapi::state.get<lua::Function>(
                    "LAPI", "World", "Entities", "get"
                ).call<lua::Object>(server::getUniqueId(sender))
            );
        #else // CLIENT
            // Send just enough info for the player's LE
            send_LogicEntityCompleteNotification( sender,
                                                  sender,
                                                  9999, // TODO: this same constant appears in multiple places
                                                  "player",
                                                  "{}" );
            MessageSystem::send_AllActiveEntitiesSent(sender);
        #endif
    }
#endif

// LogicEntityCompleteNotification

    void send_LogicEntityCompleteNotification(int clientNumber, int otherClientNumber, int otherUniqueId, const char* otherClass, const char* stateData)
    {
        int exclude = -1; // Set this to clientNumber to not send to

        logger::log(logger::DEBUG, "Sending a message of type LogicEntityCompleteNotification (1018)\r\n");
        INDENT_LOG(logger::DEBUG);

         

        int start, finish;
        if (clientNumber == -1)
        {
            // Send to all clients
            start  = 0;
            finish = getnumclients() - 1;
        } else {
            start  = clientNumber;
            finish = clientNumber;
        }

#ifdef SERVER
        int testUniqueId;
#endif
        for (clientNumber = start; clientNumber <= finish; clientNumber++)
        {
            if (clientNumber == exclude) continue;
#ifdef SERVER
            fpsent* fpsEntity = game::getclient(clientNumber);
            bool serverControlled = fpsEntity ? fpsEntity->serverControlled : false;

            testUniqueId = server::getUniqueId(clientNumber);
            if ( (!serverControlled && testUniqueId != DUMMY_SINGLETON_CLIENT_UNIQUE_ID) || // If a remote client, send even if negative (during login process)
                 (false && testUniqueId == DUMMY_SINGLETON_CLIENT_UNIQUE_ID) || // If need to send to dummy server, send there
                 (true && testUniqueId != DUMMY_SINGLETON_CLIENT_UNIQUE_ID && serverControlled) )  // If need to send to npcs, send there
#endif
            {
                #ifdef SERVER
                    logger::log(logger::DEBUG, "Sending to %d (%d) ((%d))\r\n", clientNumber, testUniqueId, serverControlled);
                #endif
                sendf(clientNumber, MAIN_CHANNEL, "riiiss", 1018, otherClientNumber, otherUniqueId, otherClass, stateData);

            }
        }
    }

    void LogicEntityCompleteNotification::receive(int receiver, int sender, ucharbuf &p)
    {
        logger::log(logger::DEBUG, "MessageSystem: Receiving a message of type LogicEntityCompleteNotification (1018)\r\n");

        int otherClientNumber = getint(p);
        int otherUniqueId = getint(p);
        types::String otherClass;
        getstring(otherClass, p);
        types::String stateData;
        getstring(stateData, p);

        #ifdef SERVER
            return; // We do send this to the NPCs sometimes, as it is sent during their creation (before they are fully
                    // registered even). But we have no need to process it on the server.
        #endif
        if (!LogicSystem::initialized)
            return;
        logger::log(logger::DEBUG, "RECEIVING LE: %d,%d,%s\r\n", otherClientNumber, otherUniqueId, otherClass.get_buf());
        INDENT_LOG(logger::DEBUG);
        // If a logic entity does not yet exist, create one
        CLogicEntity *entity = LogicSystem::getLogicEntity(otherUniqueId);
        if (entity == NULL)
        {
            lua::Table t = lapi::state.new_table();
            if (otherClientNumber >= 0) // If this is another client, NPC, etc., then send the clientnumber, critical for setup
            {
                #ifdef CLIENT
                    // If this is the player, validate it is the clientNumber we already have
                    if (otherUniqueId == ClientSystem::uniqueId)
                    {
                        logger::log(logger::DEBUG, "This is the player's entity (%d), validating client num: %d,%d\r\n",
                            otherUniqueId, otherClientNumber, ClientSystem::playerNumber);
                        assert(otherClientNumber == ClientSystem::playerNumber);
                    }
                #endif
                t[lapi::state.get<lua::Object>("LAPI", "World", "Entity", "Properties", "cn")] = otherClientNumber;
            }
            lapi::state.get<lua::Function>("LAPI", "World", "Entities", "add")(otherClass, otherUniqueId, t);
            entity = LogicSystem::getLogicEntity(otherUniqueId);
            if (!entity)
            {
                logger::log(logger::ERROR, "Received a LogicEntityCompleteNotification for a LogicEntity that cannot be created: %d - %s. Ignoring\r\n", otherUniqueId, otherClass.get_buf());
                return;
            }
        } else
            logger::log(logger::DEBUG, "Existing LogicEntity %d,%d,%d, no need to create\r\n", entity != NULL, entity->getUniqueId(),
                                            otherUniqueId);
        // A logic entity now exists (either one did before, or we created one), we now update the stateData, if we
        // are remotely connected (TODO: make this not segfault for localconnect)
        logger::log(logger::DEBUG, "Updating stateData with: %s\r\n", stateData.get_buf());
        entity->lua_ref.get<lua::Function>("update_complete_state_data")(entity->lua_ref, stateData);
        #ifdef CLIENT
            // If this new entity is in fact the Player's entity, then we finally have the player's LE, and can link to it.
            if (otherUniqueId == ClientSystem::uniqueId)
            {
                logger::log(logger::DEBUG, "Linking player information, uid: %d\r\n", otherUniqueId);
                // Note in C++
                ClientSystem::playerLogicEntity = LogicSystem::getLogicEntity(ClientSystem::uniqueId);
                // Note in lua
                lapi::state.get<lua::Function>("LAPI", "World", "Entity", "make_player")(ClientSystem::uniqueId);
            }
        #endif
        // Events post-reception
        world::trigger_received_entity();
    }


// RequestLogicEntityRemoval

    void send_RequestLogicEntityRemoval(int uniqueId)
    {        EditingSystem::madeChanges = true;

        logger::log(logger::DEBUG, "Sending a message of type RequestLogicEntityRemoval (1019)\r\n");
        INDENT_LOG(logger::DEBUG);

        game::addmsg(1019, "ri", uniqueId);
    }

#ifdef SERVER
    void RequestLogicEntityRemoval::receive(int receiver, int sender, ucharbuf &p)
    {
        logger::log(logger::DEBUG, "MessageSystem: Receiving a message of type RequestLogicEntityRemoval (1019)\r\n");

        int uniqueId = getint(p);

        if (world::scenario_code.is_empty()) return;
        if (!server::isAdmin(sender))
        {
            logger::log(logger::WARNING, "Non-admin tried to remove an entity\r\n");
            send_PersonalServerMessage(sender, "Server", "You are not an administrator, and cannot remove entities");
            return;
        }
        if ( !server::isRunningCurrentScenario(sender) ) return; // Silently ignore info from previous scenario
        lapi::state.get<lua::Function>("LAPI", "World", "Entities", "delete")(uniqueId);
    }
#endif

// LogicEntityRemoval

    void send_LogicEntityRemoval(int clientNumber, int uniqueId)
    {
        int exclude = -1; // Set this to clientNumber to not send to

        logger::log(logger::DEBUG, "Sending a message of type LogicEntityRemoval (1020)\r\n");
        INDENT_LOG(logger::DEBUG);

         

        int start, finish;
        if (clientNumber == -1)
        {
            // Send to all clients
            start  = 0;
            finish = getnumclients() - 1;
        } else {
            start  = clientNumber;
            finish = clientNumber;
        }

#ifdef SERVER
        int testUniqueId;
#endif
        for (clientNumber = start; clientNumber <= finish; clientNumber++)
        {
            if (clientNumber == exclude) continue;
#ifdef SERVER
            fpsent* fpsEntity = game::getclient(clientNumber);
            bool serverControlled = fpsEntity ? fpsEntity->serverControlled : false;

            testUniqueId = server::getUniqueId(clientNumber);
            if ( (!serverControlled && testUniqueId != DUMMY_SINGLETON_CLIENT_UNIQUE_ID) || // If a remote client, send even if negative (during login process)
                 (false && testUniqueId == DUMMY_SINGLETON_CLIENT_UNIQUE_ID) || // If need to send to dummy server, send there
                 (false && testUniqueId != DUMMY_SINGLETON_CLIENT_UNIQUE_ID && serverControlled) )  // If need to send to npcs, send there
#endif
            {
                #ifdef SERVER
                    logger::log(logger::DEBUG, "Sending to %d (%d) ((%d))\r\n", clientNumber, testUniqueId, serverControlled);
                #endif
                sendf(clientNumber, MAIN_CHANNEL, "rii", 1020, uniqueId);

            }
        }
    }

#ifdef CLIENT
    void LogicEntityRemoval::receive(int receiver, int sender, ucharbuf &p)
    {
        logger::log(logger::DEBUG, "MessageSystem: Receiving a message of type LogicEntityRemoval (1020)\r\n");

        int uniqueId = getint(p);

        if (!LogicSystem::initialized)
            return;
        lapi::state.get<lua::Function>("LAPI", "World", "Entities", "delete")(uniqueId);
    }
#endif


// ExtentCompleteNotification

    void send_ExtentCompleteNotification(int clientNumber, int otherUniqueId, const char* otherClass, const char* stateData, float x, float y, float z, int attr1, int attr2, int attr3, int attr4, int attr5)
    {
        int exclude = -1; // Set this to clientNumber to not send to

        logger::log(logger::DEBUG, "Sending a message of type ExtentCompleteNotification (1021)\r\n");
        INDENT_LOG(logger::DEBUG);

         

        int start, finish;
        if (clientNumber == -1)
        {
            // Send to all clients
            start  = 0;
            finish = getnumclients() - 1;
        } else {
            start  = clientNumber;
            finish = clientNumber;
        }

#ifdef SERVER
        int testUniqueId;
#endif
        for (clientNumber = start; clientNumber <= finish; clientNumber++)
        {
            if (clientNumber == exclude) continue;
#ifdef SERVER
            fpsent* fpsEntity = game::getclient(clientNumber);
            bool serverControlled = fpsEntity ? fpsEntity->serverControlled : false;

            testUniqueId = server::getUniqueId(clientNumber);
            if ( (!serverControlled && testUniqueId != DUMMY_SINGLETON_CLIENT_UNIQUE_ID) || // If a remote client, send even if negative (during login process)
                 (false && testUniqueId == DUMMY_SINGLETON_CLIENT_UNIQUE_ID) || // If need to send to dummy server, send there
                 (false && testUniqueId != DUMMY_SINGLETON_CLIENT_UNIQUE_ID && serverControlled) )  // If need to send to npcs, send there
#endif
            {
                #ifdef SERVER
                    logger::log(logger::DEBUG, "Sending to %d (%d) ((%d))\r\n", clientNumber, testUniqueId, serverControlled);
                #endif
                sendf(clientNumber, MAIN_CHANNEL, "riissiiiiiiii", 1021, otherUniqueId, otherClass, stateData, int(x*DMF), int(y*DMF), int(z*DMF), attr1, attr2, attr3, attr4, attr5);

            }
        }
    }

#ifdef CLIENT
    void ExtentCompleteNotification::receive(int receiver, int sender, ucharbuf &p)
    {
        logger::log(logger::DEBUG, "MessageSystem: Receiving a message of type ExtentCompleteNotification (1021)\r\n");

        int otherUniqueId = getint(p);
        types::String otherClass;
        getstring(otherClass, p);
        types::String stateData;
        getstring(stateData, p);
        float x = float(getint(p))/DMF;
        float y = float(getint(p))/DMF;
        float z = float(getint(p))/DMF;
        int attr1 = getint(p);
        int attr2 = getint(p);
        int attr3 = getint(p);
        int attr4 = getint(p);
        int attr5 = getint(p);

        if (!LogicSystem::initialized)
            return;
        logger::log(logger::DEBUG, "RECEIVING Extent: %d,%s - %f,%f,%f  %d,%d,%d,%d\r\n", otherUniqueId, otherClass.get_buf(),
            x, y, z, attr1, attr2, attr3, attr4, attr5);
        INDENT_LOG(logger::DEBUG);
        // If a logic entity does not yet exist, create one
        CLogicEntity *entity = LogicSystem::getLogicEntity(otherUniqueId);
        if (entity == NULL)
        {
            logger::log(logger::DEBUG, "Creating new active LogicEntity\r\n");
            const char *sauerType = lapi::state.get<lua::Function>("LAPI", "World", "Entities", "Classes", "get_sauer_type").call<const char*>(otherClass);

            lua::Table t = lapi::state.new_table(0, 8);
            t["_type"] = findtype((char*)sauerType);
            t["x"] = x; t["y"] = y; t["z"] = z;
            t["attr1"] = attr1; t["attr2"] = attr2; t["attr3"] = attr3; t["attr4"] = attr4; t["attr5"] = attr5;
            lapi::state.get<lua::Function>("LAPI", "World", "Entities", "add")(otherClass, otherUniqueId, t);
            entity = LogicSystem::getLogicEntity(otherUniqueId);
            assert(entity != NULL);
        } else
            logger::log(logger::DEBUG, "Existing LogicEntity %d,%d,%d, no need to create\r\n", entity != NULL, entity->getUniqueId(),
                                            otherUniqueId);
        // A logic entity now exists (either one did before, or we created one), we now update the stateData, if we
        // are remotely connected (TODO: make this not segfault for localconnect)
        logger::log(logger::DEBUG, "Updating stateData\r\n");
        lapi::state.get<lua::Function>("LAPI", "World", "Entity", "update_complete_state_data")(entity->lua_ref, stateData);
        // Events post-reception
        world::trigger_received_entity();
    }
#endif


// InitS2C

    void send_InitS2C(int clientNumber, int explicitClientNumber, int protocolVersion)
    {
        int exclude = -1; // Set this to clientNumber to not send to

        logger::log(logger::DEBUG, "Sending a message of type InitS2C (1022)\r\n");
        INDENT_LOG(logger::DEBUG);

         

        int start, finish;
        if (clientNumber == -1)
        {
            // Send to all clients
            start  = 0;
            finish = getnumclients() - 1;
        } else {
            start  = clientNumber;
            finish = clientNumber;
        }

#ifdef SERVER
        int testUniqueId;
#endif
        for (clientNumber = start; clientNumber <= finish; clientNumber++)
        {
            if (clientNumber == exclude) continue;
#ifdef SERVER
            fpsent* fpsEntity = game::getclient(clientNumber);
            bool serverControlled = fpsEntity ? fpsEntity->serverControlled : false;

            testUniqueId = server::getUniqueId(clientNumber);
            if ( (!serverControlled && testUniqueId != DUMMY_SINGLETON_CLIENT_UNIQUE_ID) || // If a remote client, send even if negative (during login process)
                 (false && testUniqueId == DUMMY_SINGLETON_CLIENT_UNIQUE_ID) || // If need to send to dummy server, send there
                 (false && testUniqueId != DUMMY_SINGLETON_CLIENT_UNIQUE_ID && serverControlled) )  // If need to send to npcs, send there
#endif
            {
                #ifdef SERVER
                    logger::log(logger::DEBUG, "Sending to %d (%d) ((%d))\r\n", clientNumber, testUniqueId, serverControlled);
                #endif
                sendf(clientNumber, MAIN_CHANNEL, "riii", 1022, explicitClientNumber, protocolVersion);

            }
        }
    }

#ifdef CLIENT
    void InitS2C::receive(int receiver, int sender, ucharbuf &p)
    {
        logger::log(logger::DEBUG, "MessageSystem: Receiving a message of type InitS2C (1022)\r\n");

        int explicitClientNumber = getint(p);
        int protocolVersion = getint(p);

        logger::log(logger::DEBUG, "client.h: N_INITS2C gave us cn/protocol: %d/%d\r\n", explicitClientNumber, protocolVersion);
        if(protocolVersion != PROTOCOL_VERSION)
        {
            conoutf(CON_ERROR, "You are using a different network protocol (you: %d, server: %d)", PROTOCOL_VERSION, protocolVersion);
            disconnect();
            return;
        }
        #ifdef CLIENT
            fpsent *player1 = game::player1;
        #else
            assert(0);
            fpsent *player1 = NULL;
        #endif
        player1->clientnum = explicitClientNumber; // we are now fully connected
                                                   // Kripken: Well, sauer would be, we still need more...
        #ifdef CLIENT
        ClientSystem::login(explicitClientNumber); // Finish the login process, send server our user/pass. NPCs need not do this.
        #endif
    }
#endif


// SoundToServer

    void send_SoundToServer(int soundId)
    {
        logger::log(logger::DEBUG, "Sending a message of type SoundToServer (1023)\r\n");
        INDENT_LOG(logger::DEBUG);

        game::addmsg(1023, "i", soundId);
    }

#ifdef SERVER
    void SoundToServer::receive(int receiver, int sender, ucharbuf &p)
    {
        logger::log(logger::DEBUG, "MessageSystem: Receiving a message of type SoundToServer (1023)\r\n");

        int soundId = getint(p);

        if (world::scenario_code.is_empty()) return;
        if ( !server::isRunningCurrentScenario(sender) ) return; // Silently ignore info from previous scenario
        dynent* otherEntity = game::getclient(sender);
        if (otherEntity)
            send_SoundToClients(-1, soundId, sender);
    }
#endif

// SoundToClients

    void send_SoundToClients(int clientNumber, int soundId, int originalClientNumber)
    {
        int exclude = -1; // Set this to clientNumber to not send to

        logger::log(logger::DEBUG, "Sending a message of type SoundToClients (1024)\r\n");
        INDENT_LOG(logger::DEBUG);

                 exclude = originalClientNumber; // This is how to ensure we do not send back to the client who originally sent it


        int start, finish;
        if (clientNumber == -1)
        {
            // Send to all clients
            start  = 0;
            finish = getnumclients() - 1;
        } else {
            start  = clientNumber;
            finish = clientNumber;
        }

#ifdef SERVER
        int testUniqueId;
#endif
        for (clientNumber = start; clientNumber <= finish; clientNumber++)
        {
            if (clientNumber == exclude) continue;
#ifdef SERVER
            fpsent* fpsEntity = game::getclient(clientNumber);
            bool serverControlled = fpsEntity ? fpsEntity->serverControlled : false;

            testUniqueId = server::getUniqueId(clientNumber);
            if ( (!serverControlled && testUniqueId != DUMMY_SINGLETON_CLIENT_UNIQUE_ID) || // If a remote client, send even if negative (during login process)
                 (false && testUniqueId == DUMMY_SINGLETON_CLIENT_UNIQUE_ID) || // If need to send to dummy server, send there
                 (false && testUniqueId != DUMMY_SINGLETON_CLIENT_UNIQUE_ID && serverControlled) )  // If need to send to npcs, send there
#endif
            {
                #ifdef SERVER
                    logger::log(logger::DEBUG, "Sending to %d (%d) ((%d))\r\n", clientNumber, testUniqueId, serverControlled);
                #endif
                sendf(clientNumber, MAIN_CHANNEL, "iii", 1024, soundId, originalClientNumber);

            }
        }
    }

#ifdef CLIENT
    void SoundToClients::receive(int receiver, int sender, ucharbuf &p)
    {
        logger::log(logger::DEBUG, "MessageSystem: Receiving a message of type SoundToClients (1024)\r\n");

        int soundId = getint(p);
        int originalClientNumber = getint(p);

        assert(ClientSystem::playerNumber != originalClientNumber);
        dynent* player = game::getclient(originalClientNumber);
        if (!player)
        {
            if (originalClientNumber == -1) // Do not play sounds from nonexisting clients - would be odd
                playsound(soundId);
        }
        else
        {
            CLogicEntity *entity = LogicSystem::getLogicEntity( player );
            if (entity)
            {
                vec where = entity->getOrigin();
                playsound(soundId, &where);
            } // If no entity - but there should be, there is a player - do not play at all.
        }
    }
#endif


// MapSoundToClients

    void send_MapSoundToClients(int clientNumber, const char* soundName, int entityUniqueId)
    {
        int exclude = -1; // Set this to clientNumber to not send to

        logger::log(logger::DEBUG, "Sending a message of type MapSoundToClients (1025)\r\n");
        INDENT_LOG(logger::DEBUG);

         

        int start, finish;
        if (clientNumber == -1)
        {
            // Send to all clients
            start  = 0;
            finish = getnumclients() - 1;
        } else {
            start  = clientNumber;
            finish = clientNumber;
        }

#ifdef SERVER
        int testUniqueId;
#endif
        for (clientNumber = start; clientNumber <= finish; clientNumber++)
        {
            if (clientNumber == exclude) continue;
#ifdef SERVER
            fpsent* fpsEntity = game::getclient(clientNumber);
            bool serverControlled = fpsEntity ? fpsEntity->serverControlled : false;

            testUniqueId = server::getUniqueId(clientNumber);
            if ( (!serverControlled && testUniqueId != DUMMY_SINGLETON_CLIENT_UNIQUE_ID) || // If a remote client, send even if negative (during login process)
                 (false && testUniqueId == DUMMY_SINGLETON_CLIENT_UNIQUE_ID) || // If need to send to dummy server, send there
                 (false && testUniqueId != DUMMY_SINGLETON_CLIENT_UNIQUE_ID && serverControlled) )  // If need to send to npcs, send there
#endif
            {
                #ifdef SERVER
                    logger::log(logger::DEBUG, "Sending to %d (%d) ((%d))\r\n", clientNumber, testUniqueId, serverControlled);
                #endif
                sendf(clientNumber, MAIN_CHANNEL, "isi", 1025, soundName, entityUniqueId);

            }
        }
    }

#ifdef CLIENT
    void MapSoundToClients::receive(int receiver, int sender, ucharbuf &p)
    {
        logger::log(logger::DEBUG, "MessageSystem: Receiving a message of type MapSoundToClients (1025)\r\n");

        types::String soundName;
        getstring(soundName, p);
        int entityUniqueId = getint(p);

        CLogicEntity *entity = LogicSystem::getLogicEntity(entityUniqueId);
        if (entity)
        {
            extentity *e = entity->staticEntity;
            stopmapsound(e);
            if(camera1->o.dist(e->o) < e->attr2)
            {
                if(!e->visible) playmapsound(soundName.get_buf(), e, e->attr4, -1);
                else if(e->visible) stopmapsound(e);
            }
        }
    }
#endif


// SoundToClientsByName

    void send_SoundToClientsByName(int clientNumber, float x, float y, float z, const char* soundName, int originalClientNumber)
    {
        int exclude = -1; // Set this to clientNumber to not send to

        logger::log(logger::DEBUG, "Sending a message of type SoundToClientsByName (1026)\r\n");
        INDENT_LOG(logger::DEBUG);

                 exclude = originalClientNumber; // This is how to ensure we do not send back to the client who originally sent it


        int start, finish;
        if (clientNumber == -1)
        {
            // Send to all clients
            start  = 0;
            finish = getnumclients() - 1;
        } else {
            start  = clientNumber;
            finish = clientNumber;
        }

#ifdef SERVER
        int testUniqueId;
#endif
        for (clientNumber = start; clientNumber <= finish; clientNumber++)
        {
            if (clientNumber == exclude) continue;
#ifdef SERVER
            fpsent* fpsEntity = game::getclient(clientNumber);
            bool serverControlled = fpsEntity ? fpsEntity->serverControlled : false;

            testUniqueId = server::getUniqueId(clientNumber);
            if ( (!serverControlled && testUniqueId != DUMMY_SINGLETON_CLIENT_UNIQUE_ID) || // If a remote client, send even if negative (during login process)
                 (false && testUniqueId == DUMMY_SINGLETON_CLIENT_UNIQUE_ID) || // If need to send to dummy server, send there
                 (false && testUniqueId != DUMMY_SINGLETON_CLIENT_UNIQUE_ID && serverControlled) )  // If need to send to npcs, send there
#endif
            {
                #ifdef SERVER
                    logger::log(logger::DEBUG, "Sending to %d (%d) ((%d))\r\n", clientNumber, testUniqueId, serverControlled);
                #endif
                sendf(clientNumber, MAIN_CHANNEL, "iiiisi", 1026, int(x*DMF), int(y*DMF), int(z*DMF), soundName, originalClientNumber);

            }
        }
    }

#ifdef CLIENT
    void SoundToClientsByName::receive(int receiver, int sender, ucharbuf &p)
    {
        logger::log(logger::DEBUG, "MessageSystem: Receiving a message of type SoundToClientsByName (1026)\r\n");

        float x = float(getint(p))/DMF;
        float y = float(getint(p))/DMF;
        float z = float(getint(p))/DMF;
        types::String soundName;
        getstring(soundName, p);
        int originalClientNumber = getint(p);

        assert(ClientSystem::playerNumber != originalClientNumber);
        vec pos(x,y,z);
        if (pos.x || pos.y || pos.z)
            playsoundname(soundName.get_buf(), &pos);
        else
            playsoundname(soundName.get_buf());
    }
#endif


// SoundStopToClientsByName

    void send_SoundStopToClientsByName(int clientNumber, int volume, const char* soundName, int originalClientNumber)
    {
        int exclude = -1; // Set this to clientNumber to not send to

        logger::log(logger::DEBUG, "Sending a message of type SoundStopToClientsByName (1027)\r\n");
        INDENT_LOG(logger::DEBUG);

                 exclude = originalClientNumber; // This is how to ensure we do not send back to the client who originally sent it


        int start, finish;
        if (clientNumber == -1)
        {
            // Send to all clients
            start  = 0;
            finish = getnumclients() - 1;
        } else {
            start  = clientNumber;
            finish = clientNumber;
        }

#ifdef SERVER
        int testUniqueId;
#endif
        for (clientNumber = start; clientNumber <= finish; clientNumber++)
        {
            if (clientNumber == exclude) continue;
#ifdef SERVER
            fpsent* fpsEntity = game::getclient(clientNumber);
            bool serverControlled = fpsEntity ? fpsEntity->serverControlled : false;

            testUniqueId = server::getUniqueId(clientNumber);
            if ( (!serverControlled && testUniqueId != DUMMY_SINGLETON_CLIENT_UNIQUE_ID) || // If a remote client, send even if negative (during login process)
                 (false && testUniqueId == DUMMY_SINGLETON_CLIENT_UNIQUE_ID) || // If need to send to dummy server, send there
                 (false && testUniqueId != DUMMY_SINGLETON_CLIENT_UNIQUE_ID && serverControlled) )  // If need to send to npcs, send there
#endif
            {
                #ifdef SERVER
                    logger::log(logger::DEBUG, "Sending to %d (%d) ((%d))\r\n", clientNumber, testUniqueId, serverControlled);
                #endif
                sendf(clientNumber, MAIN_CHANNEL, "iisi", 1027, volume, soundName, originalClientNumber);

            }
        }
    }

#ifdef CLIENT
    void SoundStopToClientsByName::receive(int receiver, int sender, ucharbuf &p)
    {
        logger::log(logger::DEBUG, "MessageSystem: Receiving a message of type SoundStopToClientsByName (1027)\r\n");

        int volume = getint(p);
        types::String soundName;
        getstring(soundName, p);
        int originalClientNumber = getint(p);

        assert(ClientSystem::playerNumber != originalClientNumber);
        stopsoundbyid(getsoundid(soundName.get_buf(), volume));
    }
#endif


// EditModeC2S

    void send_EditModeC2S(int mode)
    {
        logger::log(logger::DEBUG, "Sending a message of type EditModeC2S (1028)\r\n");
        INDENT_LOG(logger::DEBUG);

        game::addmsg(1028, "ri", mode);
    }

#ifdef SERVER
    void EditModeC2S::receive(int receiver, int sender, ucharbuf &p)
    {
        logger::log(logger::DEBUG, "MessageSystem: Receiving a message of type EditModeC2S (1028)\r\n");

        int mode = getint(p);

        if (world::scenario_code.is_empty() || !server::isRunningCurrentScenario(sender) ) return;
        send_EditModeS2C(-1, sender, mode); // Relay
    }
#endif

// EditModeS2C

    void send_EditModeS2C(int clientNumber, int otherClientNumber, int mode)
    {
        int exclude = -1; // Set this to clientNumber to not send to

        logger::log(logger::DEBUG, "Sending a message of type EditModeS2C (1029)\r\n");
        INDENT_LOG(logger::DEBUG);

                 exclude = otherClientNumber;


        int start, finish;
        if (clientNumber == -1)
        {
            // Send to all clients
            start  = 0;
            finish = getnumclients() - 1;
        } else {
            start  = clientNumber;
            finish = clientNumber;
        }

#ifdef SERVER
        int testUniqueId;
#endif
        for (clientNumber = start; clientNumber <= finish; clientNumber++)
        {
            if (clientNumber == exclude) continue;
#ifdef SERVER
            fpsent* fpsEntity = game::getclient(clientNumber);
            bool serverControlled = fpsEntity ? fpsEntity->serverControlled : false;

            testUniqueId = server::getUniqueId(clientNumber);
            if ( (!serverControlled && testUniqueId != DUMMY_SINGLETON_CLIENT_UNIQUE_ID) || // If a remote client, send even if negative (during login process)
                 (true && testUniqueId == DUMMY_SINGLETON_CLIENT_UNIQUE_ID) || // If need to send to dummy server, send there
                 (false && testUniqueId != DUMMY_SINGLETON_CLIENT_UNIQUE_ID && serverControlled) )  // If need to send to npcs, send there
#endif
            {
                #ifdef SERVER
                    logger::log(logger::DEBUG, "Sending to %d (%d) ((%d))\r\n", clientNumber, testUniqueId, serverControlled);
                #endif
                sendf(clientNumber, MAIN_CHANNEL, "riii", 1029, otherClientNumber, mode);

            }
        }
    }

    void EditModeS2C::receive(int receiver, int sender, ucharbuf &p)
    {
        logger::log(logger::DEBUG, "MessageSystem: Receiving a message of type EditModeS2C (1029)\r\n");

        int otherClientNumber = getint(p);
        int mode = getint(p);

        dynent* d = game::getclient(otherClientNumber);
        // Code from sauer's client.h
        if (d)
        {
            if (mode) 
            {
                d->editstate = d->state;
                d->state     = CS_EDITING;
            }
            else 
            {
                d->state = d->editstate;
            }
        }
    }


// RequestMap

    void send_RequestMap()
    {
        logger::log(logger::DEBUG, "Sending a message of type RequestMap (1030)\r\n");
        INDENT_LOG(logger::DEBUG);

        game::addmsg(1030, "r");
    }

#ifdef SERVER
    void RequestMap::receive(int receiver, int sender, ucharbuf &p)
    {
        logger::log(logger::DEBUG, "MessageSystem: Receiving a message of type RequestMap (1030)\r\n");


        if (world::scenario_code.is_empty()) return;
        world::send_curr_map(sender);
    }
#endif

// DoClick

    void send_DoClick(int button, int down, float x, float y, float z, int uniqueId)
    {
        logger::log(logger::DEBUG, "Sending a message of type DoClick (1031)\r\n");
        INDENT_LOG(logger::DEBUG);

        game::addmsg(1031, "riiiiii", button, down, int(x*DMF), int(y*DMF), int(z*DMF), uniqueId);
    }

#ifdef SERVER
    void DoClick::receive(int receiver, int sender, ucharbuf &p)
    {
        logger::log(logger::DEBUG, "MessageSystem: Receiving a message of type DoClick (1031)\r\n");

        int button = getint(p);
        int down = getint(p);
        float x = float(getint(p))/DMF;
        float y = float(getint(p))/DMF;
        float z = float(getint(p))/DMF;
        int uniqueId = getint(p);

        if (world::scenario_code.is_empty()) return;
        if ( !server::isRunningCurrentScenario(sender) ) return; // Silently ignore info from previous scenario
        lua::Function click(lapi::state.get<lua::Object>("LAPI", "Input", "Events", "Server", "click"));
        if (uniqueId != -1)
        {
            CLogicEntity *entity = LogicSystem::getLogicEntity(uniqueId);
            if (entity) click(button, down, vec(x, y, z), entity->lua_ref);
        }
        else click(button, down, vec(x, y, z));
    }
#endif

// ParticleSplashToClients

    void send_ParticleSplashToClients(int clientNumber, int _type, int num, int fade, float x, float y, float z)
    {
        int exclude = -1; // Set this to clientNumber to not send to

        logger::log(logger::DEBUG, "Sending a message of type ParticleSplashToClients (1032)\r\n");
        INDENT_LOG(logger::DEBUG);

         

        int start, finish;
        if (clientNumber == -1)
        {
            // Send to all clients
            start  = 0;
            finish = getnumclients() - 1;
        } else {
            start  = clientNumber;
            finish = clientNumber;
        }

#ifdef SERVER
        int testUniqueId;
#endif
        for (clientNumber = start; clientNumber <= finish; clientNumber++)
        {
            if (clientNumber == exclude) continue;
#ifdef SERVER
            fpsent* fpsEntity = game::getclient(clientNumber);
            bool serverControlled = fpsEntity ? fpsEntity->serverControlled : false;

            testUniqueId = server::getUniqueId(clientNumber);
            if ( (!serverControlled && testUniqueId != DUMMY_SINGLETON_CLIENT_UNIQUE_ID) || // If a remote client, send even if negative (during login process)
                 (false && testUniqueId == DUMMY_SINGLETON_CLIENT_UNIQUE_ID) || // If need to send to dummy server, send there
                 (false && testUniqueId != DUMMY_SINGLETON_CLIENT_UNIQUE_ID && serverControlled) )  // If need to send to npcs, send there
#endif
            {
                #ifdef SERVER
                    logger::log(logger::DEBUG, "Sending to %d (%d) ((%d))\r\n", clientNumber, testUniqueId, serverControlled);
                #endif
                sendf(clientNumber, MAIN_CHANNEL, "iiiiiii", 1032, _type, num, fade, int(x*DMF), int(y*DMF), int(z*DMF));

            }
        }
    }

#ifdef CLIENT
    void ParticleSplashToClients::receive(int receiver, int sender, ucharbuf &p)
    {
        logger::log(logger::DEBUG, "MessageSystem: Receiving a message of type ParticleSplashToClients (1032)\r\n");

        int _type = getint(p);
        int num = getint(p);
        int fade = getint(p);
        float x = float(getint(p))/DMF;
        float y = float(getint(p))/DMF;
        float z = float(getint(p))/DMF;

        vec pos(x,y,z);
        particle_splash(_type, num, fade, pos);
    }
#endif


// ParticleSplashRegularToClients

    void send_ParticleSplashRegularToClients(int clientNumber, int _type, int num, int fade, float x, float y, float z)
    {
        int exclude = -1; // Set this to clientNumber to not send to

        logger::log(logger::DEBUG, "Sending a message of type ParticleSplashRegularToClients (1033)\r\n");
        INDENT_LOG(logger::DEBUG);

         

        int start, finish;
        if (clientNumber == -1)
        {
            // Send to all clients
            start  = 0;
            finish = getnumclients() - 1;
        } else {
            start  = clientNumber;
            finish = clientNumber;
        }

#ifdef SERVER
        int testUniqueId;
#endif
        for (clientNumber = start; clientNumber <= finish; clientNumber++)
        {
            if (clientNumber == exclude) continue;
#ifdef SERVER
            fpsent* fpsEntity = game::getclient(clientNumber);
            bool serverControlled = fpsEntity ? fpsEntity->serverControlled : false;

            testUniqueId = server::getUniqueId(clientNumber);
            if ( (!serverControlled && testUniqueId != DUMMY_SINGLETON_CLIENT_UNIQUE_ID) || // If a remote client, send even if negative (during login process)
                 (false && testUniqueId == DUMMY_SINGLETON_CLIENT_UNIQUE_ID) || // If need to send to dummy server, send there
                 (false && testUniqueId != DUMMY_SINGLETON_CLIENT_UNIQUE_ID && serverControlled) )  // If need to send to npcs, send there
#endif
            {
                #ifdef SERVER
                    logger::log(logger::DEBUG, "Sending to %d (%d) ((%d))\r\n", clientNumber, testUniqueId, serverControlled);
                #endif
                sendf(clientNumber, MAIN_CHANNEL, "iiiiiii", 1033, _type, num, fade, int(x*DMF), int(y*DMF), int(z*DMF));

            }
        }
    }

#ifdef CLIENT
    void ParticleSplashRegularToClients::receive(int receiver, int sender, ucharbuf &p)
    {
        logger::log(logger::DEBUG, "MessageSystem: Receiving a message of type ParticleSplashRegularToClients (1033)\r\n");

        int _type = getint(p);
        int num = getint(p);
        int fade = getint(p);
        float x = float(getint(p))/DMF;
        float y = float(getint(p))/DMF;
        float z = float(getint(p))/DMF;

        vec pos(x,y,z);
        regular_particle_splash(_type, num, fade, pos);
    }
#endif


// RequestPrivateEditMode

    void send_RequestPrivateEditMode()
    {
        logger::log(logger::DEBUG, "Sending a message of type RequestPrivateEditMode (1034)\r\n");
        INDENT_LOG(logger::DEBUG);

        game::addmsg(1034, "r");
    }

#ifdef SERVER
    void RequestPrivateEditMode::receive(int receiver, int sender, ucharbuf &p)
    {
        logger::log(logger::DEBUG, "MessageSystem: Receiving a message of type RequestPrivateEditMode (1034)\r\n");


        if (world::scenario_code.is_empty()) return;
        send_NotifyPrivateEditMode(sender);
    }
#endif

// NotifyPrivateEditMode

    void send_NotifyPrivateEditMode(int clientNumber)
    {
        int exclude = -1; // Set this to clientNumber to not send to

        logger::log(logger::DEBUG, "Sending a message of type NotifyPrivateEditMode (1035)\r\n");
        INDENT_LOG(logger::DEBUG);

         

        int start, finish;
        if (clientNumber == -1)
        {
            // Send to all clients
            start  = 0;
            finish = getnumclients() - 1;
        } else {
            start  = clientNumber;
            finish = clientNumber;
        }

#ifdef SERVER
        int testUniqueId;
#endif
        for (clientNumber = start; clientNumber <= finish; clientNumber++)
        {
            if (clientNumber == exclude) continue;
#ifdef SERVER
            fpsent* fpsEntity = game::getclient(clientNumber);
            bool serverControlled = fpsEntity ? fpsEntity->serverControlled : false;

            testUniqueId = server::getUniqueId(clientNumber);
            if ( (!serverControlled && testUniqueId != DUMMY_SINGLETON_CLIENT_UNIQUE_ID) || // If a remote client, send even if negative (during login process)
                 (false && testUniqueId == DUMMY_SINGLETON_CLIENT_UNIQUE_ID) || // If need to send to dummy server, send there
                 (false && testUniqueId != DUMMY_SINGLETON_CLIENT_UNIQUE_ID && serverControlled) )  // If need to send to npcs, send there
#endif
            {
                #ifdef SERVER
                    logger::log(logger::DEBUG, "Sending to %d (%d) ((%d))\r\n", clientNumber, testUniqueId, serverControlled);
                #endif
                sendf(clientNumber, MAIN_CHANNEL, "ri", 1035);

            }
        }
    }

#ifdef CLIENT
    void NotifyPrivateEditMode::receive(int receiver, int sender, ucharbuf &p)
    {
        logger::log(logger::DEBUG, "MessageSystem: Receiving a message of type NotifyPrivateEditMode (1035)\r\n");


        conoutf("Server: You are now in private edit mode");
        ClientSystem::editingAlone = true;
    }
#endif


// Register all messages

void MessageManager::registerAll()
{
    registerMessageType( new PersonalServerMessage() );
    registerMessageType( new RequestServerMessageToAll() );
    registerMessageType( new LoginRequest() );
    registerMessageType( new YourUniqueId() );
    registerMessageType( new LoginResponse() );
    registerMessageType( new PrepareForNewScenario() );
    registerMessageType( new RequestCurrentScenario() );
    registerMessageType( new NotifyAboutCurrentScenario() );
    registerMessageType( new RestartMap() );
    registerMessageType( new NewEntityRequest() );
    registerMessageType( new StateDataUpdate() );
    registerMessageType( new StateDataChangeRequest() );
    registerMessageType( new UnreliableStateDataUpdate() );
    registerMessageType( new UnreliableStateDataChangeRequest() );
    registerMessageType( new NotifyNumEntities() );
    registerMessageType( new AllActiveEntitiesSent() );
    registerMessageType( new ActiveEntitiesRequest() );
    registerMessageType( new LogicEntityCompleteNotification() );
    registerMessageType( new RequestLogicEntityRemoval() );
    registerMessageType( new LogicEntityRemoval() );
    registerMessageType( new ExtentCompleteNotification() );
    registerMessageType( new InitS2C() );
    registerMessageType( new SoundToServer() );
    registerMessageType( new SoundToClients() );
    registerMessageType( new MapSoundToClients() );
    registerMessageType( new SoundToClientsByName() );
    registerMessageType( new SoundStopToClientsByName() );
    registerMessageType( new EditModeC2S() );
    registerMessageType( new EditModeS2C() );
    registerMessageType( new RequestMap() );
    registerMessageType( new DoClick() );
    registerMessageType( new ParticleSplashToClients() );
    registerMessageType( new ParticleSplashRegularToClients() );
    registerMessageType( new RequestPrivateEditMode() );
    registerMessageType( new NotifyPrivateEditMode() );
}

}

