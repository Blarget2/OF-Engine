// main.cpp: initialisation & main loop

#include "engine.h"

#include "client_system.h" // INTENSITY
#include "editing_system.h" // INTENSITY
#include "message_system.h"
#include "of_localserver.h"
#include "of_tools.h"

extern void cleargamma();

void cleanup()
{
    recorder::stop();
    cleanupserver();
    SDL_ShowCursor(1);
    SDL_WM_GrabInput(SDL_GRAB_OFF);
    cleargamma();
    freeocta(worldroot);
    varsys::clear();
    extern void clear_console(); clear_console();
    extern void clear_mdls();    clear_mdls();
    extern void clear_sound();   clear_sound();
    closelogfile();
    SDL_Quit();
}

void quit()                     // normal exit
{
    extern void writeinitcfg();
    writeinitcfg();

    abortconnect();
    disconnect();
    localdisconnect();
    tools::writecfg();
    cleanup();

    varsys::flush();

    exit(EXIT_SUCCESS);
}

void fatal(const char *s, ...)    // failure exit
{
    static int errors = 0;
    errors++;

    if(errors <= 2) // print up to one extra recursive error
    {
        defvformatstring(msg,s,s);
        logoutf("%s", msg);

        if(errors <= 1) // avoid recursion
        {
            if(SDL_WasInit(SDL_INIT_VIDEO))
            {
                SDL_ShowCursor(1);
                SDL_WM_GrabInput(SDL_GRAB_OFF);
                cleargamma();
            }
            #ifdef WIN32
                MessageBox(NULL, msg, "OctaForge fatal error", MB_OK|MB_SYSTEMMODAL);
            #endif
            SDL_Quit();
        }
    }

    exit(EXIT_FAILURE);
}

SDL_Surface *screen = NULL;

int curtime = 0, totalmillis = 1, lastmillis = 1;

dynent *player = NULL;

int initing = NOT_INITING;

bool initwarning(const char *desc, int level, int type)
{
    if(initing < level) 
    {
        addchange(desc, type);
        return true;
    }
    return false;
}

#define SCR_MINW 320
#define SCR_MINH 200
#define SCR_MAXW 10000
#define SCR_MAXH 10000
#define SCR_DEFAULTW 1024
#define SCR_DEFAULTH 768
VARF(scr_w, SCR_MINW, -1, SCR_MAXW, initwarning("screen resolution"));
VARF(scr_h, SCR_MINH, -1, SCR_MAXH, initwarning("screen resolution"));
VARF(colorbits, 0, 0, 32, initwarning("color depth"));
VARF(depthbits, 0, 0, 32, initwarning("depth-buffer precision"));
VARF(stencilbits, 0, 0, 32, initwarning("stencil-buffer precision"));
VARF(fsaa, -1, -1, 16, initwarning("anti-aliasing"));
VARF(vsync, -1, -1, 1, initwarning("vertical sync"));

void writeinitcfg()
{
    stream *f = openutf8file("init.lua", "w");
    if(!f) return;
    f->printf("-- automatically written on exit, DO NOT MODIFY\n-- modify settings in game\n");
    extern int fullscreen, shaderprecision, glineardepth, sound, soundchans, soundfreq, soundbufferlen;
    f->printf("EVAR.fullscreen = %d\n", fullscreen);
    f->printf("EVAR.scr_w = %d\n", scr_w);
    f->printf("EVAR.scr_h = %d\n", scr_h);
    f->printf("EVAR.colorbits = %d\n", colorbits);
    f->printf("EVAR.depthbits = %d\n", depthbits);
    f->printf("EVAR.stencilbits = %d\n", stencilbits);
    f->printf("EVAR.fsaa = %d\n", fsaa);
    f->printf("EVAR.vsync = %d\n", vsync);
    f->printf("EVAR.shaderprecision = %d\n", shaderprecision);
    f->printf("EVAR.glineardepth = %d\n", glineardepth);
    f->printf("EVAR.sound = %d\n", sound);
    f->printf("EVAR.soundchans = %d\n", soundchans);
    f->printf("EVAR.soundfreq = %d\n", soundfreq);
    f->printf("EVAR.soundbufferlen = %d\n", soundbufferlen);
    delete f;
}

static void getbackgroundres(int &w, int &h)
{
    float wk = 1, hk = 1;
    if(w < 1024) wk = 1024.0f/w;
    if(h < 768) hk = 768.0f/h;
    wk = hk = max(wk, hk);
    w = int(ceil(w*wk));
    h = int(ceil(h*hk));
}

string backgroundcaption = "";
Texture *backgroundmapshot = NULL;
string backgroundmapname = "";
char *backgroundmapinfo = NULL;

void restorebackground()
{
    if(renderedframe) return;
    renderbackground(backgroundcaption[0] ? backgroundcaption : NULL, backgroundmapshot, backgroundmapname[0] ? backgroundmapname : NULL, backgroundmapinfo, true);
}

