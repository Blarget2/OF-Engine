// server.cpp: little more than enhanced multicaster
// runs dedicated or as client coroutine

#include "engine.h"

#include "game.h" // INTENSITY: needed for fpsent
 // INTENSITY
#include "network_system.h"
#include "message_system.h"
#include "of_world.h"

namespace server
{
    extern bool shutdown_if_empty;
    extern bool shutdown_if_idle;
    extern int  shutdown_idle_interval;
    int& getUniqueId(int clientNumber);
}
bool should_quit = false;

#define LOGSTRLEN 512

static FILE *logfile = NULL;

void closelogfile()
{
    if(logfile)
    {
        fclose(logfile);
        logfile = NULL;
    }
}

void setlogfile(const char *fname)
{
    closelogfile();
    if(fname && fname[0])
    {
        fname = findfile(fname, "w");
        if(fname) logfile = fopen(fname, "w");
    }
    setvbuf(logfile ? logfile : stdout, NULL, _IOLBF, BUFSIZ);
}

void logoutf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    logoutfv(fmt, args);
    va_end(args);
}

static void writelog(FILE *file, const char *fmt, va_list args)
{
    static char buf[LOGSTRLEN];
    static uchar ubuf[512];
    vformatstring(buf, fmt, args, sizeof(buf));
    int len = strlen(buf), carry = 0;
    while(carry < len)
    {
        int numu = encodeutf8(ubuf, sizeof(ubuf)-1, &((uchar *)buf)[carry], len - carry, &carry);
        if(carry >= len) ubuf[numu++] = '\n';
        fwrite(ubuf, 1, numu, file);
    }
}

void logoutfv(const char *fmt, va_list args)
{
    writelog(logfile ? logfile : stdout, fmt, args);
}

#ifdef STANDALONE
void fatal(const char *fmt, ...) 
{ 
    void cleanupserver();
    cleanupserver(); 
    va_list args;
    va_start(args, fmt);
    if(logfile) logoutfv(fmt, args);
    fprintf(stderr, "server error: ");
    vfprintf(stderr, fmt, args);
    fputc('\n', stderr);
    va_end(args);
    closelogfile();
    exit(EXIT_FAILURE); 
}

void conoutfv(int type, const char *fmt, va_list args)
{
    string sf, sp;
    vformatstring(sf, fmt, args);
    filtertext(sp, sf);
    logoutf("%s", sp);
}

void conoutf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    conoutfv(CON_INFO, fmt, args);
    va_end(args);
}

void conoutf(int type, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    conoutfv(type, fmt, args);
    va_end(args);
}
#endif

// all network traffic is in 32bit ints, which are then compressed using the following simple scheme (assumes that most values are small).

template<class T>
static inline void putint_(T &p, int n)
{
    if(n<128 && n>-127) p.put(n);
    else if(n<0x8000 && n>=-0x8000) { p.put(0x80); p.put(n); p.put(n>>8); }
    else { p.put(0x81); p.put(n); p.put(n>>8); p.put(n>>16); p.put(n>>24); }
}
void putint(ucharbuf &p, int n) { putint_(p, n); }
void putint(packetbuf &p, int n) { putint_(p, n); }
void putint(vector<uchar> &p, int n) { putint_(p, n); }

int getint(ucharbuf &p)
{
    int c = (char)p.get();
    if(c==-128) { int n = p.get(); n |= char(p.get())<<8; return n; }
    else if(c==-127) { int n = p.get(); n |= p.get()<<8; n |= p.get()<<16; return n|(p.get()<<24); } 
    else return c;
}

// much smaller encoding for unsigned integers up to 28 bits, but can handle signed
template<class T>
static inline void putuint_(T &p, int n)
{
    if(n < 0 || n >= (1<<21))
    {
        p.put(0x80 | (n & 0x7F));
        p.put(0x80 | ((n >> 7) & 0x7F));
        p.put(0x80 | ((n >> 14) & 0x7F));
        p.put(n >> 21);
    }
    else if(n < (1<<7)) p.put(n);
    else if(n < (1<<14))
    {
        p.put(0x80 | (n & 0x7F));
        p.put(n >> 7);
    }
    else 
    { 
        p.put(0x80 | (n & 0x7F)); 
        p.put(0x80 | ((n >> 7) & 0x7F));
        p.put(n >> 14); 
    }
}
void putuint(ucharbuf &p, int n) { putuint_(p, n); }
void putuint(packetbuf &p, int n) { putuint_(p, n); }
void putuint(vector<uchar> &p, int n) { putuint_(p, n); }

