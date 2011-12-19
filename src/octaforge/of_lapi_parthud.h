VARP(blood, 0, 1, 1);

namespace lapi_binds
{
    void _lua_adddecal(
        int type, vec center, vec surface, float radius, bvec color, int info
    )
    {
        adddecal(type, center, surface, radius, color, info);
    }

    void _lua_particle_splash(
        int type, int num, int fade, vec p, int color, float size, int radius,
        int gravity, bool regfade, int flag, bool fastsplash, int grow
    )
    {
        if (type == PART_BLOOD && !blood) return;
        particle_splash(
            type, num, fade, p, color, size, radius,
            gravity, regfade, flag, fastsplash, grow
        );
    }

    void _lua_regular_particle_splash(
        int type, int num, int fade, vec p, int color, float size,
        int radius, int gravity, int delay, bool hover, int grow
    )
    {
        if (type == PART_BLOOD && !blood) return;
        regular_particle_splash(
            type, num, fade, p, color, size,
            radius, gravity, delay, hover, grow
        );
    }

    void _lua_particle_fireball(
        vec dest, float max, int type, int fade, int color, float size
    )
    {
        particle_fireball(dest, max, type, fade, color, size);
    }

    void _lua_particle_explodesplash(
        vec o, int fade, int type, int color, int size, int gravity, int num
    )
    {
        particle_explodesplash(o, fade, type, color, size, gravity, num);
    }

    void _lua_particle_flare(
        vec p, vec dest, int fade, int type,
        int color, float size, int grow, int uid
    )
    {
        if (uid < 0)
            particle_flare(p, dest, fade, type, color, size, NULL, grow);
        else
        {
            CLogicEntity *o = LogicSystem::getLogicEntity(uid);
            assert(o->dynamicEntity);

            particle_flare(
                p, dest, fade, type, color, size,
                (fpsent*)(o->dynamicEntity), grow
            );
        }
    }

    void _lua_particle_flying_flare(
        vec o, vec d, int fade, int type, int color, float size, int gravity
    )
    {
        particle_flying_flare(o, d, fade, type, color, size, gravity);
    }

    void _lua_particle_trail(
        int type, int fade, vec from, vec to,
        int color, float size, int gravity, bool bubbles
    )
    {
        particle_trail(type, fade, from, to, color, size, gravity, bubbles);
    }

    void _lua_particle_flame(
        int type, vec p, float radius, float height, int color,
        int density, float scale, float speed, float fade, int gravity
    )
    {
        regular_particle_flame(
            type, p, radius, height, color,
            density, scale, speed, fade, gravity
        );
    }

    void _lua_adddynlight(
        vec o, float rad, vec color, int fade,
        int peak, int flags, float irad, vec icolor
    )
    {
        queuedynlight(o, rad, color, fade, peak, flags, irad, icolor, NULL);
    }

    void _lua_particle_meter(vec s, float val, int type, int fade)
    {
        particle_meter(s, val, type, fade);
    }

    void _lua_particle_text(
        vec s, const char *t, int type, int fade,
        int color, float size, float gravity
    )
    {
        particle_textcopy(s, t, type, fade, color, size, gravity);
    }

    void _lua_client_damage_effect(int roll, int n)
    {
        ((fpsent*)player)->damageroll(roll);
        damageblend(n);
    }

    void _lua_showhudrect(
        float x1, float y1, float x2, float y2, int color, float alpha
    )
    {
        if (!color) color = 0xFFFFFF;
        if (!alpha) alpha = 1.0f;
        ClientSystem::addHUDRect(x1, y1, x2, y2, color, alpha);
    }

    void _lua_showhudimage(
        const char *tex, float cx, float cy,
        float wiu, float hiu, int color, float alpha
    )
    {
        if (!color) color = 0xFFFFFF;
        if (!alpha) alpha = 1.0f;
        ClientSystem::addHUDImage(tex, cx, cy, wiu, hiu, color, alpha);
    }

    void _lua_showhudtext(
        const char *tx, float x, float y, float scale, int color
    )
    {
        ClientSystem::addHUDText(tx, x, y, scale, color);
    }

    void reg_parthud(lua::Table& t)
    {
        LAPI_REG(adddecal);
        LAPI_REG(particle_splash);
        LAPI_REG(regular_particle_splash);
        LAPI_REG(particle_fireball);
        LAPI_REG(particle_explodesplash);
        LAPI_REG(particle_flare);
        LAPI_REG(particle_flying_flare);
        LAPI_REG(particle_trail);
        LAPI_REG(particle_flame);
        LAPI_REG(adddynlight);
        LAPI_REG(particle_meter);
        LAPI_REG(particle_text);
        LAPI_REG(client_damage_effect);
        LAPI_REG(showhudrect);
        LAPI_REG(showhudimage);
        LAPI_REG(showhudtext);
    }
}