void renderbackground(const char *caption, Texture *mapshot, const char *mapname, const char *mapinfo, bool restore, bool force)
{
    if(!inbetweenframes && !force) return;

    stopsounds(); // stop sounds while loading
 
    int w = screen->w, h = screen->h;
    getbackgroundres(w, h);
    gettextres(w, h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, w, h, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    defaultshader->set();
    glEnable(GL_TEXTURE_2D);

    static int lastupdate = -1, lastw = -1, lasth = -1;
    static float backgroundu = 0, backgroundv = 0;
#if 0
    static float detailu = 0, detailv = 0;
    static int numdecals = 0;
    static struct decal { float x, y, size; int side; } decals[12];
#endif
    if((renderedframe && !gui::mainmenu && lastupdate != lastmillis) || lastw != w || lasth != h)
    {
        lastupdate = lastmillis;
        lastw = w;
        lasth = h;

        backgroundu = rndscale(1);
        backgroundv = rndscale(1);
#if 0
        detailu = rndscale(1);
        detailv = rndscale(1);
        numdecals = sizeof(decals)/sizeof(decals[0]);
        numdecals = numdecals/3 + rnd((numdecals*2)/3 + 1);
        float maxsize = min(w, h)/16.0f;
        loopi(numdecals)
        {
            decal d = { rndscale(w), rndscale(h), maxsize/2 + rndscale(maxsize/2), rnd(2) };
            decals[i] = d;
        }
#endif
    }
    else if(lastupdate != lastmillis) lastupdate = lastmillis;

    loopi(restore ? 1 : 3)
    {
        glColor3f(1, 1, 1);
        settexture("data/textures/ui/background.png", 0);
        float bu = w*0.67f/256.0f + backgroundu, bv = h*0.67f/256.0f + backgroundv;
        glBegin(GL_TRIANGLE_STRIP);
        glTexCoord2f(0,  0);  glVertex2f(0, 0);
        glTexCoord2f(bu, 0);  glVertex2f(w, 0);
        glTexCoord2f(0,  bv); glVertex2f(0, h);
        glTexCoord2f(bu, bv); glVertex2f(w, h);
        glEnd();
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);
#if 0
        settexture("<premul>data/textures/ui/background_detail.png", 0);
        float du = w*0.8f/512.0f + detailu, dv = h*0.8f/512.0f + detailv;
        glBegin(GL_TRIANGLE_STRIP);
        glTexCoord2f(0,  0);  glVertex2f(0, 0);
        glTexCoord2f(du, 0);  glVertex2f(w, 0);
        glTexCoord2f(0,  dv); glVertex2f(0, h);
        glTexCoord2f(du, dv); glVertex2f(w, h);
        glEnd();
        settexture("<premul>data/textures/ui/background_decal.png", 3);
        glBegin(GL_QUADS);
        loopj(numdecals)
        {
            float hsz = decals[j].size, hx = clamp(decals[j].x, hsz, w-hsz), hy = clamp(decals[j].y, hsz, h-hsz), side = decals[j].side;
            glTexCoord2f(side,   0); glVertex2f(hx-hsz, hy-hsz);
            glTexCoord2f(1-side, 0); glVertex2f(hx+hsz, hy-hsz);
            glTexCoord2f(1-side, 1); glVertex2f(hx+hsz, hy+hsz);
            glTexCoord2f(side,   1); glVertex2f(hx-hsz, hy+hsz);
        }
        glEnd();
#endif
        float lh = 0.5f*min(w, h), lw = lh*2,
              lx = 0.5f*(w - lw), ly = 0.5f*(h*0.5f - lh);
        settexture(/*(maxtexsize ? min(maxtexsize, hwtexsize) : hwtexsize) >= 1024 && (screen->w > 1280 || screen->h > 800) ? "<premul>data/logo_1024.png" :*/ "<premul>data/textures/ui/logo.png", 3);
        glBegin(GL_TRIANGLE_STRIP);
        glTexCoord2f(0, 0); glVertex2f(lx,    ly);
        glTexCoord2f(1, 0); glVertex2f(lx+lw, ly);
        glTexCoord2f(0, 1); glVertex2f(lx,    ly+lh);
        glTexCoord2f(1, 1); glVertex2f(lx+lw, ly+lh);
        glEnd();

#if 0
        float bh = 0.1f*min(w, h), bw = bh*2,
              bx = w - 1.1f*bw, by = h - 1.1f*bh;
        settexture("<premul>data/textures/ui/cube2badge.png", 3);
        glBegin(GL_TRIANGLE_STRIP);
        glTexCoord2f(0, 0); glVertex2f(bx,    by);
        glTexCoord2f(1, 0); glVertex2f(bx+bw, by);
        glTexCoord2f(0, 1); glVertex2f(bx,    by+bh);
        glTexCoord2f(1, 1); glVertex2f(bx+bw, by+bh);
        glEnd();
#endif

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        if(caption)
        {
            int tw = text_width(caption);
            float tsz = 0.04f*min(w, h)/FONTH,
                  tx = 0.5f*(w - tw*tsz), ty = h - 0.075f*1.5f*min(w, h) - 1.25f*FONTH*tsz;
            glPushMatrix();
            glTranslatef(tx, ty, 0);
            glScalef(tsz, tsz, 1);
            draw_text(caption, 0, 0);
            glPopMatrix();
        }
        if(mapshot || mapname)
        {
            int infowidth = 12*FONTH;
            float sz = 0.35f*min(w, h), msz = (0.75f*min(w, h) - sz)/(infowidth + FONTH), x = 0.5f*(w-sz), y = ly+lh - sz/15;
            if(mapinfo)
            {
                int mw, mh;
                text_bounds(mapinfo, mw, mh, infowidth);
                x -= 0.5f*(mw*msz + FONTH*msz);
            }
            if(mapshot && mapshot!=notexture)
            {
                glBindTexture(GL_TEXTURE_2D, mapshot->id);
                glBegin(GL_TRIANGLE_STRIP);
                glTexCoord2f(0, 0); glVertex2f(x,    y);
                glTexCoord2f(1, 0); glVertex2f(x+sz, y);
                glTexCoord2f(0, 1); glVertex2f(x,    y+sz);
                glTexCoord2f(1, 1); glVertex2f(x+sz, y+sz);
                glEnd();
            }
            else
            {
                int qw, qh;
                text_bounds("?", qw, qh);
                float qsz = sz*0.5f/max(qw, qh);
                glPushMatrix();
                glTranslatef(x + 0.5f*(sz - qw*qsz), y + 0.5f*(sz - qh*qsz), 0);
                glScalef(qsz, qsz, 1);
                draw_text("?", 0, 0);
                glPopMatrix();
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            }        
            settexture("data/textures/ui/mapshot_frame.png", 3);
            glBegin(GL_TRIANGLE_STRIP);
            glTexCoord2f(0, 0); glVertex2f(x,    y);
            glTexCoord2f(1, 0); glVertex2f(x+sz, y);
            glTexCoord2f(0, 1); glVertex2f(x,    y+sz);
            glTexCoord2f(1, 1); glVertex2f(x+sz, y+sz);
            glEnd();
            if(mapname)
            {
                int tw = text_width(mapname);
                float tsz = sz/(8*FONTH),
                      tx = 0.9f*sz - tw*tsz, ty = 0.9f*sz - FONTH*tsz;
                if(tx < 0.1f*sz) { tsz = 0.1f*sz/tw; tx = 0.1f; }
                glPushMatrix();
                glTranslatef(x+tx, y+ty, 0);
                glScalef(tsz, tsz, 1);
                draw_text(mapname, 0, 0);
                glPopMatrix();
            }
            if(mapinfo)
            {
                glPushMatrix();
                glTranslatef(x+sz+FONTH*msz, y, 0);
                glScalef(msz, msz, 1);
                draw_text(mapinfo, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF, -1, infowidth);
                glPopMatrix();
            }
        }
        glDisable(GL_BLEND);
        if(!restore) swapbuffers();
    }
    glDisable(GL_TEXTURE_2D);

    if(!restore)
    {
        renderedframe = false;
        copystring(backgroundcaption, caption ? caption : "");
        backgroundmapshot = mapshot;
        copystring(backgroundmapname, mapname ? mapname : "");
        if(mapinfo != backgroundmapinfo)
        {
            DELETEA(backgroundmapinfo);
            if(mapinfo) backgroundmapinfo = newstring(mapinfo);
        }
    }
}