int getuint(ucharbuf &p)
{
    int n = p.get();
    if(n & 0x80)
    {
        n += (p.get() << 7) - 0x80;
        if(n & (1<<14)) n += (p.get() << 14) - (1<<14);
        if(n & (1<<21)) n += (p.get() << 21) - (1<<21);
        if(n & (1<<28)) n |= -1<<28;
    }
    return n;
}

template<class T>
static inline void putfloat_(T &p, float f)
{
    lilswap(&f, 1);
    p.put((uchar *)&f, sizeof(float));
}
void putfloat(ucharbuf &p, float f) { putfloat_(p, f); }
void putfloat(packetbuf &p, float f) { putfloat_(p, f); }
void putfloat(vector<uchar> &p, float f) { putfloat_(p, f); }

float getfloat(ucharbuf &p)
{
    float f;
    p.get((uchar *)&f, sizeof(float));
    return lilswap(f);
}

template<class T>
static inline void sendstring_(const char *t, T &p)
{
    while(*t) putint(p, *t++);
    putint(p, 0);
}
void sendstring(const char *t, ucharbuf &p) { sendstring_(t, p); }
void sendstring(const char *t, packetbuf &p) { sendstring_(t, p); }
void sendstring(const char *t, vector<uchar> &p) { sendstring_(t, p); }

void getstring(types::String& text, ucharbuf &p, int len)
{
    text.resize(len);

    char *it = text.begin();
    char *lt = text.begin() + text.capacity();
    do
    {
        if (it > lt)
        {
            text[len - 1] = '\0';
            return;
        }
        if (!p.remaining())
        {
            *it = '\0';
            return;
        } 
        *it = getint(p);
    } while (*it++);

    /* length update - ugly hack, FIXME */
    text.format("%s", text.get_buf());
}

void filtertext(char *dst, const char *src, bool whitespace, int len)
{
    for(int c = uchar(*src); c; c = uchar(*++src))
    {
        if(c == '\f')
        {
            if(!*++src) break;
            continue;
        }
        if(iscubeprint(c) || (iscubespace(c) && whitespace))
        {
            *dst++ = c;
            if(!--len) break;
        }
    }
    *dst = '\0';
}

enum { ST_EMPTY, ST_LOCAL, ST_TCPIP };

struct client                   // server side version of "dynent" type
{
    int type;
    int num;
    ENetPeer *peer;
    string hostname;
    void *info;
};

vector<client *> clients;

ENetHost *serverhost = NULL;
int laststatus = 0; 
ENetSocket pongsock = ENET_SOCKET_NULL, lansock = ENET_SOCKET_NULL;

int localclients = 0, nonlocalclients = 0;

bool hasnonlocalclients() { return nonlocalclients!=0; }
bool haslocalclients() { return localclients!=0; }

client &addclient(int type)
{
    client *c = NULL;
    loopv(clients) if(clients[i]->type==ST_EMPTY)
    {
        c = clients[i];
        break;
    }
    if(!c)
    {
        c = new client;
        c->num = clients.length();
        clients.add(c);
    }
    c->info = server::newclientinfo();
    c->type = type;
    switch(type)
    {
        case ST_TCPIP: nonlocalclients++; break;
        case ST_LOCAL: localclients++; break;
    }
    return *c;
}

void delclient(client *c)
{
    if(!c) return;
    switch(c->type)
    {
        case ST_TCPIP: nonlocalclients--; if(c->peer) c->peer->data = NULL; break;
        case ST_LOCAL: localclients--; break;
        case ST_EMPTY: return;
    }
    c->type = ST_EMPTY;
    c->peer = NULL;
    if(c->info)
    {
        server::deleteclientinfo(c->info);
        c->info = NULL;
    }
}

