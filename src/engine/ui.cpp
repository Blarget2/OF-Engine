#include "engine.h"

namespace gui
{
    VAR(uitogglehside, 1, 0, 0);
    VAR(uitogglevside, 1, 0, 0);
    // default size of text in terms of rows per screenful
    VARP(uitextrows, 1, 40, 200);
    FVAR(cursorsensitivity, 1e-3f, 1, 1000);
    VAR(mainmenu, 0, 1, 1);

    void resetcursor()
    {
        lapi::state.get<lua::Function>("gui", "core", "resetcursor")();
    }

    bool movecursor(int dx, int dy)
    {
        return lapi::state.get<lua::Function>("gui", "core", "movecursor").call<bool>(dx, dy);
    }

    bool hascursor(bool targeting)
    {
        return lapi::state.get<lua::Function>("gui", "core", "hascursor").call<bool>(targeting);
    }

    void getcursorpos(float &x, float &y)
    {
        lua_getglobal(lapi::state.state(), "getcursorpos");
        lua_call(lapi::state.state(), 0, 2);
        x = lua_tonumber(lapi::state.state(), -2);
        y = lua_tonumber(lapi::state.state(), -1);
        lua_pop(lapi::state.state(), 2);
    }

    bool keypress(int code, bool isdown, int cooked)
    {
        return lapi::state.get<lua::Function>("gui", "core", "keypress").call<bool>(code, isdown, cooked);
    }

    void clearmainmenu()
    {
        lapi::state.get<lua::Function>("gui", "core", "clearmainmenu")();
    }

    void setup()
    {
        lapi::state.get<lua::Function>("gui", "core", "setup")();
    }

    void update()
    {
        lapi::state.get<lua::Function>("gui", "core", "update")();
    }

    void render()
    {
        lapi::state.get<lua::Function>("gui", "core", "render")();
    }
}

VARP(applydialog, 0, 1, 1);

void addchange(const char *desc, int type) {
    lapi::state.get<lua::Function>("gui", "core", "change_new")(desc, type);
}

void clearchanges(int type) {
    lapi::state.get<lua::Function>("gui", "core", "changes_clear")(type);
}

VAR(fonth, 512, 0, 0);
VAR(fontw, 512, 0, 0);
HVARP(fullconcolor, 0, 0x4F4F4F, 0xFFFFFF);
FVARP(fullconblend, 0, .8, 1);

void consolebox(int x1, int y1, int x2, int y2)
{
    glPushMatrix();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_TEXTURE_2D);
    notextureshader->set();

    glTranslatef(x1, y1, 0);
    float r = ((fullconcolor >> 16) & 0xFF) / 255.f,
        g = ((fullconcolor >> 8) & 0xFF) / 255.f,
        b = (fullconcolor & 0xFF) / 255.f;
    glColor4f(r, g, b, fullconblend);
    glBegin(GL_TRIANGLE_STRIP);

    glVertex2i(x1, y1);
    glVertex2i(x2, y1);
    glVertex2i(x1, y2);
    glVertex2i(x2, y2);

    glEnd();
    glEnable(GL_TEXTURE_2D);
    defaultshader->set();

    glPopMatrix();
}