float loadprogress = 0;

void renderprogress(float bar, const char *text, GLuint tex, bool background)   // also used during loading
{
    if(!inbetweenframes || envmapping) return;

    clientkeepalive();      // make sure our connection doesn't time out while loading maps etc.
    
    renderbackground(NULL, backgroundmapshot, NULL, NULL, true, true); // INTENSITY

    #ifdef __APPLE__
    interceptkey(SDLK_UNKNOWN); // keep the event queue awake to avoid 'beachball' cursor
    #endif

    extern int sdl_backingstore_bug;
    if(background || sdl_backingstore_bug > 0) restorebackground();

    int w = screen->w, h = screen->h;
    getbackgroundres(w, h);
    gettextres(w, h);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, w, h, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glEnable(GL_TEXTURE_2D);
    defaultshader->set();
    glColor3f(1, 1, 1);

    float fh = 0.075f*min(w, h), fw = fh*10,
          fx = renderedframe ? w - fw - fh/4 : 0.5f*(w - fw), 
          fy = renderedframe ? fh/4 : h - fh*1.5f,
          fu1 = 0/512.0f, fu2 = 511/512.0f,
          fv1 = 0/64.0f, fv2 = 52/64.0f;

    glEnable(GL_BLEND); // INTENSITY: Moved to here, to cover loading_frame as well
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // INTENSITY: ditto

    settexture("data/textures/ui/loading_frame.png", 3);
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(fu1, fv1); glVertex2f(fx,    fy);
    glTexCoord2f(fu2, fv1); glVertex2f(fx+fw, fy);
    glTexCoord2f(fu1, fv2); glVertex2f(fx,    fy+fh);
    glTexCoord2f(fu2, fv2); glVertex2f(fx+fw, fy+fh);
    glEnd();

    float bw = fw*(511 - 2*17)/511.0f, bh = fh*20/52.0f,
          bx = fx + fw*17/511.0f, by = fy + fh*16/52.0f,
          bv1 = 0/32.0f, bv2 = 20/32.0f,
          su1 = 0/32.0f, su2 = 7/32.0f, sw = fw*7/511.0f,
          eu1 = 23/32.0f, eu2 = 30/32.0f, ew = fw*7/511.0f,
          mw = bw - sw - ew,
          ex = bx+sw + max(mw*bar, fw*7/511.0f);
    if(bar > 0)
    {
        settexture("data/textures/ui/loading_bar.png", 3);
        glBegin(GL_QUADS);
        glTexCoord2f(su1, bv1); glVertex2f(bx,    by);
        glTexCoord2f(su2, bv1); glVertex2f(bx+sw, by);
        glTexCoord2f(su2, bv2); glVertex2f(bx+sw, by+bh);
        glTexCoord2f(su1, bv2); glVertex2f(bx,    by+bh);

        glTexCoord2f(su2, bv1); glVertex2f(bx+sw, by);
        glTexCoord2f(eu1, bv1); glVertex2f(ex,    by);
        glTexCoord2f(eu1, bv2); glVertex2f(ex,    by+bh);
        glTexCoord2f(su2, bv2); glVertex2f(bx+sw, by+bh);

        glTexCoord2f(eu1, bv1); glVertex2f(ex,    by);
        glTexCoord2f(eu2, bv1); glVertex2f(ex+ew, by);
        glTexCoord2f(eu2, bv2); glVertex2f(ex+ew, by+bh);
        glTexCoord2f(eu1, bv2); glVertex2f(ex,    by+bh);
        glEnd();
    }
    else if (bar < 0) // INTENSITY: Show side-to-side progress for negative values (-0 to -infinity)
    {
        float width = 0.382; // 1-golden ratio 
        float start;
        bar = -bar;
        bar = fmod(bar, 1.0f);
        if (bar < 0.5)
            start = (bar*2)*(1-width);
        else
            start = 2*(1-bar)*(1-width);

        float bw = fw*(511 - 2*17)/511.0f, bh = fh*20/52.0f,
              bx = fx + fw*17/511.0f + mw*start, by = fy + fh*16/52.0f,
              bv1 = 0/32.0f, bv2 = 20/32.0f,
              su1 = 0/32.0f, su2 = 7/32.0f, sw = fw*7/511.0f,
              eu1 = 23/32.0f, eu2 = 30/32.0f, ew = fw*7/511.0f,
              mw = bw - sw - ew,
              ex = bx+sw + max(mw*width, fw*7/511.0f);

        settexture("data/textures/ui/loading_bar.png", 3);
        glBegin(GL_QUADS);
        glTexCoord2f(su1, bv1); glVertex2f(bx,    by);
        glTexCoord2f(su2, bv1); glVertex2f(bx+sw, by);
        glTexCoord2f(su2, bv2); glVertex2f(bx+sw, by+bh);
        glTexCoord2f(su1, bv2); glVertex2f(bx,    by+bh);

        glTexCoord2f(su2, bv1); glVertex2f(bx+sw, by);
        glTexCoord2f(eu1, bv1); glVertex2f(ex,    by);
        glTexCoord2f(eu1, bv2); glVertex2f(ex,    by+bh);
        glTexCoord2f(su2, bv2); glVertex2f(bx+sw, by+bh);

        glTexCoord2f(eu1, bv1); glVertex2f(ex,    by);
        glTexCoord2f(eu2, bv1); glVertex2f(ex+ew, by);
        glTexCoord2f(eu2, bv2); glVertex2f(ex+ew, by+bh);
        glTexCoord2f(eu1, bv2); glVertex2f(ex,    by+bh);
        glEnd();
    } // INTENSITY: End side-to-side progress


    if(text)
    {
        int tw = text_width(text);
        float tsz = bh*0.8f/FONTH;
        if(tw*tsz > mw) tsz = mw/tw;
        glPushMatrix();
        glTranslatef(bx+sw, by + (bh - FONTH*tsz)/2, 0);
        glScalef(tsz, tsz, 1);
        draw_text(text, 0, 0);
        glPopMatrix();
    }

    glDisable(GL_BLEND);

    if(tex)
    {
        glBindTexture(GL_TEXTURE_2D, tex);
        float sz = 0.35f*min(w, h), x = 0.5f*(w-sz), y = 0.5f*min(w, h) - sz/15;
        glBegin(GL_TRIANGLE_STRIP);
        glTexCoord2f(0, 0); glVertex2f(x,    y);
        glTexCoord2f(1, 0); glVertex2f(x+sz, y);
        glTexCoord2f(0, 1); glVertex2f(x,    y+sz);
        glTexCoord2f(1, 1); glVertex2f(x+sz, y+sz);
        glEnd();

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        settexture("data/textures/ui/mapshot_frame.png", 3);
        glBegin(GL_TRIANGLE_STRIP);
        glTexCoord2f(0, 0); glVertex2f(x,    y);
        glTexCoord2f(1, 0); glVertex2f(x+sz, y);
        glTexCoord2f(0, 1); glVertex2f(x,    y+sz);
        glTexCoord2f(1, 1); glVertex2f(x+sz, y+sz);
        glEnd();
        glDisable(GL_BLEND);
    }

    glDisable(GL_TEXTURE_2D);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    swapbuffers();
}