void cleanupserver()
{
    if(serverhost) enet_host_destroy(serverhost);
    serverhost = NULL;

    if(pongsock != ENET_SOCKET_NULL) enet_socket_destroy(pongsock);
    if(lansock != ENET_SOCKET_NULL) enet_socket_destroy(lansock);
    pongsock = lansock = ENET_SOCKET_NULL;
}

void process(ENetPacket *packet, int sender, int chan);
//void disconnect_client(int n, int reason);

void *getclientinfo(int i) { return !clients.inrange(i) || clients[i]->type==ST_EMPTY ? NULL : clients[i]->info; }
int getnumclients()        { return clients.length(); }
uint getclientip(int n)    { return clients.inrange(n) && clients[n]->type==ST_TCPIP ? clients[n]->peer->address.host : 0; }

void sendpacket(int n, int chan, ENetPacket *packet, int exclude)
{
    if(n<0)
    {
        if (getnumclients() <= 0) return; // INTENSITY: Added this, because otherwise sending to '-1' when there are no clients segfaults
        server::recordpacket(chan, packet->data, packet->dataLength);
        loopv(clients) if(i!=exclude && server::allowbroadcast(i)) sendpacket(i, chan, packet);
        return;
    }
    switch(clients[n]->type)
    {
        case ST_TCPIP:
        {
            enet_peer_send(clients[n]->peer, chan, packet);

            //NetworkSystem::Cataloger::packetSent(chan, packet->dataLength); // INTENSITY

            break;
        }

#ifndef STANDALONE
        case ST_LOCAL:
            localservertoclient(chan, packet);
            break;
#endif
    }
}

ENetPacket *sendf(int cn, int chan, const char *format, ...)
{
    int exclude = -1;
    bool reliable = false;
    if(*format=='r') { reliable = true; ++format; }
    packetbuf p(MAXTRANS, reliable ? ENET_PACKET_FLAG_RELIABLE : 0);
    va_list args;
    va_start(args, format);
    while(*format) switch(*format++)
    {
        case 'x':
            exclude = va_arg(args, int);
            break;

        case 'v':
        {
            int n = va_arg(args, int);
            int *v = va_arg(args, int *);
            loopi(n) putint(p, v[i]);
            break;
        }

        case 'i': 
        {
            int n = isdigit(*format) ? *format++-'0' : 1;
            loopi(n) putint(p, va_arg(args, int));
            break;
        }
        case 'f':
        {
            int n = isdigit(*format) ? *format++-'0' : 1;
            loopi(n) putfloat(p, (float)va_arg(args, double));
            break;
        }
        case 's': sendstring(va_arg(args, const char *), p); break;
        case 'm':
        {
            int n = va_arg(args, int);
            p.put(va_arg(args, uchar *), n);
            break;
        }
    }
    va_end(args);
    ENetPacket *packet = p.finalize();
    sendpacket(cn, chan, packet, exclude);
    return packet->referenceCount > 0 ? packet : NULL;
}

const char *disc_reasons[] = { "normal", "end of packet", "client num", "kicked/banned", "tag type", "ip is banned", "server is in private mode", "server FULL", "connection timed out", "overflow" };

void disconnect_client(int n, int reason)
{
    if(!clients.inrange(n) || clients[n]->type!=ST_TCPIP) return;
    enet_peer_disconnect(clients[n]->peer, reason);
    server::clientdisconnect(n);
    delclient(clients[n]);
    defformatstring(s)("client (%s) disconnected because: %s", clients[n]->hostname, disc_reasons[reason]);
    logoutf("%s", s);
    server::sendservmsg(s);
}

void kicknonlocalclients(int reason)
{
    loopv(clients) if(clients[i]->type==ST_TCPIP) disconnect_client(i, reason);
}

void process(ENetPacket *packet, int sender, int chan)   // sender may be -1
{
    packetbuf p(packet);
    server::parsepacket(sender, chan, p);
    if(p.overread()) { disconnect_client(sender, DISC_EOP); return; }
}

void localclienttoserver(int chan, ENetPacket *packet, int cn) // INTENSITY: Added cn
{
    client *c = NULL;
    if (cn == -1) // INTENSITY
    {
        loopv(clients) if(clients[i]->type==ST_LOCAL) { c = clients[i]; break; }
    } else {
        c = clients[cn]; // INTENSITY
    }

    if(c) process(packet, c->num, chan);
}

static ENetAddress pongaddr;

void sendserverinforeply(ucharbuf &p)
{
    ENetBuffer buf;
    buf.data = p.buf;
    buf.dataLength = p.length();
    enet_socket_send(pongsock, &pongaddr, &buf, 1);
}

#define DEFAULTCLIENTS 6

int uprate = 0, maxclients = DEFAULTCLIENTS;
const char *ip = "";

#if defined(STANDALONE) || defined(SERVER) // INTENSITY: Added server
int curtime = 0, lastmillis = 0, totalmillis = 0;
#endif

VAR(serveruprate, 0, 0, INT_MAX);
SVAR(serverip, "");
VARF(serverport, 0, server::serverport(), 0xFFFF, { if(!serverport) serverport = server::serverport(); });

uint totalsecs = 0;

void updatetime()
{
    static int lastsec = 0;
    if(totalmillis - lastsec >= 1000) 
    {
        int cursecs = (totalmillis - lastsec) / 1000;
        totalsecs += cursecs;
        lastsec += cursecs * 1000;
    }
}

void serverslice(bool dedicated, uint timeout)   // main server update, called from main loop in sp, or from below in dedicated server
{
    if(!serverhost) 
    {
        server::serverupdate();
        server::sendpackets();
        return;
    }
       
    // below is network only

    if(dedicated) 
    {
        int millis = (int)enet_time_get();
        curtime = millis - totalmillis;
        lastmillis = totalmillis = millis;
        updatetime();
    }
    server::serverupdate();
    
    ENetEvent event;
    bool serviced = false;
    while(!serviced)
    {
        if(enet_host_check_events(serverhost, &event) <= 0)
        {
            if(enet_host_service(serverhost, &event, timeout) <= 0) break;
            serviced = true;
        }
        switch(event.type)
        {
            case ENET_EVENT_TYPE_CONNECT:
            {
                client &c = addclient(ST_TCPIP);
                c.peer = event.peer;
                c.peer->data = &c;
                char hn[1024];
                copystring(c.hostname, (enet_address_get_host_ip(&c.peer->address, hn, sizeof(hn))==0) ? hn : "unknown");
                logoutf("client connected (%s)", c.hostname);
                int reason = server::clientconnect(c.num, c.peer->address.host);
                if(reason) disconnect_client(c.num, reason);
                break;
            }
            case ENET_EVENT_TYPE_RECEIVE:
            {
                client *c = (client *)event.peer->data;
                if(c) process(event.packet, c->num, event.channelID);
                if(event.packet->referenceCount==0) enet_packet_destroy(event.packet);
                break;
            }
            case ENET_EVENT_TYPE_DISCONNECT: 
            {
                client *c = (client *)event.peer->data;
                if(!c) break;
                logoutf("disconnected client (%s)", c->hostname);
                server::clientdisconnect(c->num);
                delclient(c);
                break;
            }
            default:
                break;
        }
    }
    if(server::sendpackets()) enet_host_flush(serverhost);
}

// INTENSITY: Added this, so we can flush out messages at will, e.g., login failure messages,
// which must be done before the next cycle as it might disconnect the other client before
// sending anything.
void force_network_flush()
{
    if (!serverhost)
    {
        logger::log(logger::ERROR, "Trying to force_flush, but no serverhost yet\r\n");
        return;
    }

//    if(sv->sendpackets())
        enet_host_flush(serverhost);
}

void flushserver(bool force)
{
    if(server::sendpackets(force) && serverhost) enet_host_flush(serverhost);
}