void keyrepeat(bool on)
{
    SDL_EnableKeyRepeat(on ? SDL_DEFAULT_REPEAT_DELAY : 0,
                             SDL_DEFAULT_REPEAT_INTERVAL);
}

static bool grabinput = false, minimized = false;

void inputgrab(bool on)
{
#ifndef WIN32
    if(!(screen->flags & SDL_FULLSCREEN)) SDL_WM_GrabInput(SDL_GRAB_OFF);
    else
#endif
    SDL_WM_GrabInput(on ? SDL_GRAB_ON : SDL_GRAB_OFF);
    SDL_ShowCursor(on ? SDL_DISABLE : SDL_ENABLE);
}

void setfullscreen(bool enable)
{
    if(!screen) return;
#if defined(WIN32) || defined(__APPLE__)
    initwarning(enable ? "fullscreen" : "windowed");
#else
    if(enable == !(screen->flags&SDL_FULLSCREEN))
    {
        SDL_WM_ToggleFullScreen(screen);
        inputgrab(grabinput);
    }
#endif
}

VARF(fullscreen, 0, 0, 1, setfullscreen(fullscreen!=0));

void screenres(int *w, int *h)
{
#if !defined(WIN32) && !defined(__APPLE__)
    if(initing >= INIT_RESET)
    {
#endif
        scr_w = clamp(*w, SCR_MINW, SCR_MAXW);
        scr_h = clamp(*h, SCR_MINH, SCR_MAXH);
#if defined(WIN32) || defined(__APPLE__)
        initwarning("screen resolution");
#else
        return;
    }
    SDL_Surface *surf = SDL_SetVideoMode(clamp(*w, SCR_MINW, SCR_MAXW), clamp(*h, SCR_MINH, SCR_MAXH), 0, SDL_OPENGL|(screen->flags&SDL_FULLSCREEN ? SDL_FULLSCREEN : SDL_RESIZABLE));
    if(!surf) return;
    screen = surf;
    scr_w = screen->w;
    scr_h = screen->h;
    glViewport(0, 0, scr_w, scr_h);
#endif
}

static int curgamma = 100;
VARFP(gamma, 30, 100, 300,
{
    if(gamma == curgamma) return;
    curgamma = gamma;
    float f = gamma/100.0f;
    if(SDL_SetGamma(f,f,f)==-1) conoutf(CON_ERROR, "Could not set gamma: %s", SDL_GetError());
});

void restoregamma()
{
    if(curgamma == 100) return;
    float f = curgamma/100.0f;
    SDL_SetGamma(1, 1, 1);
    SDL_SetGamma(f, f, f);
}

void cleargamma()
{
    if(curgamma != 100) SDL_SetGamma(1, 1, 1);
}

VAR(dbgmodes, 0, 0, 1);

int desktopw = 0, desktoph = 0;

void setupscreen(int &usedcolorbits, int &useddepthbits, int &usedfsaa)
{
    int flags = SDL_RESIZABLE;
    #if defined(WIN32) || defined(__APPLE__)
    flags = 0;
    #endif
    if(fullscreen) flags = SDL_FULLSCREEN;
    SDL_Rect **modes = SDL_ListModes(NULL, SDL_OPENGL|flags);
    if(modes && modes!=(SDL_Rect **)-1)
    {
        int widest = -1, best = -1;
        for(int i = 0; modes[i]; i++)
        {
            if(dbgmodes) conoutf(CON_DEBUG, "mode[%d]: %d x %d", i, modes[i]->w, modes[i]->h);
            if(widest < 0 || modes[i]->w > modes[widest]->w || (modes[i]->w == modes[widest]->w && modes[i]->h > modes[widest]->h)) 
                widest = i; 
        }
        if(scr_w < 0 || scr_h < 0)
        {
            int w = scr_w, h = scr_h, ratiow = desktopw, ratioh = desktoph;
            if(w < 0 && h < 0) { w = SCR_DEFAULTW; h = SCR_DEFAULTH; }
            if(ratiow <= 0 || ratioh <= 0) { ratiow = modes[widest]->w; ratioh = modes[widest]->h; }
            for(int i = 0; modes[i]; i++) if(modes[i]->w*ratioh == modes[i]->h*ratiow)
            {
                if(w <= modes[i]->w && h <= modes[i]->h && (best < 0 || modes[i]->w < modes[best]->w))
                    best = i;
            }
        } 
        if(best < 0)
        {
            int w = scr_w, h = scr_h;
            if(w < 0 && h < 0) { w = SCR_DEFAULTW; h = SCR_DEFAULTH; }
            else if(w < 0) w = (h*SCR_DEFAULTW)/SCR_DEFAULTH;
            else if(h < 0) h = (w*SCR_DEFAULTH)/SCR_DEFAULTW;
            for(int i = 0; modes[i]; i++)
            {
                if(w <= modes[i]->w && h <= modes[i]->h && (best < 0 || modes[i]->w < modes[best]->w || (modes[i]->w == modes[best]->w && modes[i]->h < modes[best]->h)))
                    best = i;
            }
        }
        if(flags&SDL_FULLSCREEN)
        {
            if(best >= 0) { scr_w = modes[best]->w; scr_h = modes[best]->h; }
            else if(desktopw > 0 && desktoph > 0) { scr_w = desktopw; scr_h = desktoph; }
            else if(widest >= 0) { scr_w = modes[widest]->w; scr_h = modes[widest]->h; } 
        }
        else if(best < 0)
        { 
            scr_w = min(scr_w >= 0 ? scr_w : (scr_h >= 0 ? (scr_h*SCR_DEFAULTW)/SCR_DEFAULTH : SCR_DEFAULTW), (int)modes[widest]->w); 
            scr_h = min(scr_h >= 0 ? scr_h : (scr_w >= 0 ? (scr_w*SCR_DEFAULTH)/SCR_DEFAULTW : SCR_DEFAULTH), (int)modes[widest]->h);
        }
        if(dbgmodes) conoutf(CON_DEBUG, "selected %d x %d", scr_w, scr_h);
    }
    if(scr_w < 0 && scr_h < 0) { scr_w = SCR_DEFAULTW; scr_h = SCR_DEFAULTH; }
    else if(scr_w < 0) scr_w = (scr_h*SCR_DEFAULTW)/SCR_DEFAULTH;
    else if(scr_h < 0) scr_h = (scr_w*SCR_DEFAULTH)/SCR_DEFAULTW;

    bool hasbpp = true;
    if(colorbits)
        hasbpp = SDL_VideoModeOK(scr_w, scr_h, colorbits, SDL_OPENGL|flags)==colorbits;

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
#if SDL_VERSION_ATLEAST(1, 2, 11)
    if(vsync>=0) SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, vsync);