#ifndef STANDALONE
void localdisconnect(bool cleanup, int cn) // INTENSITY: Added cn
{
#ifdef CLIENT
    bool disconnected = false;
#endif
    loopv(clients) if(clients[i]->type==ST_LOCAL) 
    {
        if (cn != -1 && cn != clients[i]->num) continue; // INTENSITY: if cn given, only process that one
        server::localdisconnect(i);
        delclient(clients[i]);
#ifdef CLIENT
        disconnected = true;
#endif
    }

#ifdef CLIENT // INTENSITY: Added this
    if(!disconnected) return;
    game::gamedisconnect(cleanup);
    SETV(mainmenu, 1);
#endif
}

int localconnect() // INTENSITY: Added returning client num
{
    client &c = addclient(ST_LOCAL);
    copystring(c.hostname, "local");
    game::gameconnect(false);
    server::localconnect(c.num);
    return c.num; // INTENSITY: Added returning client num
}
#endif

void rundedicatedserver()
{
    #ifdef WIN32
    SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
    #endif
#if 0
    logoutf("dedicated server started, waiting for clients...");
    for(;;) serverslice(true, 5);
#endif
}

#ifdef WIN32
static char *parsecommandline(const char *src, vector<char *> &args)
{
    char *buf = new char[strlen(src) + 1], *dst = buf;
    for(;;)
    {
        while(isspace(*src)) src++;
        if(!*src) break;
        args.add(dst);
        for(bool quoted = false; *src && (quoted || !isspace(*src)); src++)
        {
            if(*src != '"') *dst++ = *src;
            else if(dst > buf && src[-1] == '\\') dst[-1] = '"';
            else quoted = !quoted;
        }
        *dst++ = '\0';
    }
    args.add(NULL);
    return buf;
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR szCmdLine, int sw)
{
    vector<char *> args;
    char *buf = parsecommandline(GetCommandLine(), args);

    SDL_SetModuleHandle(hInst);
    int status = SDL_main(args.length()-1, args.getbuf());

    delete[] buf;
    exit(status);
    return 0;
}
#endif

bool servererror(bool dedicated, const char *desc)
{
#ifndef STANDALONE
    if(!dedicated)
    {
        conoutf(CON_ERROR, desc);
        cleanupserver();
    }
    else
#endif
        fatal(desc);
    return false;
}
  
bool setuplistenserver(bool dedicated)
{
    ENetAddress address = { ENET_HOST_ANY, enet_uint16(serverport <= 0 ? server::serverport() : serverport) };
    if(serverip[0])
    {
        if(enet_address_set_host(&address, serverip)<0) conoutf(CON_WARN, "WARNING: server ip not resolved");
    }
    serverhost = enet_host_create(&address, min(maxclients + server::reserveclients(), MAXCLIENTS), server::numchannels(), 0, serveruprate);
    if(!serverhost)
    {
        // INTENSITY: Do *NOT* fatally quit on this error. It can lead to repeated restarts etc.
        // of the sort that standby mode is meant to prevent, but standby does not protect from this.
        // So, just wait to be manually restarted.
        //return servererror(dedicated, "could not create server host");
        logger::log(logger::ERROR, "***!!! could not create server host (awaiting manual restart) !!!***");
        return false;
    }
    loopi(maxclients) serverhost->peers[i].data = NULL;
    address.port = server::serverinfoport(serverport > 0 ? serverport : -1);
    return true;
}

void initserver(bool listen, bool dedicated)
{
    if (dedicated)
        lapi::state.do_file("server-init.lua");

    if(listen) setuplistenserver(dedicated);

    server::serverinit();

    if(listen)
    {
        if(dedicated) rundedicatedserver(); // never returns
#ifndef STANDALONE
        else conoutf("listen server started");
#endif
    }
}

bool serveroption(char *opt)
{
    switch(opt[1])
    {
        case 'u': SETV(serveruprate, atoi(opt+2)); return true;
        case 'c': maxclients = atoi(opt+2); return true;
        case 'i': SETVF(serverip, opt+2); return true;
        case 'j': SETVFN(serverport, atoi(opt+2)); return true; 
        default: return false;
    }
}

vector<const char *> gameargs;