#endif
    static int configs[] =
    {
        0x7, /* try everything */
        0x6, 0x5, 0x3, /* try disabling one at a time */
        0x4, 0x2, 0x1, /* try disabling two at a time */
        0 /* try disabling everything */
    };
    int config = 0;
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
    if(!fsaa)
    {
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
    }
    loopi(sizeof(configs)/sizeof(configs[0]))
    {
        config = configs[i];
        if(!depthbits && config&1) continue;
        if(!stencilbits && config&2) continue;
        if(fsaa<=0 && config&4) continue;
        if(depthbits) SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, config&1 ? depthbits : 0);
        if(stencilbits)
        {
            hasstencil = config&2 ? stencilbits : 0;
            SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, hasstencil);
        }
        else hasstencil = 0;
        if(fsaa>0)
        {
            SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, config&4 ? 1 : 0);
            SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, config&4 ? fsaa : 0);
        }
        screen = SDL_SetVideoMode(scr_w, scr_h, hasbpp ? colorbits : 0, SDL_OPENGL|flags);
        if(screen) break;
    }
    if(!screen) fatal("Unable to create OpenGL screen: %s", SDL_GetError());
    else
    {
        if(!hasbpp) conoutf(CON_WARN, "%d bit color buffer not supported - disabling", colorbits);
        if(depthbits && (config&1)==0) conoutf(CON_WARN, "%d bit z-buffer not supported - disabling", depthbits);
        if(stencilbits && (config&2)==0) conoutf(CON_WARN, "Stencil buffer not supported - disabling");
        if(fsaa>0 && (config&4)==0) conoutf(CON_WARN, "%dx anti-aliasing not supported - disabling", fsaa);
    }

    scr_w = screen->w;
    scr_h = screen->h;

    usedcolorbits = hasbpp ? colorbits : 0;
    useddepthbits = config&1 ? depthbits : 0;
    usedfsaa = config&4 ? fsaa : 0;
}

void resetgl()
{
    clearchanges(CHANGE_GFX);

    renderbackground("resetting OpenGL");

    extern void cleanupva();
    extern void cleanupparticles();
    extern void cleanupsky();
    extern void cleanupmodels();
    extern void cleanuptextures();
    extern void cleanupblendmap();
    extern void cleanshadowmap();
    extern void cleanupglare();
    extern void cleanupshaders();
    extern void cleanupgl();
    recorder::cleanup();
    cleanupva();
    cleanupparticles();
    cleanupsky();
    cleanupmodels();
    cleanuptextures();
    cleanupblendmap();
    cleanupshaders();
    cleanupgl();
    
    SDL_SetVideoMode(0, 0, 0, 0);

    int usedcolorbits = 0, useddepthbits = 0, usedfsaa = 0;
    setupscreen(usedcolorbits, useddepthbits, usedfsaa);
    gl_init(scr_w, scr_h, usedcolorbits, useddepthbits, usedfsaa);

    extern void reloadfonts();
    extern void reloadtextures();
    extern void reloadshaders();
    inbetweenframes = false;
    if(!reloadtexture(*notexture) ||
       !reloadtexture("<premul>data/textures/ui/logo.png") ||
       !reloadtexture("<premul>data/textures/ui/logo_1024.png") ||
#if 0
       !reloadtexture("<premul>data/textures/ui/cube2badge.png") ||
#endif
       !reloadtexture("data/textures/ui/background.png") ||
#if 0
       !reloadtexture("<premul>data/textures/ui/background_detail.png") ||
       !reloadtexture("<premul>data/textures/ui/background_decal.png") ||
#endif
       !reloadtexture("data/textures/ui/mapshot_frame.png") ||
       !reloadtexture("data/textures/ui/loading_frame.png") ||
       !reloadtexture("data/textures/ui/loading_bar.png"))
        fatal("failed to reload core texture");
    reloadfonts();
    inbetweenframes = true;
    renderbackground("initializing...");
    restoregamma();
    reloadshaders();
    reloadtextures();
    initlights();
    allchanged(true);
}

vector<SDL_Event> events;

void pushevent(const SDL_Event &e)
{
    events.add(e); 
}

static bool filterevent(const SDL_Event &event)
{
    switch(event.type)
    {
        case SDL_MOUSEMOTION:
            #ifndef WIN32
            if(grabinput && !(screen->flags&SDL_FULLSCREEN))
            {
                if(event.motion.x == screen->w / 2 && event.motion.y == screen->h / 2) 
                    return false;  // ignore any motion events generated by SDL_WarpMouse
                #ifdef __APPLE__
                if(event.motion.y == 0) 
                    return false;  // let mac users drag windows via the title bar
                #endif
            }
            #endif
            break;
    }
    return true;
}

static inline bool pollevent(SDL_Event &event)
{
    while(SDL_PollEvent(&event))
    {
        if(filterevent(event)) return true;
    }
    return false;
}

bool interceptkey(int sym)
{
    static int lastintercept = SDLK_UNKNOWN;
    int len = lastintercept == sym ? events.length() : 0;
    SDL_Event event;
    while(pollevent(event))
    {
        switch(event.type)
        {
            case SDL_MOUSEMOTION: break;
            default: pushevent(event); break;
        }
    }
    lastintercept = sym;
    if(sym != SDLK_UNKNOWN) for(int i = len; i < events.length(); i++)
    {
        if(events[i].type == SDL_KEYDOWN && events[i].key.keysym.sym == sym) { events.remove(i); return true; }
    }
    return false;
}

static void ignoremousemotion()
{
    SDL_Event e;
    SDL_PumpEvents();
    while(SDL_PeepEvents(&e, 1, SDL_GETEVENT, SDL_EVENTMASK(SDL_MOUSEMOTION)));
}

static void resetmousemotion()
{
#ifndef WIN32
    if(grabinput && !(screen->flags&SDL_FULLSCREEN))
    {
        SDL_WarpMouse(screen->w / 2, screen->h / 2);
    }
#endif
}

static void checkmousemotion(int &dx, int &dy)
{
    loopv(events)
    {
        SDL_Event &event = events[i];
        if(event.type != SDL_MOUSEMOTION)
        { 
            if(i > 0) events.remove(0, i); 
            return; 
        }
        dx += event.motion.xrel;
        dy += event.motion.yrel;
    }
    events.setsize(0);
    SDL_Event event;
    while(pollevent(event))
    {
        if(event.type != SDL_MOUSEMOTION)
        {
            events.add(event);
            return;
        }
        dx += event.motion.xrel;
        dy += event.motion.yrel;
    }
}

void checkinput()
{
    SDL_Event event;
    int lasttype = 0, lastbut = 0;
    bool mousemoved = false; 
    while(events.length() || pollevent(event))
    {
        if(events.length()) event = events.remove(0);

        switch(event.type)
        {
            case SDL_QUIT:
                quit();
                return;

            #if !defined(WIN32) && !defined(__APPLE__)
            case SDL_VIDEORESIZE:
                screenres(&event.resize.w, &event.resize.h);
                break;
            #endif

            case SDL_KEYDOWN:
            case SDL_KEYUP:
                #if 0 // INTENSITY start XXX security issue
                printf("SDL_KEY: %d, %d, %d\r\n", event.key.keysym.sym, event.key.state==SDL_PRESSED, event.key.keysym.unicode);
                #endif // INTENSITY end

                keypress(event.key.keysym.sym, event.key.state==SDL_PRESSED, uni2cube(event.key.keysym.unicode));
                break;

            case SDL_ACTIVEEVENT:
                if(event.active.state & SDL_APPINPUTFOCUS)
                    inputgrab(grabinput = event.active.gain!=0);
                if(event.active.state & SDL_APPACTIVE)
                    minimized = !event.active.gain;
                break;

            case SDL_MOUSEMOTION:
                if(grabinput)
                {
                    int dx = event.motion.xrel, dy = event.motion.yrel;
                    checkmousemotion(dx, dy);
                    if(!gui::movecursor(dx, dy) && !gui::hascursor()) mousemove(dx, dy);
                    mousemoved = true;
                }
                break;

            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
                #if 0 // INTENSITY start XXX security issue
                float x, y;
                g3d_cursorpos(x, y);
                printf("SDL_MOUSEBUTTON: %d, %d (at %f,%f)\r\n", event.button.button, event.button.state, x, y);
                #endif // INTENSITY end

                if(lasttype==event.type && lastbut==event.button.button) break; // why?? get event twice without it
                keypress(-event.button.button, event.button.state!=0, 0);
                lasttype = event.type;
                lastbut = event.button.button;
                break;
        }
    }
    if(mousemoved) resetmousemotion();
}

void swapbuffers()
{
    recorder::capture();
    SDL_GL_SwapBuffers();
}

VARF(gamespeed, 10, 100, 1000, if(multiplayer()) gamespeed = 100);

VARF(paused, 0, 0, 1, if(multiplayer()) paused = 0);

VAR(menufps, 0, 60, 1000);
VARP(maxfps, 0, 200, 1000);

void limitfps(int &millis, int curmillis)
{
    int limit = gui::mainmenu && menufps ? (maxfps ? min(maxfps, menufps) : menufps) : maxfps;
    if(!limit) return;
    static int fpserror = 0;
    int delay = 1000/limit - (millis-curmillis);
    if(delay < 0) fpserror = 0;
    else
    {
        fpserror += 1000%limit;
        if(fpserror >= limit)
        {
            ++delay;
            fpserror -= limit;
        }
        if(delay > 0)
        {
            SDL_Delay(delay);
            millis += delay;
        }
    }
}

#if defined(WIN32) && !defined(_DEBUG) && !defined(__GNUC__)
void stackdumper(unsigned int type, EXCEPTION_POINTERS *ep)
{
    if(!ep) fatal("unknown type");
    EXCEPTION_RECORD *er = ep->ExceptionRecord;
    CONTEXT *context = ep->ContextRecord;
    string out, t;
    formatstring(out)("Tesseract Win32 Exception: 0x%x [0x%x]\n\n", er->ExceptionCode, er->ExceptionCode==EXCEPTION_ACCESS_VIOLATION ? er->ExceptionInformation[1] : -1);
    STACKFRAME sf = {{context->Eip, 0, AddrModeFlat}, {}, {context->Ebp, 0, AddrModeFlat}, {context->Esp, 0, AddrModeFlat}, 0};
    SymInitialize(GetCurrentProcess(), NULL, TRUE);

    while(::StackWalk(IMAGE_FILE_MACHINE_I386, GetCurrentProcess(), GetCurrentThread(), &sf, context, NULL, ::SymFunctionTableAccess, ::SymGetModuleBase, NULL))
    {
        struct { IMAGEHLP_SYMBOL sym; string n; } si = { { sizeof( IMAGEHLP_SYMBOL ), 0, 0, 0, sizeof(string) } };
        IMAGEHLP_LINE li = { sizeof( IMAGEHLP_LINE ) };
        DWORD off;
        if(SymGetSymFromAddr(GetCurrentProcess(), (DWORD)sf.AddrPC.Offset, &off, &si.sym) && SymGetLineFromAddr(GetCurrentProcess(), (DWORD)sf.AddrPC.Offset, &off, &li))
        {
            char *del = strrchr(li.FileName, '\\');
            formatstring(t)("%s - %s [%d]\n", si.sym.Name, del ? del + 1 : li.FileName, li.LineNumber);
            concatstring(out, t);
        }
    }
    fatal(out);
}
#endif