#ifdef SERVER
void server_init()//int argc, char* argv[])
{
    setvbuf(stdout, NULL, _IOLBF, BUFSIZ);
    setlogfile(NULL);
    if(enet_initialize()<0) fatal("Unable to initialise network module");
    atexit(enet_deinitialize);
    enet_time_set(0);

    conoutf("Registering messages");
    MessageSystem::MessageManager::registerAll();

    // Init server
    initserver(true, true);

    // Generate 'dummy' singleton client. This is to whom we send position updates on the server so our internal
    // fpsclient is updates.

    localconnect();
    assert(clients.length() == 1); // Ensure noone else connected before

    fpsent* fpsEntity = game::newclient(0); // Create a new fpsclient for this client

    fpsEntity->serverControlled = true; // Mark this as not controlled by server, so we don't try to actually do anything with it
                                        // After all it doesn't really exist

    fpsEntity->uniqueId = DUMMY_SINGLETON_CLIENT_UNIQUE_ID;
    server::getUniqueId(0) = DUMMY_SINGLETON_CLIENT_UNIQUE_ID;
}

void serverkeepalive();

void server_runslice()
{
    /* Keep connection alive?
     * 
    clientkeepalive();
    serverkeepalive();*/

    serverslice(true, 5);

    time_t now = time(0);

    static time_t    total_time = 0;
    if (!total_time) total_time = now;

    curtime    = (long)(1000 * (now - total_time));
    total_time = now;

    if(lastmillis) game::updateworld();

    static time_t shutdown_idle_last_update = 0;
    if (!shutdown_idle_last_update)
         shutdown_idle_last_update = time(0);

    if (server::shutdown_if_idle && (time(0) - shutdown_idle_last_update) >= (server::shutdown_idle_interval))
    {
        if (clients.length() <= 1)
        {
            extern bool should_quit;
            should_quit = true;
        }
        shutdown_idle_last_update = time(0);
    }
}

int main(int argc, char **argv)
{
    // Pre-initializations
    static Texture dummyTexture;

    dummyTexture.name = (char*)"";
    dummyTexture.type = Texture::IMAGE;
    dummyTexture.w = 1;
    dummyTexture.h = 1;
    dummyTexture.xs = 1;
    dummyTexture.ys = 1;
    dummyTexture.bpp = 8;
    dummyTexture.clamp = 1;
    dummyTexture.mipmap = 0;
    dummyTexture.canreduce = 0;
    dummyTexture.id = -1;
    dummyTexture.alphamask = new uchar[100]; // Whatever

    notexture = &dummyTexture;

    setlogfile(NULL);

    char *loglevel  = (char*)"WARNING";
    char *map_asset = NULL;
    for(int i = 1; i < argc; i++)
    {
        if(argv[i][0]=='-') switch(argv[i][1])
        {
            case 'q': 
            {
                const char *dir = sethomedir(&argv[i][2]);
                if(dir) logoutf("Using home directory: %s", dir);
                break;
            }
            case 'g': logoutf("Setting logging level", &argv[i][2]); loglevel = &argv[i][2]; break;
            case 'm': logoutf("Setting map", &argv[i][2]); map_asset = &argv[i][2]; break;
            default:
            {
                if (!strcmp(argv[i], "-shutdown-if-empty"))
                    server::shutdown_if_empty = true;
                else if (!strcmp(argv[i], "-shutdown-if-idle"))
                    server::shutdown_if_idle = true;
                else if (!strcmp(argv[i], "-shutdown-idle-interval"))
                {
                    char *endptr = NULL;
                    int interval = strtol(argv[i + 1], &endptr, 10);
                    if (!endptr && interval) server::shutdown_idle_interval = interval;
                }
            }
        }
        else gameargs.add(argv[i]);
    }
    logger::setlevel(loglevel);

    if (!map_asset)
    {
        logger::log(logger::ERROR, "No map asset to run. Shutting down.");
        return 1;
    }

    lapi::init();
    server_init();

    logger::log(logger::DEBUG, "Running first slice.\n");
    while (!should_quit)
    {
        server_runslice();
        if (map_asset)
        {
            logger::log(logger::DEBUG, "Setting map to %s ..\n", map_asset);
            world::set_map(map_asset);
            map_asset = NULL;
        }
    }

    logger::log(logger::WARNING, "Stopping main server.");
    varsys::flush();

    return 0;
}
#endif