#define MAXFPSHISTORY 60

int fpspos = 0, fpshistory[MAXFPSHISTORY];

void resetfpshistory()
{
    loopi(MAXFPSHISTORY) fpshistory[i] = 1;
    fpspos = 0;
}

void updatefpshistory(int millis)
{
    fpshistory[fpspos++] = max(1, min(1000, millis));
    if(fpspos>=MAXFPSHISTORY) fpspos = 0;
}

void getframemillis(float &avg, float &bestdiff, float &worstdiff)
{
    int total = fpshistory[MAXFPSHISTORY-1], best = total, worst = total;
    loopi(MAXFPSHISTORY-1)
    {
        int millis = fpshistory[i];
        total += millis;
        if(millis < best) best = millis;
        if(millis > worst) worst = millis;
    }

    avg = total/float(MAXFPSHISTORY);
    best = best - avg;
    worstdiff = avg - worst;    
}

void getfps(int &fps, int &bestdiff, int &worstdiff)
{
    int total = fpshistory[MAXFPSHISTORY-1], best = total, worst = total;
    loopi(MAXFPSHISTORY-1)
    {
        int millis = fpshistory[i];
        total += millis;
        if(millis < best) best = millis;
        if(millis > worst) worst = millis;
    }

    fps = (1000*MAXFPSHISTORY)/total;
    bestdiff = 1000/best-fps;
    worstdiff = fps-1000/worst;
}

types::Tuple<int, int, int> getfps_(bool raw)
{
    int fps, bestdiff = 0, worstdiff = 0;
    if(raw) fps = 1000/fpshistory[(fpspos+MAXFPSHISTORY-1)%MAXFPSHISTORY];
    else getfps(fps, bestdiff, worstdiff);
    return types::make_tuple(fps, bestdiff, worstdiff);
}

bool inbetweenframes = false, renderedframe = true;

static bool findarg(int argc, char **argv, const char *str)
{
    for(int i = 1; i<argc; i++) if(strstr(argv[i], str)==argv[i]) return true;
    return false;
}

int clockrealbase = 0, clockvirtbase = 0; // INTENSITY: Removed 'static'
void clockreset() { clockrealbase = SDL_GetTicks(); clockvirtbase = totalmillis; }
VARFP(clockerror, 990000, 1000000, 1010000, clockreset());
VARFP(clockfix, 0, 0, 1, clockreset());

int getclockmillis()
{
    int millis = SDL_GetTicks() - clockrealbase;
    if(clockfix) millis = int(millis*(double(clockerror)/1000000));
    millis += clockvirtbase;
    return max(millis, totalmillis);
}

VAR(frametimer, 0, 0, 1);
int framemillis = 0; // frame time (ie does not take into account the swap)

int main(int argc, char **argv)
{
    #ifdef WIN32
    //atexit((void (__cdecl *)(void))_CrtDumpMemoryLeaks);
    #ifndef _DEBUG
    #ifndef __GNUC__
    //__try { Currently broken thanks to syntensity stuff
    #endif
    #endif
    #endif

    setvbuf(stdout, NULL, _IOLBF, BUFSIZ);
    setlogfile(NULL);

    int dedicated = 0;
    char *load = NULL, *initscript = NULL;

    #define initlog(s) logger::log(logger::INIT, "%s\n", s)

    initing = INIT_RESET;

    char *loglevel = (char*)"WARNING";
    for(int i = 1; i<argc; i++)
    {
        if(argv[i][0]=='-') switch(argv[i][1])
        {
            case 'q': 
            {
                const char *dir = sethomedir(&argv[i][2]);
                if(dir) logoutf("Using home directory: %s", dir);
                break;
            }
        }
    }
    for(int i = 1; i<argc; i++)
    {
        if(argv[i][0]=='-') switch(argv[i][1])
        {
            case 'q': /* parsed first */ break;
            case 'r': /* compat, ignore */ break;
            case 'k':
            {
                const char *dir = addpackagedir(&argv[i][2]);
                if(dir) logoutf("Adding package directory: %s", dir);
                break;
            }
            case 'g': logoutf("Setting logging level", &argv[i][2]); loglevel = &argv[i][2]; break;
            case 'd': dedicated = atoi(&argv[i][2]); if(dedicated<=0) dedicated = 2; break;
            case 'w': scr_w = clamp(atoi(&argv[i][2]), SCR_MINW, SCR_MAXW); if(!findarg(argc, argv, "-h")) scr_h = -1; break;
            case 'h': scr_h = clamp(atoi(&argv[i][2]), SCR_MINH, SCR_MAXH); if(!findarg(argc, argv, "-w")) scr_w = -1; break;
            case 'z': depthbits = atoi(&argv[i][2]); break;
            case 'b': colorbits = atoi(&argv[i][2]); break;
            case 'a': fsaa = atoi(&argv[i][2]); break;
            case 'v': vsync = atoi(&argv[i][2]); break;
            case 't': fullscreen = atoi(&argv[i][2]); break;
            case 's': stencilbits = atoi(&argv[i][2]); break;
            case 'f': 
            {
                extern int shaderprecision;
                int n = atoi(&argv[i][2]);
                shaderprecision = clamp(n - 1, 0, 2);
                break;
            }
            case 'l': 
            {
                char pkgdir[] = "data/"; 
                load = strstr(path(&argv[i][2]), path(pkgdir)); 
                if(load) load += sizeof(pkgdir)-1; 
                else load = &argv[i][2]; 
                break;
            }
            case 'x': initscript = &argv[i][2]; break;
            default: if(!serveroption(argv[i])) gameargs.add(argv[i]); break;
        }
        else gameargs.add(argv[i]);
    }
    /* Initialize logging at first, right after that lua. */
    logger::setlevel(loglevel);

    initlog("lua");
    lapi::init();
    if (!lapi::state.state()) fatal("cannot initialize lua script engine");
    tools::execcfg("init.lua", true);

    initing = NOT_INITING;

    if(dedicated <= 1)
    {
        initlog("sdl");

        int par = 0;
        #ifdef _DEBUG
        par = SDL_INIT_NOPARACHUTE;
        #ifdef WIN32
        SetEnvironmentVariable("SDL_DEBUG", "1");
        #endif
        #endif

        if(SDL_Init(SDL_INIT_TIMER|SDL_INIT_VIDEO|SDL_INIT_AUDIO|par)<0) fatal("Unable to initialize SDL: %s", SDL_GetError());
    }

    initlog("net");
    if(enet_initialize()<0) fatal("Unable to initialise network module");
    atexit(enet_deinitialize);
    enet_time_set(0);

    initlog("game");
    game::parseoptions(gameargs);
    initserver(dedicated>0, dedicated>1);  // never returns if dedicated
    ASSERT(dedicated <= 1);
    game::initclient();

    initlog("video: mode");
    const SDL_VideoInfo *video = SDL_GetVideoInfo();
    if(video) 
    {
        desktopw = video->current_w;
        desktoph = video->current_h;
    }
    int usedcolorbits = 0, useddepthbits = 0, usedfsaa = 0;
    setupscreen(usedcolorbits, useddepthbits, usedfsaa);

    initlog("video: misc");
    SDL_WM_SetCaption("OctaForge", NULL); // INTENSITY
    keyrepeat(false);
    SDL_ShowCursor(0);

    initlog("gl");
    gl_checkextensions();
    gl_init(scr_w, scr_h, usedcolorbits, useddepthbits, usedfsaa);
    notexture = textureload("data/textures/core/notexture.png");
    if(!notexture) fatal("could not find core textures");
    gui::setup();

    initlog("console");
    varsys::persistvars = false;

    types::Tuple<int, const char*> err;

    err = lapi::state.do_file("data/cfg/font.lua");

    if(types::get<0>(err))
        fatal("cannot find font definitions: %s", types::get<1>(err));
    if(!setfont("default")) fatal("no default font specified");

    inbetweenframes = true;
    renderbackground("initializing...");

    initlog("gl: effects");
    loadshaders();
    particleinit();
    initdecals();

    initlog("world");
    camera1 = player = game::iterdynents(0);
    //emptymap(0, true, NULL, false);

    initlog("sound");
    initsound();

    initlog("cfg");

    lapi::state.do_file("data/cfg/keymap.lua", lua::ERROR_EXIT_TRACEBACK);
    lapi::state.do_file("data/cfg/sounds.lua", lua::ERROR_EXIT_TRACEBACK);
    lapi::state.do_file("data/cfg/menus.lua",  lua::ERROR_EXIT_TRACEBACK);
    lapi::state.do_file("data/cfg/brush.lua",  lua::ERROR_EXIT_TRACEBACK);
    err = lapi::state.do_file("mybrushes.lua", lua::ERROR_TRACEBACK);
    if (types::get<0>(err))
        logger::log(logger::ERROR, "%s\n", types::get<1>(err));

    if (game::savedservers())
    {
        err = lapi::state.do_file(
            game::savedservers(), lua::ERROR_TRACEBACK
        );
        if (types::get<0>(err))
            logger::log(logger::ERROR, "%s\n", types::get<1>(err));
    }
    
    varsys::persistvars = true;
    
    initing = INIT_LOAD;
    if(!tools::execcfg(game::savedconfig())) 
    {
        lapi::state.do_file(game::defaultconfig(), lua::ERROR_EXIT_TRACEBACK);
        tools::writecfg(game::restoreconfig());
    }
    lapi::state.do_file("data/cfg/config.lua", lua::ERROR_EXIT_TRACEBACK);
    err = lapi::state.do_file(game::autoexec(), lua::ERROR_TRACEBACK);
    if (types::get<0>(err))
        logger::log(logger::ERROR, "%s\n", types::get<1>(err));
    initing = NOT_INITING;

    varsys::persistvars = false;

    game::loadconfigs();

    varsys::persistvars = true;

    initlog("Registering messages\n");
    MessageSystem::MessageManager::registerAll();

    if(load)
    {
        initlog("localconnect");
        //localconnect();
        game::changemap(load);
    }

    if (initscript)
    {
        err = lapi::state.do_file(initscript, lua::ERROR_TRACEBACK);
        if (types::get<0>(err))
            logger::log(logger::ERROR, "%s\n", types::get<1>(err));
    }

    initlog("mainloop");

    initmumble();
    resetfpshistory();

    inputgrab(grabinput = true);
    ignoremousemotion();

    for(;;)
    {
        static int frames = 0;
        int millis = getclockmillis();
        limitfps(millis, totalmillis);
        int elapsed = millis-totalmillis;
        if(multiplayer(false)) curtime = game::ispaused() ? 0 : elapsed;
        else
        {
            static int timeerr = 0;
            int scaledtime = elapsed*gamespeed + timeerr;
            curtime = scaledtime/100;
            timeerr = scaledtime%100;
            if(curtime>200) curtime = 200;
            if(paused || game::ispaused()) curtime = 0;
        }
        local_server::try_connect(); /* Try connecting if server is ready */

        lastmillis += curtime;
        totalmillis = millis;

        logger::log(logger::INFO, "New frame: lastmillis: %d   curtime: %d\r\n", lastmillis, curtime); // INTENSITY

        extern void updatetime();
        updatetime();

        checkinput();
        gui::update();
        tryedit();

        if(lastmillis) game::updateworld();

        serverslice(false, 0);

        if(frames) updatefpshistory(elapsed);
        frames++;

        // miscellaneous general game effects
        recomputecamera();
        updateparticles();
        updatesounds();

        if(minimized) continue;

        if(!gui::mainmenu && ClientSystem::scenarioStarted()) gl_setupframe(screen->w, screen->h);

        inbetweenframes = false;

        if(gui::mainmenu) gl_drawmainmenu(screen->w, screen->h);
        else
        {
            // INTENSITY: If we have all the data we need from the server to run the game, then we can actually draw
            if (ClientSystem::scenarioStarted()) gl_drawframe(screen->w, screen->h);
        }
        if(frametimer)
        {
            glFinish();
            framemillis = getclockmillis() - millis;
        }
        swapbuffers();
        renderedframe = inbetweenframes = true;

        ClientSystem::frameTrigger(curtime); // INTENSITY
    }
    
    ASSERT(0);   
    return EXIT_FAILURE;

    #if defined(WIN32) && !defined(_DEBUG) && !defined(__GNUC__)
    //} __except(stackdumper(0, GetExceptionInformation()), EXCEPTION_CONTINUE_SEARCH) { return 0; } Currently broken thanks to syntensity stuff
    #endif
}
