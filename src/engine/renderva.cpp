// renderva.cpp: handles the occlusion and rendering of vertex arrays

#include "engine.h"
#include "of_entities.h"

static inline void drawtris(GLsizei numindices, const GLvoid *indices, ushort minvert, ushort maxvert)
{
    if(hasDRE) glDrawRangeElements_(GL_TRIANGLES, minvert, maxvert, numindices, GL_UNSIGNED_SHORT, indices);
    else glDrawElements(GL_TRIANGLES, numindices, GL_UNSIGNED_SHORT, indices);
    glde++;
}

static inline void drawvatris(vtxarray *va, GLsizei numindices, const GLvoid *indices)
{
    drawtris(numindices, indices, va->minvert, va->maxvert);
}

///////// view frustrum culling ///////////////////////

extern vector<vtxarray *> varoot, valist;

plane vfcP[5];  // perpindictular vectors to view frustrum bounding planes
float vfcDfog;  // far plane culling distance (fog limit).
float vfcDnear[5], vfcDfar[5];

vtxarray *visibleva = NULL;

bool isfoggedsphere(float rad, const vec &cv)
{
    loopi(4) if(vfcP[i].dist(cv) < -rad) return true;
    float dist = vfcP[4].dist(cv);
    return dist < -rad || dist > vfcDfog + rad;
}

int isvisiblesphere(float rad, const vec &cv)
{
    int v = VFC_FULL_VISIBLE;
    float dist;

    loopi(5)
    {
        dist = vfcP[i].dist(cv);
        if(dist < -rad) return VFC_NOT_VISIBLE;
        if(dist < rad) v = VFC_PART_VISIBLE;
    }

    dist -= vfcDfog;
    if(dist > rad) return VFC_FOGGED;  //VFC_NOT_VISIBLE;    // culling when fog is closer than size of world results in HOM
    if(dist > -rad) v = VFC_PART_VISIBLE;

    return v;
}

static inline int ishiddencube(const ivec &o, int size)
{
    loopi(5) if(o.dist(vfcP[i]) < -vfcDfar[i]*size) return true;
    return false;
}

static inline int isfoggedcube(const ivec &o, int size)
{
    loopi(4) if(o.dist(vfcP[i]) < -vfcDfar[i]*size) return true;
    float dist = o.dist(vfcP[4]);
    return dist < -vfcDfar[4]*size || dist > vfcDfog - vfcDnear[4]*size;
}

int isvisiblecube(const ivec &o, int size)
{
    int v = VFC_FULL_VISIBLE;
    float dist;

    loopi(5)
    {
        dist = o.dist(vfcP[i]);
        if(dist < -vfcDfar[i]*size) return VFC_NOT_VISIBLE;
        if(dist < -vfcDnear[i]*size) v = VFC_PART_VISIBLE;
    }

    dist -= vfcDfog;
    if(dist > -vfcDnear[4]*size) return VFC_FOGGED;
    if(dist > -vfcDfar[4]*size) v = VFC_PART_VISIBLE;

    return v;
}

static inline float vadist(vtxarray *va, const vec &p)
{
    return p.dist_to_bb(va->bbmin, va->bbmax);
}

#define VASORTSIZE 64

static vtxarray *vasort[VASORTSIZE];

void addvisibleva(vtxarray *va)
{
    float dist = vadist(va, camera1->o);
    va->distance = int(dist); /*cv.dist(camera1->o) - va->size*SQRT3/2*/

    int hash = clamp(int(dist*VASORTSIZE/worldsize), 0, VASORTSIZE-1);
    vtxarray **prev = &vasort[hash], *cur = vasort[hash];

    while(cur && va->distance >= cur->distance)
    {
        prev = &cur->next;
        cur = cur->next;
    }

    va->next = cur;
    *prev = va;
}

void sortvisiblevas()
{
    visibleva = NULL; 
    vtxarray **last = &visibleva;
    loopi(VASORTSIZE) if(vasort[i])
    {
        vtxarray *va = vasort[i];
        *last = va;
        while(va->next) va = va->next;
        last = &va->next;
    }
}

void findvisiblevas(vector<vtxarray *> &vas, bool resetocclude = false)
{
    loopv(vas)
    {
        vtxarray &v = *vas[i];
        int prevvfc = resetocclude ? VFC_NOT_VISIBLE : v.curvfc;
        v.curvfc = isvisiblecube(v.o, v.size);
        if(v.curvfc!=VFC_NOT_VISIBLE) 
        {
            if(pvsoccluded(v.o, v.size))
            {
                v.curvfc += PVS_FULL_VISIBLE - VFC_FULL_VISIBLE;
                continue;
            }
            addvisibleva(&v);
            if(v.children.length()) findvisiblevas(v.children, prevvfc>=VFC_NOT_VISIBLE);
            if(prevvfc>=VFC_NOT_VISIBLE)
            {
                v.occluded = !v.texs ? OCCLUDE_GEOM : OCCLUDE_NOTHING;
                v.query = NULL;
            }
        }
    }
}

void findvisiblevas()
{
    memset(vasort, 0, sizeof(vasort));
    findvisiblevas(varoot);
    sortvisiblevas();
}

void calcvfcD()
{
    loopi(5)
    {
        plane &p = vfcP[i];
        vfcDnear[i] = vfcDfar[i] = 0;
        loopk(3) if(p[k] > 0) vfcDfar[i] += p[k];
        else vfcDnear[i] += p[k];
    }
} 

void setvfcP(const vec &bbmin, const vec &bbmax)
{
    vec4 px = mvpmatrix.getrow(0), py = mvpmatrix.getrow(1), pz = mvpmatrix.getrow(2), pw = mvpmatrix.getrow(3);
    vfcP[0] = plane(vec4(pw).mul(-bbmin.x).add(px)).normalize(); // left plane
    vfcP[1] = plane(vec4(pw).mul(bbmax.x).sub(px)).normalize(); // right plane
    vfcP[2] = plane(vec4(pw).mul(-bbmin.y).add(py)).normalize(); // bottom plane
    vfcP[3] = plane(vec4(pw).mul(bbmax.y).sub(py)).normalize(); // top plane
    vfcP[4] = plane(vec4(pw).add(pz)).normalize(); // near/far planes

    vfcDfog = fog;
    calcvfcD();
}

plane oldvfcP[5];

void savevfcP()
{
    memcpy(oldvfcP, vfcP, sizeof(vfcP));
}

void restorevfcP()
{
    memcpy(vfcP, oldvfcP, sizeof(vfcP));
    calcvfcD();
}

void visiblecubes(bool cull)
{
    if(cull)
    {
        setvfcP();
        findvisiblevas();
    }
    else
    {
        memset(vfcP, 0, sizeof(vfcP));
        vfcDfog = 1000000;
        memset(vfcDnear, 0, sizeof(vfcDnear));
        memset(vfcDfar, 0, sizeof(vfcDfar));
        visibleva = NULL;
        loopv(valist)
        {
            vtxarray *va = valist[i];
            va->distance = 0;
            va->curvfc = VFC_FULL_VISIBLE;
            va->occluded = !va->texs ? OCCLUDE_GEOM : OCCLUDE_NOTHING;
            va->query = NULL;
            va->next = visibleva;
            visibleva = va;
        }
    }
}

static inline bool insideva(const vtxarray *va, const vec &v, int margin = 2)
{
    int size = va->size + margin;
    return v.x>=va->o.x-margin && v.y>=va->o.y-margin && v.z>=va->o.z-margin && 
           v.x<=va->o.x+size && v.y<=va->o.y+size && v.z<=va->o.z+size;
}

///////// occlusion queries /////////////

#define MAXQUERY 2048

struct queryframe
{
    int cur, max, defer;
    occludequery queries[MAXQUERY];
};

static queryframe queryframes[2] = {{0, 0}, {0, 0}};
static uint flipquery = 0;
int deferquery = 0;

int getnumqueries()
{
    return queryframes[flipquery].cur;
}

void flipqueries()
{
    flipquery = (flipquery + 1) % 2;
    queryframe &qf = queryframes[flipquery];
    loopi(qf.cur) qf.queries[i].owner = NULL;
    for(; qf.defer > 0 && qf.max < MAXQUERY; qf.defer--)
    {
        qf.queries[qf.max].owner = NULL;
        qf.queries[qf.max].fragments = -1;
        glGenQueries_(1, &qf.queries[qf.max++].id);
    }
    qf.cur = qf.defer = 0;
}

occludequery *newquery(void *owner)
{
    queryframe &qf = queryframes[flipquery];
    if(qf.cur >= qf.max)
    {
        if(qf.max >= MAXQUERY) return NULL;
        if(deferquery)
        {
            if(qf.max + qf.defer < MAXQUERY) qf.defer++;
            return NULL;
        }
        glGenQueries_(1, &qf.queries[qf.max++].id);
    }
    occludequery *query = &qf.queries[qf.cur++];
    query->owner = owner;
    query->fragments = -1;
    return query;
}

void resetqueries()
{
    loopi(2) loopj(queryframes[i].max) queryframes[i].queries[j].owner = NULL;
}

void clearqueries()
{
    loopi(2)
    {
        queryframe &qf = queryframes[i];
        loopj(qf.max) 
        {
            glDeleteQueries_(1, &qf.queries[j].id);
            qf.queries[j].owner = NULL;
        }
        qf.cur = qf.max = qf.defer = 0;
    }
}

VAR(oqfrags, 0, 8, 64);
VAR(oqwait, 0, 1, 1);

bool checkquery(occludequery *query, bool nowait)
{
    GLuint fragments;
    if(query->fragments >= 0) fragments = query->fragments;
    else
    {
        if(nowait || !oqwait)
        {
            GLint avail;
            glGetQueryObjectiv_(query->id, GL_QUERY_RESULT_AVAILABLE, &avail);
            if(!avail) return false;
        }
        glGetQueryObjectuiv_(query->id, GL_QUERY_RESULT_ARB, &fragments);
        query->fragments = fragments;
    }
    return fragments < uint(oqfrags);
}

void drawbb(const ivec &bo, const ivec &br, const vec &camera)
{
    glBegin(GL_QUADS);

    #define GENFACEORIENT(orient, v0, v1, v2, v3) do { \
        int dim = dimension(orient); \
        if(dimcoord(orient)) \
        { \
            if(camera[dim] < bo[dim] + br[dim]) continue; \
        } \
        else if(camera[dim] > bo[dim]) continue; \
        v0 v1 v2 v3 \
        xtraverts += 4; \
    } while(0); 
    #define GENFACEVERT(orient, vert, ox,oy,oz, rx,ry,rz) \
        glVertex3f(ox rx, oy ry, oz rz);
    GENFACEVERTS(bo.x, bo.x + br.x, bo.y, bo.y + br.y, bo.z, bo.z + br.z, , , , , , )
    #undef GENFACEORIENT
    #undef GENFACEVERTS

    glEnd();
}

extern int octaentsize;

static octaentities *visiblemms, **lastvisiblemms;

static inline bool insideoe(const octaentities *oe, const vec &v, int margin = 1)
{
    return v.x>=oe->bbmin.x-margin && v.y>=oe->bbmin.y-margin && v.z>=oe->bbmin.z-margin &&
           v.x<=oe->bbmax.x+margin && v.y<=oe->bbmax.y+margin && v.z<=oe->bbmax.z+margin;
}

void findvisiblemms(vector< types::Shared_Ptr<extentity> > &ents)
{
    visiblemms = NULL;
    lastvisiblemms = &visiblemms;
    for(vtxarray *va = visibleva; va; va = va->next)
    {
        if(va->mapmodels.empty() || va->curvfc >= VFC_FOGGED || va->occluded >= OCCLUDE_BB) continue;
        loopv(va->mapmodels)
        {
            octaentities *oe = va->mapmodels[i];
            if(isfoggedcube(oe->o, oe->size) || pvsoccluded(oe->bbmin, ivec(oe->bbmax).sub(oe->bbmin))) continue;

            bool occluded = oe->query && oe->query->owner == oe && checkquery(oe->query);
            if(occluded)
            {
                oe->distance = -1;

                oe->next = NULL;
                *lastvisiblemms = oe;
                lastvisiblemms = &oe->next;
            }
            else
            {
                int visible = 0;
                loopv(oe->mapmodels)
                {
                    extentity &e = *(ents[oe->mapmodels[i]].get());
                    if(e.flags&extentity::F_NOVIS) continue;
                    e.visible = true;
                    ++visible;
                }
                if(!visible) continue;

                oe->distance = int(camera1->o.dist_to_bb(oe->o, oe->size));

                octaentities **prev = &visiblemms, *cur = visiblemms;
                while(cur && cur->distance >= 0 && oe->distance > cur->distance)
                {
                    prev = &cur->next;
                    cur = cur->next;
                }

                if(*prev == NULL) lastvisiblemms = &oe->next;
                oe->next = *prev;
                *prev = oe;
            }
        }
    }
}

VAR(oqmm, 0, 4, 8);

extern bool getentboundingbox(extentity &e, ivec &o, ivec &r);

static inline void rendermapmodel(extentity &e)
{
    CLogicEntity *entity = LogicSystem::getLogicEntity(e);
    if (!entity || entity->isNone())
    {
        logger::log(logger::ERROR, "Trying to show a missing mapmodel\r\n");
        logger::log(logger::ERROR, "                                  %d\r\n", LogicSystem::getUniqueId(&e));
        assert(0);
    }
    int anim     = entity->getAnimation(); // ANIM_MAPMODEL|ANIM_LOOP
    int basetime = entity->getStartTime();

    rendermapmodel(entity, anim, e.o, e.attr1, 0, MDL_CULL_VFC | MDL_CULL_DIST, basetime);
}

void rendermapmodels()
{
    findvisiblemms(entities::storage);

    static int skipoq = 0;
    bool doquery = hasOQ && oqfrags && oqmm;

    for(octaentities *oe = visiblemms; oe; oe = oe->next) if(oe->distance>=0)
    {
        bool rendered = false;
        loopv(oe->mapmodels)
        {
            extentity &e = *entities::get(oe->mapmodels[i]);
            if(!e.visible) continue;
            if(!rendered)
            {
                rendered = true;
                oe->query = doquery && oe->distance>0 && !(++skipoq%oqmm) ? newquery(oe) : NULL;
                if(oe->query) startmodelquery(oe->query);
            }        
            rendermapmodel(e);
            e.visible = false;
        }
        if(rendered && oe->query) endmodelquery();
    }
    rendermodelbatches();
    resetmodelbatches();

    bool colormask = true;
    for(octaentities *oe = visiblemms; oe; oe = oe->next) if(oe->distance<0)
    {
        oe->query = doquery && !insideoe(oe, camera1->o) ? newquery(oe) : NULL;
        if(!oe->query) continue;
        if(colormask)
        {
            glDepthMask(GL_FALSE);
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
            nocolorshader->set();
            colormask = false;
        }
        startquery(oe->query);
        drawbb(oe->bbmin, ivec(oe->bbmax).sub(oe->bbmin));
        endquery(oe->query);
    }
    if(!colormask)
    {
        glDepthMask(GL_TRUE);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    }
}

static inline bool bbinsideva(const ivec &bo, const ivec &br, vtxarray *va)
{
    return bo.x >= va->bbmin.x && bo.y >= va->bbmin.y && va->o.z >= va->bbmin.z &&
        bo.x + br.x <= va->bbmax.x && bo.y + br.y <= va->bbmax.y && bo.z + br.z <= va->bbmax.z; 
}

static inline bool bboccluded(const ivec &bo, const ivec &br, cube *c, const ivec &o, int size)
{
    loopoctabox(o, size, bo, br)
    {
        ivec co(i, o.x, o.y, o.z, size);
        if(c[i].ext && c[i].ext->va)
        {
            vtxarray *va = c[i].ext->va;
            if(va->curvfc >= VFC_FOGGED || (va->occluded >= OCCLUDE_BB && bbinsideva(bo, br, va))) continue;
        }
        if(c[i].children && bboccluded(bo, br, c[i].children, co, size>>1)) continue;
        return false;
    }
    return true;
}

bool bboccluded(const ivec &bo, const ivec &br)
{
    int diff = (bo.x^(bo.x+br.x)) | (bo.y^(bo.y+br.y)) | (bo.z^(bo.z+br.z));
    if(diff&~((1<<worldscale)-1)) return false;
    int scale = worldscale-1;
    if(diff&(1<<scale)) return bboccluded(bo, br, worldroot, ivec(0, 0, 0), 1<<scale);
    cube *c = &worldroot[octastep(bo.x, bo.y, bo.z, scale)];
    if(c->ext && c->ext->va)
    {
        vtxarray *va = c->ext->va;
        if(va->curvfc >= VFC_FOGGED || (va->occluded >= OCCLUDE_BB && bbinsideva(bo, br, va))) return true;
    }
    scale--;
    while(c->children && !(diff&(1<<scale)))
    {
        c = &c->children[octastep(bo.x, bo.y, bo.z, scale)];
        if(c->ext && c->ext->va)
        {
            vtxarray *va = c->ext->va;
            if(va->curvfc >= VFC_FOGGED || (va->occluded >= OCCLUDE_BB && bbinsideva(bo, br, va))) return true;
        }
        scale--;
    }
    if(c->children) return bboccluded(bo, br, c->children, ivec(bo).mask(~((2<<scale)-1)), 1<<scale);
    return false;
}

VAR(outline, 0, 1, 1);
HVARP(outlinecolour, 0, 0, 0xFFFFFF);
VAR(dtoutline, 0, 1, 1);

void renderoutline()
{
    notextureshader->set();

    glDisable(GL_TEXTURE_2D);
    glEnableClientState(GL_VERTEX_ARRAY);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    bvec color((outlinecolour>>16)&0xFF, (outlinecolour>>8)&0xFF, outlinecolour&0xFF);
    glColor3f(color.x*ldrscaleb, color.y*ldrscaleb, color.z*ldrscaleb);

    enablepolygonoffset(GL_POLYGON_OFFSET_LINE);

    if(!dtoutline) glDisable(GL_DEPTH_TEST);

    vtxarray *prev = NULL;
    for(vtxarray *va = visibleva; va; va = va->next)
    {
        if(va->occluded >= OCCLUDE_BB) continue;
        if((!va->texs || va->occluded >= OCCLUDE_GEOM) && !va->alphaback && !va->alphafront && !va->refracttris) continue;

        if(!prev || va->vbuf != prev->vbuf)
        {
            if(hasVBO)
            {
                glBindBuffer_(GL_ARRAY_BUFFER_ARB, va->vbuf);
                glBindBuffer_(GL_ELEMENT_ARRAY_BUFFER_ARB, va->ebuf);
            }
            glVertexPointer(3, GL_FLOAT, VTXSIZE, va->vdata[0].pos.v);
        }

        if(va->texs && va->occluded < OCCLUDE_GEOM)
        {
            drawvatris(va, 3*va->tris, va->edata);
            xtravertsva += va->verts;
        }
        if(va->alphaback || va->alphafront || va->refract)
        {
            drawvatris(va, 3*(va->alphabacktris + va->alphafronttris + va->refracttris), &va->edata[3*(va->tris + va->blendtris)]);
            xtravertsva += 3*(va->alphabacktris + va->alphafronttris + va->refracttris);
        }
        
        prev = va;
    }

    if(!dtoutline) glEnable(GL_DEPTH_TEST);

    disablepolygonoffset(GL_POLYGON_OFFSET_LINE);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    if(hasVBO)
    {
        glBindBuffer_(GL_ARRAY_BUFFER_ARB, 0);
        glBindBuffer_(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
    }
    glDisableClientState(GL_VERTEX_ARRAY);
    glEnable(GL_TEXTURE_2D);

    defaultshader->set();
}

HVAR(blendbrushcolor, 0, 0x0000C0, 0xFFFFFF);

void renderblendbrush(GLuint tex, float x, float y, float w, float h)
{
    static Shader *blendbrushshader = NULL;
    if(!blendbrushshader) blendbrushshader = lookupshaderbyname("blendbrush");
    blendbrushshader->set();

    glEnableClientState(GL_VERTEX_ARRAY);

    glDepthFunc(GL_LEQUAL);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_TEXTURE_2D); 
    glBindTexture(GL_TEXTURE_2D, tex);
    bvec color((blendbrushcolor>>16)&0xFF, (blendbrushcolor>>8)&0xFF, blendbrushcolor&0xFF);
    glColor4f(color.x*ldrscaleb, color.y*ldrscaleb, color.z*ldrscaleb, 0.25f);

    LOCALPARAM(texgenS, (1.0f/w, 0, 0, -x/w));
    LOCALPARAM(texgenT, (0, 1.0f/h, 0, -y/h));

    vtxarray *prev = NULL;
    for(vtxarray *va = visibleva; va; va = va->next)
    {
        if(!va->texs || va->occluded >= OCCLUDE_GEOM) continue;
        if(va->o.x + va->size <= x || va->o.y + va->size <= y || va->o.x >= x + w || va->o.y >= y + h) continue;

        if(!prev || va->vbuf != prev->vbuf)
        {
            if(hasVBO)
            {
                glBindBuffer_(GL_ARRAY_BUFFER_ARB, va->vbuf);
                glBindBuffer_(GL_ELEMENT_ARRAY_BUFFER_ARB, va->ebuf);
            }
            glVertexPointer(3, GL_FLOAT, VTXSIZE, va->vdata[0].pos.v);
        }

        drawvatris(va, 3*va->tris, va->edata);
        xtravertsva += va->verts;

        prev = va;
    }

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);

    glDepthFunc(GL_LESS);

    if(hasVBO)
    {
        glBindBuffer_(GL_ARRAY_BUFFER_ARB, 0);
        glBindBuffer_(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
    }
    glDisableClientState(GL_VERTEX_ARRAY);

    notextureshader->set();
}
 
int calcbbsidemask(const ivec &bbmin, const ivec &bbmax, const vec &lightpos, float lightradius, float bias)
{
    vec pmin = bbmin.tovec().sub(lightpos).div(lightradius), pmax = bbmax.tovec().sub(lightpos).div(lightradius);
    int mask = 0x3F;
    float dp1 = pmax.x + pmax.y, dn1 = pmax.x - pmin.y, ap1 = fabs(dp1), an1 = fabs(dn1),
          dp2 = pmin.x + pmin.y, dn2 = pmin.x - pmax.y, ap2 = fabs(dp2), an2 = fabs(dn2);
    if(ap1 > bias*an1 && ap2 > bias*an2)
        mask &= (3<<4)
            | (dp1 >= 0 ? (1<<0)|(1<<2) : (2<<0)|(2<<2))
            | (dp2 >= 0 ? (1<<0)|(1<<2) : (2<<0)|(2<<2));
    if(an1 > bias*ap1 && an2 > bias*ap2)
        mask &= (3<<4)
            | (dn1 >= 0 ? (1<<0)|(2<<2) : (2<<0)|(1<<2))
            | (dn2 >= 0 ? (1<<0)|(2<<2) : (2<<0)|(1<<2));

    dp1 = pmax.y + pmax.z, dn1 = pmax.y - pmin.z, ap1 = fabs(dp1), an1 = fabs(dn1),
    dp2 = pmin.y + pmin.z, dn2 = pmin.y - pmax.z, ap2 = fabs(dp2), an2 = fabs(dn2);
    if(ap1 > bias*an1 && ap2 > bias*an2)
        mask &= (3<<0)
            | (dp1 >= 0 ? (1<<2)|(1<<4) : (2<<2)|(2<<4))
            | (dp2 >= 0 ? (1<<2)|(1<<4) : (2<<2)|(2<<4));
    if(an1 > bias*ap1 && an2 > bias*ap2)
        mask &= (3<<0)
            | (dn1 >= 0 ? (1<<2)|(2<<4) : (2<<2)|(1<<4))
            | (dn2 >= 0 ? (1<<2)|(2<<4) : (2<<2)|(1<<4));

    dp1 = pmax.z + pmax.x, dn1 = pmax.z - pmin.x, ap1 = fabs(dp1), an1 = fabs(dn1),
    dp2 = pmin.z + pmin.x, dn2 = pmin.z - pmax.x, ap2 = fabs(dp2), an2 = fabs(dn2);
    if(ap1 > bias*an1 && ap2 > bias*an2)
        mask &= (3<<2)
            | (dp1 >= 0 ? (1<<4)|(1<<0) : (2<<4)|(2<<0))
            | (dp2 >= 0 ? (1<<4)|(1<<0) : (2<<4)|(2<<0));
    if(an1 > bias*ap1 && an2 > bias*ap2)
        mask &= (3<<2)
            | (dn1 >= 0 ? (1<<4)|(2<<0) : (2<<4)|(1<<0))
            | (dn2 >= 0 ? (1<<4)|(2<<0) : (2<<4)|(1<<0));

    return mask;
}

int calcbbtetramask(const ivec &bbmin, const ivec &bbmax, const vec &lightpos, float lightradius, float bias)
{
    // top 1: +1, +1, +1
    // top 2: -1, -1, +1
    // bot 1: +1, -1, -1,
    // bot 2: -1, +1, -1

    vec pmin = bbmin.tovec().sub(lightpos).div(lightradius), pmax = bbmax.tovec().sub(lightpos).div(lightradius);
    int mask = 0xF;

    bias *= 2;

    // +x,+y top split, no bias
    if(pmin.x + pmin.y > 0) mask &= ~(1<<1);
    else if(pmax.x + pmax.y < 0) mask &= ~(1<<0);

    // +x,-y bottom split, no bias
    if(pmin.x - pmax.y > 0) mask &= ~(1<<3);
    else if(pmax.x - pmin.y < 0) mask &= ~(1<<2);

    // +x,-y,+z % +x,+y,-z = +y,+z top 1/bottom 1 split
    if(pmin.y + pmin.z > bias) mask &= ~(1<<2);
    else if(pmax.y + pmax.z < -bias) mask &= ~(1<<0);

    // +x,+y,-z % -x,+y,+z = +x,+z top 1/bottom 2 split
    if(pmin.x + pmin.z > bias) mask &= ~(1<<3);
    else if(pmax.x + pmax.z < -bias) mask &= ~(1<<0);

    // -x,-y,-z % +x,-y,+z = -x,+z top 2/bottom 1 split
    if(pmin.z - pmax.x > bias) mask &= ~(1<<2);
    else if(pmax.z - pmin.x < -bias) mask &= ~(1<<1);

    // -x,+y,+z % -x,-y,-z = -y,+z top 2/bottom 2 split
    if(pmin.z - pmax.y > bias) mask &= ~(1<<3);
    else if(pmax.z - pmin.y < -bias) mask &= ~(1<<1);

    return mask;
}

int calcspheresidemask(const vec &p, float radius, float bias)
{
    // p is in the cubemap's local coordinate system
    // bias = border/(size - border)
    float dxyp = p.x + p.y, dxyn = p.x - p.y, axyp = fabs(dxyp), axyn = fabs(dxyn),
          dyzp = p.y + p.z, dyzn = p.y - p.z, ayzp = fabs(dyzp), ayzn = fabs(dyzn),
          dzxp = p.z + p.x, dzxn = p.z - p.x, azxp = fabs(dzxp), azxn = fabs(dzxn);
    int mask = 0x3F;
    radius *= SQRT2;
    if(axyp > bias*axyn + radius) mask &= dxyp < 0 ? ~((1<<0)|(1<<2)) : ~((2<<0)|(2<<2));
    if(axyn > bias*axyp + radius) mask &= dxyn < 0 ? ~((1<<0)|(2<<2)) : ~((2<<0)|(1<<2));
    if(ayzp > bias*ayzn + radius) mask &= dyzp < 0 ? ~((1<<2)|(1<<4)) : ~((2<<2)|(2<<4));
    if(ayzn > bias*ayzp + radius) mask &= dyzn < 0 ? ~((1<<2)|(2<<4)) : ~((2<<2)|(1<<4));
    if(azxp > bias*azxn + radius) mask &= dzxp < 0 ? ~((1<<4)|(1<<0)) : ~((2<<4)|(2<<0));
    if(azxn > bias*azxp + radius) mask &= dzxn < 0 ? ~((1<<4)|(2<<0)) : ~((2<<4)|(1<<0));
    return mask;
}

int calcspheretetramask(const vec &p, float radius, float bias)
{
    // p is in the cubemap's local coordinate system
    // bias = border/(size - border)
    int mask = 0xF;

    bias *= 2;
    radius *= SQRT2;

    // +x,+y top split, no bias
    float d = p.x + p.y;
    if(fabs(d) > radius) mask &= d >= 0 ? ~(1<<1) : ~(1<<0);
    // +x,-y bottom split, no bias
    d = p.x - p.y;
    if(fabs(d) > radius) mask &= d >= 0 ? ~(1<<3) : ~(1<<2);

    // +x,-y,+z % +x,+y,-z = +y,+z top 1/bottom 1 split
    d = p.y + p.z;
    if(fabs(d) > radius + bias) mask &= d >= 0 ? ~(1<<2) : ~(1<<0);

    // +x,+y,-z % -x,+y,+z = +x,+z top 1/bottom 2 split
    d = p.x + p.z;
    if(fabs(d) > radius + bias) mask &= d >= 0 ? ~(1<<3) : ~(1<<0);

    // -x,-y,-z % +x,-y,+z = -x,+z top 2/bottom 1 split
    d = p.z - p.x;
    if(fabs(d) > radius + bias) mask &= d >= 0 ? ~(1<<2) : ~(1<<1);

    // -x,+y,+z % -x,-y,-z = -y,+z top 2/bottom 2 split
    d = p.z - p.y;
    if(fabs(d) > radius + bias) mask &= d >= 0 ? ~(1<<3) : ~(1<<1);

    return mask;
}

int calctrisidemask(const vec &p1, const vec &p2, const vec &p3, float bias)
{
    // p1, p2, p3 are in the cubemap's local coordinate system
    // bias = border/(size - border)
    int mask = 0x3F;
    float dp1 = p1.x + p1.y, dn1 = p1.x - p1.y, ap1 = fabs(dp1), an1 = fabs(dn1),
          dp2 = p2.x + p2.y, dn2 = p2.x - p2.y, ap2 = fabs(dp2), an2 = fabs(dn2),
          dp3 = p3.x + p3.y, dn3 = p3.x - p3.y, ap3 = fabs(dp3), an3 = fabs(dn3);
    if(ap1 > bias*an1 && ap2 > bias*an2 && ap3 > bias*an3)
        mask &= (3<<4)
            | (dp1 >= 0 ? (1<<0)|(1<<2) : (2<<0)|(2<<2))
            | (dp2 >= 0 ? (1<<0)|(1<<2) : (2<<0)|(2<<2))
            | (dp3 >= 0 ? (1<<0)|(1<<2) : (2<<0)|(2<<2));
    if(an1 > bias*ap1 && an2 > bias*ap2 && an3 > bias*ap3)
        mask &= (3<<4)
            | (dn1 >= 0 ? (1<<0)|(2<<2) : (2<<0)|(1<<2))
            | (dn2 >= 0 ? (1<<0)|(2<<2) : (2<<0)|(1<<2))
            | (dn3 >= 0 ? (1<<0)|(2<<2) : (2<<0)|(1<<2));

    dp1 = p1.y + p1.z, dn1 = p1.y - p1.z, ap1 = fabs(dp1), an1 = fabs(dn1),
    dp2 = p2.y + p2.z, dn2 = p2.y - p2.z, ap2 = fabs(dp2), an2 = fabs(dn2),
    dp3 = p3.y + p3.z, dn3 = p3.y - p3.z, ap3 = fabs(dp3), an3 = fabs(dn3);
    if(ap1 > bias*an1 && ap2 > bias*an2 && ap3 > bias*an3)
        mask &= (3<<0)
            | (dp1 >= 0 ? (1<<2)|(1<<4) : (2<<2)|(2<<4))
            | (dp2 >= 0 ? (1<<2)|(1<<4) : (2<<2)|(2<<4))
            | (dp3 >= 0 ? (1<<2)|(1<<4) : (2<<2)|(2<<4));
    if(an1 > bias*ap1 && an2 > bias*ap2 && an3 > bias*ap3)
        mask &= (3<<0)
            | (dn1 >= 0 ? (1<<2)|(2<<4) : (2<<2)|(1<<4))
            | (dn2 >= 0 ? (1<<2)|(2<<4) : (2<<2)|(1<<4))
            | (dn3 >= 0 ? (1<<2)|(2<<4) : (2<<2)|(1<<4));

    dp1 = p1.z + p1.x, dn1 = p1.z - p1.x, ap1 = fabs(dp1), an1 = fabs(dn1),
    dp2 = p2.z + p2.x, dn2 = p2.z - p2.x, ap2 = fabs(dp2), an2 = fabs(dn2),
    dp3 = p3.z + p3.x, dn3 = p3.z - p3.x, ap3 = fabs(dp3), an3 = fabs(dn3);
    if(ap1 > bias*an1 && ap2 > bias*an2 && ap3 > bias*an3)
        mask &= (3<<2)
            | (dp1 >= 0 ? (1<<4)|(1<<0) : (2<<4)|(2<<0))
            | (dp2 >= 0 ? (1<<4)|(1<<0) : (2<<4)|(2<<0))
            | (dp3 >= 0 ? (1<<4)|(1<<0) : (2<<4)|(2<<0));
    if(an1 > bias*ap1 && an2 > bias*ap2 && an3 > bias*ap3)
        mask &= (3<<2)
            | (dn1 >= 0 ? (1<<4)|(2<<0) : (2<<4)|(1<<0))
            | (dn2 >= 0 ? (1<<4)|(2<<0) : (2<<4)|(1<<0))
            | (dn3 >= 0 ? (1<<4)|(2<<0) : (2<<4)|(1<<0));

    return mask;
}

int calctritetramask(const vec &p1, const vec &p2, const vec &p3, float bias)
{
    // p1, p2, p3 are in the cubemap's local coordinate system
    // bias = border/(size - border)
    int mask = 0xF;

    bias *= 2;

    // +x,+y top split, no bias
    float d1 = p1.x + p1.y, d2 = p2.x + p2.y, d3 = p3.x + p3.y;
    if(d1 > 0)
    {
        if(d2 > 0 && d3 > 0) mask &= ~(1<<1);
    }
    else if(d1 < 0 && d2 < 0 && d3 < 0) mask &= ~(1<<0);

    // +x,-y bottom split, no bias
    d1 = p1.x - p1.y, d2 = p2.x - p2.y, d3 = p3.x - p3.y;
    if(d1 > 0)
    {
        if(d2 > 0 && d3 > 0) mask &= ~(1<<3);
    }
    else if(d1 < 0 && d2 < 0 && d3 < 0) mask &= ~(1<<2);

    // +x,-y,+z % +x,+y,-z = +y,+z top 1/bottom 1 split
    d1 = p1.y + p1.z, d2 = p2.y + p2.z, d3 = p3.y + p3.z;
    if(d1 > bias)
    {
         if(d2 > bias && d3 > bias) mask &= ~(1<<2);
    }
    else if(d1 < -bias && d2 < -bias && d3 < -bias) mask &= ~(1<<0);

    // +x,+y,-z % -x,+y,+z = +x,+z top 1/bottom 2 split
    d1 = p1.x + p1.z, d2 = p2.x + p2.z, d3 = p3.x + p3.z;
    if(d1 > bias)
    {
        if(d2 > bias && d3 > bias) mask &= ~(1<<3);
    }
    else if(d1 < -bias && d2 < -bias && d3 < -bias) mask &= ~(1<<0);

    // -x,-y,-z % +x,-y,+z = -x,+z top 2/bottom 1 split
    d1 = p1.z - p1.x, d2 = p2.z - p2.x, d3 = p3.z - p3.x;
    if(d1 > bias)
    {
        if(d2 > bias && d3 > bias) mask &= ~(1<<2);
    }
    else if(d1 < -bias && d2 < -bias && d3 < -bias) mask &= ~(1<<1);

    // -x,+y,+z % -x,-y,-z = -y,+z top 2/bottom 2 split
    d1 = p1.z - p1.y, d2 = p2.z - p2.y, d3 = p3.z - p3.y;
    if(d1 > bias)
    {
        if(d2 > bias && d3 > bias) mask &= ~(1<<3);
    }
    else if(d1 < -bias && d2 < -bias && d3 < -bias) mask &= ~(1<<1);

    return mask;
}

int cullfrustumsides(const vec &lightpos, float lightradius, float size, float border)
{
    int sides = 0x3F, masks[6] = { 3<<4, 3<<4, 3<<0, 3<<0, 3<<2, 3<<2 };
    float scale = (size - 2*border)/size, bias = border / (float)(size - border);
    // check if cone enclosing side would cross frustum plane 
    scale = 2 / (scale*scale + 2);
    loopi(5) if(vfcP[i].dist(lightpos) <= -0.03125f)
    {
        vec n = vec(vfcP[i]).div(lightradius);
        float len = scale*n.squaredlen();
        if(n.x*n.x > len) sides &= n.x < 0 ? ~(1<<0) : ~(2 << 0);
        if(n.y*n.y > len) sides &= n.y < 0 ? ~(1<<2) : ~(2 << 2);
        if(n.z*n.z > len) sides &= n.z < 0 ? ~(1<<4) : ~(2 << 4);
    }
    if (vfcP[4].dist(lightpos) >= vfcDfog + 0.03125f)
    {
        vec n = vec(vfcP[4]).div(lightradius);
        float len = scale*n.squaredlen();
        if(n.x*n.x > len) sides &= n.x >= 0 ? ~(1<<0) : ~(2 << 0);
        if(n.y*n.y > len) sides &= n.y >= 0 ? ~(1<<2) : ~(2 << 2);
        if(n.z*n.z > len) sides &= n.z >= 0 ? ~(1<<4) : ~(2 << 4);
    }
    // this next test usually clips off more sides than the former, but occasionally clips fewer/different ones, so do both and combine results
    // check if frustum corners/origin cross plane sides
    // infinite version, assumes frustum corners merely give direction and extend to infinite distance
    vec p = vec(camera1->o).sub(lightpos).div(lightradius);
    float dp = p.x + p.y, dn = p.x - p.y, ap = fabs(dp), an = fabs(dn);
    masks[0] |= ap <= bias*an ? 0x3F : (dp >= 0 ? (1<<0)|(1<<2) : (2<<0)|(2<<2));
    masks[1] |= an <= bias*ap ? 0x3F : (dn >= 0 ? (1<<0)|(2<<2) : (2<<0)|(1<<2));
    dp = p.y + p.z, dn = p.y - p.z, ap = fabs(dp), an = fabs(dn);
    masks[2] |= ap <= bias*an ? 0x3F : (dp >= 0 ? (1<<2)|(1<<4) : (2<<2)|(2<<4));
    masks[3] |= an <= bias*ap ? 0x3F : (dn >= 0 ? (1<<2)|(2<<4) : (2<<2)|(1<<4));
    dp = p.z + p.x, dn = p.z - p.x, ap = fabs(dp), an = fabs(dn);
    masks[4] |= ap <= bias*an ? 0x3F : (dp >= 0 ? (1<<4)|(1<<0) : (2<<4)|(2<<0));
    masks[5] |= an <= bias*ap ? 0x3F : (dn >= 0 ? (1<<4)|(2<<0) : (2<<4)|(1<<0));
    loopi(4)
    {
        vec n;
        switch(i)
        {
            case 0: n.cross(vfcP[0], vfcP[2]); break;
            case 1: n.cross(vfcP[3], vfcP[0]); break;
            case 2: n.cross(vfcP[2], vfcP[1]); break;
            case 3: n.cross(vfcP[1], vfcP[3]); break;
        }
        dp = n.x + n.y, dn = n.x - n.y, ap = fabs(dp), an = fabs(dn);
        if(ap > 0) masks[0] |= dp >= 0 ? (1<<0)|(1<<2) : (2<<0)|(2<<2);
        if(an > 0) masks[1] |= dn >= 0 ? (1<<0)|(2<<2) : (2<<0)|(1<<2);
        dp = n.y + n.z, dn = n.y - n.z, ap = fabs(dp), an = fabs(dn);
        if(ap > 0) masks[2] |= dp >= 0 ? (1<<2)|(1<<4) : (2<<2)|(2<<4);
        if(an > 0) masks[3] |= dn >= 0 ? (1<<2)|(2<<4) : (2<<2)|(1<<4);
        dp = n.z + n.x, dn = n.z - n.x, ap = fabs(dp), an = fabs(dn);
        if(ap > 0) masks[4] |= dp >= 0 ? (1<<4)|(1<<0) : (2<<4)|(2<<0);
        if(an > 0) masks[5] |= dn >= 0 ? (1<<4)|(2<<0) : (2<<4)|(1<<0);
    }
    return sides & masks[0] & masks[1] & masks[2] & masks[3] & masks[4] & masks[5];
}

int cullfrustumtetra(const vec &lightpos, float lightradius, float size, float border)
{
    int outside[5] = { 0, 0, 0, 0 };
    loopi(4) if(vfcP[i].dist(lightpos) < 0) outside[4] |= 1<<i;
    float dist = vfcP[4].dist(lightpos);
    if(dist < 0) outside[4] |= 1<<4; 
    if(dist > vfcDfog) outside[4] |= 1<<5;
    if(!outside[4]) return 0xF;

    float pr = SQRT3*lightradius;
    vec p[4] =
    {
        vec(lightpos).add(vec(-pr, pr, pr)),
        vec(lightpos).add(vec(pr, -pr, pr)),
        vec(lightpos).add(vec(pr, pr, -pr)),
        vec(lightpos).add(vec(-pr, -pr, -pr))
    }; 
    loopk(4)
    {
        loopi(4) if(vfcP[i].dist(p[k]) < 0) outside[k] |= 1<<i;
        float dist = vfcP[k].dist(p[k]);
        if(dist < 0) outside[k] |= 1<<4; 
        if(dist > vfcDfog) outside[k] |= 1<<5;
    }

    int mask = 0xF;
    if(outside[4] & outside[0] & outside[1] & outside[2]) mask &= ~(1<<0);
    if(outside[4] & outside[0] & outside[1] & outside[3]) mask &= ~(1<<1);
    if(outside[4] & outside[1] & outside[2] & outside[3]) mask &= ~(1<<2);
    if(outside[4] & outside[0] & outside[2] & outside[3]) mask &= ~(1<<3);
    return mask;
}

VAR(smbbcull, 0, 1, 1);
VAR(smdistcull, 0, 1, 1);
VAR(smnodraw, 0, 0, 1);

vec shadoworigin(0, 0, 0), shadowdir(0, 0, 0);
float shadowradius = 0, shadowbias = 0;
int shadowside = 0, shadowspot = 0;

vtxarray *shadowva = NULL;

static inline void addshadowva(vtxarray *va, float dist)
{
    va->rdistance = int(dist);

    int hash = clamp(int(dist*VASORTSIZE/shadowradius), 0, VASORTSIZE-1);
    vtxarray **prev = &vasort[hash], *cur = vasort[hash];

    while(cur && va->rdistance > cur->rdistance)
    {
        prev = &cur->rnext;
        cur = cur->rnext;
    }

    va->rnext = cur;
    *prev = va;
}

void sortshadowvas()
{
    shadowva = NULL;
    vtxarray **last = &shadowva;
    loopi(VASORTSIZE) if(vasort[i])
    {
        vtxarray *va = vasort[i];
        *last = va;
        while(va->rnext) va = va->rnext;
        last = &va->rnext;
    }
}

void findtetrashadowvas(vector<vtxarray *> &vas)
{
    loopv(vas)
    {
        vtxarray &v = *vas[i];
        float dist = vadist(&v, shadoworigin);
        if(dist < shadowradius || !smdistcull)
        {
            v.shadowmask = !smbbcull ? 0x3F : (v.children.length() || v.mapmodels.length() ? 
                                calcbbtetramask(v.bbmin, v.bbmax, shadoworigin, shadowradius, shadowbias) :
                                calcbbtetramask(v.geommin, v.geommax, shadoworigin, shadowradius, shadowbias));
            addshadowva(&v, dist);
            if(v.children.length()) findtetrashadowvas(v.children);
        }
    }
}

void findshadowvas(vector<vtxarray *> &vas)
{
    loopv(vas)
    {
        vtxarray &v = *vas[i];
        float dist = vadist(&v, shadoworigin);
        if(dist < shadowradius || !smdistcull)
        {
            v.shadowmask = !smbbcull ? 0x3F : (v.children.length() || v.mapmodels.length() ?
                                calcbbsidemask(v.bbmin, v.bbmax, shadoworigin, shadowradius, shadowbias) :
                                calcbbsidemask(v.geommin, v.geommax, shadoworigin, shadowradius, shadowbias));
            addshadowva(&v, dist);
            if(v.children.length()) findshadowvas(v.children);
        }
    }
}

void findcsmshadowvas(vector<vtxarray *> &vas)
{
    loopv(vas)
    {
        vtxarray &v = *vas[i];
        ivec bbmin, bbmax;
        if(v.children.length() || v.mapmodels.length()) { bbmin = v.bbmin; bbmax = v.bbmax; }
        else { bbmin = v.geommin; bbmax = v.geommax; }
        v.shadowmask = calcbbcsmsplits(bbmin, bbmax);
        if(v.shadowmask)
        {
            float dist = shadowdir.project_bb(bbmin, bbmax) - shadowbias;
            addshadowva(&v, dist);
            if(v.children.length()) findcsmshadowvas(v.children);
        }
    }
}

void findrsmshadowvas(vector<vtxarray *> &vas)
{
    loopv(vas)
    {
        vtxarray &v = *vas[i];
        ivec bbmin, bbmax;
        if(v.children.length() || v.mapmodels.length()) { bbmin = v.bbmin; bbmax = v.bbmax; }
        else { bbmin = v.geommin; bbmax = v.geommax; }
        v.shadowmask = calcbbrsmsplits(bbmin, bbmax);
        if(v.shadowmask)
        {
            float dist = shadowdir.project_bb(bbmin, bbmax) - shadowbias;
            addshadowva(&v, dist);
            if(v.children.length()) findrsmshadowvas(v.children);
        }
    }
}

void findspotshadowvas(vector<vtxarray *> &vas)
{
    loopv(vas)
    {
        vtxarray &v = *vas[i];
        float dist = vadist(&v, shadoworigin);
        if(dist < shadowradius || !smdistcull)
        {
            v.shadowmask = !smbbcull || (v.children.length() || v.mapmodels.length() ?
                                bbinsidespot(shadoworigin, shadowdir, shadowspot, v.bbmin, v.bbmax) :
                                bbinsidespot(shadoworigin, shadowdir, shadowspot, v.geommin, v.geommax)) ? 1 : 0;
            addshadowva(&v, dist);
            if(v.children.length()) findspotshadowvas(v.children);
        }
    }
}

void findshadowvas()
{
    memset(vasort, 0, sizeof(vasort));
    switch(shadowmapping)
    {
        case SM_REFLECT: findrsmshadowvas(varoot); break;
        case SM_CUBEMAP: findshadowvas(varoot); break;
        case SM_TETRA: findtetrashadowvas(varoot); break;
        case SM_CASCADE: findcsmshadowvas(varoot); break;
        case SM_SPOT: findspotshadowvas(varoot); break;
    }
    sortshadowvas();
}

void rendershadowmapworld()
{
    if(shadowmapping == SM_TETRA) SETSHADER(tetraworld);
    else SETSHADER(shadowmapworld);

    glEnableClientState(GL_VERTEX_ARRAY);

    vtxarray *prev = NULL;
    for(vtxarray *va = shadowva; va; va = va->rnext)
    {
        if(!va->tris || !(va->shadowmask&(1<<shadowside))) continue;

        if(!prev || va->vbuf != prev->vbuf)
        {
            if(hasVBO)
            {
                glBindBuffer_(GL_ARRAY_BUFFER_ARB, va->vbuf);
                glBindBuffer_(GL_ELEMENT_ARRAY_BUFFER_ARB, va->ebuf);
            }
            glVertexPointer(3, GL_FLOAT, VTXSIZE, va->vdata[0].pos.v);
            glNormalPointer(GL_BYTE, VTXSIZE, va->vdata[0].norm.v);
        }

        if(!smnodraw) drawvatris(va, 3*va->tris, va->edata);
        xtravertsva += va->verts;

        prev = va;
    }

    if(hasVBO)
    {
        glBindBuffer_(GL_ARRAY_BUFFER_ARB, 0);
        glBindBuffer_(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
    }
    glDisableClientState(GL_VERTEX_ARRAY);
}

static octaentities *shadowmms = NULL;

void findshadowmms()
{
    shadowmms = NULL;
    octaentities **lastmms = &shadowmms; 
    for(vtxarray *va = shadowva; va; va = va->rnext)
    {
        loopvj(va->mapmodels)
        {
            octaentities *oe = va->mapmodels[j];
            switch(shadowmapping)
            {
                case SM_REFLECT:
                    break;
                case SM_CASCADE:
                    if(!calcbbcsmsplits(oe->bbmin, oe->bbmax))
                        continue;
                    break;
                case SM_TETRA:
                case SM_CUBEMAP: 
                    if(smdistcull && shadoworigin.dist_to_bb(oe->bbmin, oe->bbmax) >= shadowradius)
                        continue;
                    break;
                case SM_SPOT:
                    if(smdistcull && shadoworigin.dist_to_bb(oe->bbmin, oe->bbmax) >= shadowradius)
                        continue;
                    if(smbbcull && !bbinsidespot(shadoworigin, shadowdir, shadowspot, oe->bbmin, oe->bbmax))
                        continue; 
                    break;
            }
            oe->rnext = NULL;
            *lastmms = oe;
            lastmms = &oe->rnext;
        }
    }
}

void batchshadowmapmodels()
{
    if(!shadowmms) return;
    for(octaentities *oe = shadowmms; oe; oe = oe->rnext)
    {
        loopvk(oe->mapmodels)
        {
            extentity &e = *entities::get(oe->mapmodels[k]);
            if(e.flags&extentity::F_NOVIS) continue;
            e.visible = true;
        }
    }
    for(octaentities *oe = shadowmms; oe; oe = oe->rnext)
    {
        loopvj(oe->mapmodels)
        {
            extentity &e = *entities::get(oe->mapmodels[j]);
            if(!e.visible) continue;
            rendermapmodel(e);
            e.visible = false;
        }
    }
}

VAR(oqdist, 0, 256, 1024);
VAR(zpass, 0, 1, 1);
VAR(glowpass, 0, 1, 1);
VAR(envpass, 0, 1, 1);

struct renderstate
{
    bool colormask, depthmask;
    int alphaing;
    GLuint vbuf;
    int diffusetmu;
    GLfloat color[4], fogcolor[4];
    vec colorscale;
    float alphascale;
    float refractscale;
    vec refractcolor;
    int blendx, blendy;
    GLuint textures[8];
    Slot *slot, *texgenslot;
    VSlot *vslot, *texgenvslot;
    float texgenscrollS, texgenscrollT;
    int texgendim;

    renderstate() : colormask(true), depthmask(true), alphaing(0), vbuf(0), diffusetmu(0), colorscale(1, 1, 1), alphascale(0), refractscale(0), refractcolor(1, 1, 1), blendx(-1), blendy(-1), slot(NULL), texgenslot(NULL), vslot(NULL), texgenvslot(NULL), texgenscrollS(0), texgenscrollT(0), texgendim(-1)
    {
        loopk(4) color[k] = 1;
        loopk(8) textures[k] = 0;
    }
};

void renderquery(renderstate &cur, occludequery *query, vtxarray *va, bool full = true)
{
    nocolorshader->set();
    if(cur.colormask) { cur.colormask = false; glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); }
    if(cur.depthmask) { cur.depthmask = false; glDepthMask(GL_FALSE); }

    vec camera(camera1->o);

    startquery(query);

    if(full) drawbb(ivec(va->bbmin).sub(1), ivec(va->bbmax).sub(va->bbmin).add(2), camera);
    else drawbb(va->geommin, ivec(va->geommax).sub(va->geommin), camera);

    endquery(query);
}

enum
{
    RENDERPASS_GBUFFER = 0,
    RENDERPASS_Z,
    RENDERPASS_CAUSTICS,
    RENDERPASS_GBUFFER_BLEND,
    RENDERPASS_RSM,
    RENDERPASS_RSM_BLEND
};

struct geombatch
{
    const elementset &es;
    VSlot &vslot;
    ushort *edata;
    vtxarray *va;
    int next, batch;

    geombatch(const elementset &es, ushort *edata, vtxarray *va)
      : es(es), vslot(lookupvslot(es.texture)), edata(edata), va(va),
        next(-1), batch(-1)
    {}

    int compare(const geombatch &b) const
    {
        if(va->vbuf < b.va->vbuf) return -1;
        if(va->vbuf > b.va->vbuf) return 1;
        if(es.layer&LAYER_BOTTOM)
        {
            if(!(b.es.layer&LAYER_BOTTOM)) return 1;
            int x1 = va->o.x&~0xFFF, x2 = b.va->o.x&~0xFFF;
            if(x1 < x2) return -1;
            if(x1 > x2) return 1;
            int y1 = va->o.y&~0xFFF, y2 = b.va->o.y&~0xFFF;
            if(y1 < y2) return -1;
            if(y1 > y2) return 1;
        }
        else if(b.es.layer&LAYER_BOTTOM) return -1;
        if(vslot.slot->shader < b.vslot.slot->shader) return -1;
        if(vslot.slot->shader > b.vslot.slot->shader) return 1;
        if(vslot.slot->params.length() < b.vslot.slot->params.length()) return -1;
        if(vslot.slot->params.length() > b.vslot.slot->params.length()) return 1;
        if(es.texture < b.es.texture) return -1;
        if(es.texture > b.es.texture) return 1;
        if(es.envmap < b.es.envmap) return -1;
        if(es.envmap > b.es.envmap) return 1;
        if(es.dim < b.es.dim) return -1;
        if(es.dim > b.es.dim) return 1;
        return 0;
    }
};

static vector<geombatch> geombatches;
static int firstbatch = -1, numbatches = 0;

static void mergetexs(renderstate &cur, vtxarray *va, elementset *texs = NULL, int numtexs = 0, ushort *edata = NULL)
{
    if(!texs) 
    { 
        texs = va->eslist; 
        numtexs = va->texs; 
        edata = va->edata;
        if(cur.alphaing)
        {
            texs += va->texs + va->blends;
            edata += 3*(va->tris + va->blendtris);
            numtexs = va->alphaback;
            if(cur.alphaing > 1) numtexs += va->alphafront + va->refract;
        }
    }

    if(firstbatch < 0)
    {
        firstbatch = geombatches.length();
        numbatches = numtexs;
        loopi(numtexs-1) 
        {
            geombatches.add(geombatch(texs[i], edata, va)).next = i+1;
            edata += texs[i].length;
        }
        geombatches.add(geombatch(texs[numtexs-1], edata, va));
        return;
    }
    
    int prevbatch = -1, curbatch = firstbatch, curtex = 0;
    do
    {
        geombatch &b = geombatches.add(geombatch(texs[curtex], edata, va));
        edata += texs[curtex].length;
        int dir = -1;
        while(curbatch >= 0)
        {
            dir = b.compare(geombatches[curbatch]);
            if(dir <= 0) break;
            prevbatch = curbatch;
            curbatch = geombatches[curbatch].next;
        }
        if(!dir)
        {
            int last = curbatch, next;
            for(;;)
            {
                next = geombatches[last].batch;
                if(next < 0) break;
                last = next;
            }
            if(last==curbatch)
            {
                b.batch = curbatch;
                b.next = geombatches[curbatch].next;
                if(prevbatch < 0) firstbatch = geombatches.length()-1;
                else geombatches[prevbatch].next = geombatches.length()-1;
                curbatch = geombatches.length()-1;
            }
            else
            {
                b.batch = next;
                geombatches[last].batch = geombatches.length()-1;
            }    
        }
        else 
        {
            numbatches++;
            b.next = curbatch;
            if(prevbatch < 0) firstbatch = geombatches.length()-1;
            else geombatches[prevbatch].next = geombatches.length()-1;
            prevbatch = geombatches.length()-1;
        }
    }
    while(++curtex < numtexs);
}

static void changevbuf(renderstate &cur, int pass, vtxarray *va)
{
    if(hasVBO)
    {
        glBindBuffer_(GL_ARRAY_BUFFER_ARB, va->vbuf);
        glBindBuffer_(GL_ELEMENT_ARRAY_BUFFER_ARB, va->ebuf);
    }
    cur.vbuf = va->vbuf;

    glVertexPointer(3, GL_FLOAT, VTXSIZE, va->vdata[0].pos.v);

    if(pass==RENDERPASS_GBUFFER || pass==RENDERPASS_RSM)
    {
        glTexCoordPointer(2, GL_FLOAT, VTXSIZE, &va->vdata[0].u);
        glNormalPointer(GL_BYTE, VTXSIZE, va->vdata[0].norm.v);
        glColorPointer(4, GL_UNSIGNED_BYTE, VTXSIZE, va->vdata[0].tangent.v);
    }
}

static void changebatchtmus(renderstate &cur, int pass, geombatch &b)
{
    bool changed = false;
    int tmu = cur.diffusetmu+1;
    if(b.vslot.slot->shader->type&SHADER_ENVMAP && b.es.envmap!=EMID_CUSTOM)
    {
        GLuint emtex = lookupenvmap(b.es.envmap);
        if(cur.textures[tmu]!=emtex)
        {
            glActiveTexture_(GL_TEXTURE0_ARB+tmu);
            glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, cur.textures[tmu] = emtex);
            changed = true;
        }
    }
    if(b.es.layer&LAYER_BOTTOM && (cur.blendx != (b.va->o.x&~0xFFF) || cur.blendy != (b.va->o.y&~0xFFF)))
    {
        glActiveTexture_(GL_TEXTURE7_ARB);
        bindblendtexture(b.va->o);
        cur.blendx = b.va->o.x&~0xFFF;
        cur.blendy = b.va->o.y&~0xFFF;
        changed = true;
    }  
    if(changed) glActiveTexture_(GL_TEXTURE0_ARB+cur.diffusetmu);
}

static void changeslottmus(renderstate &cur, int pass, Slot &slot, VSlot &vslot)
{
    if(pass==RENDERPASS_GBUFFER || pass==RENDERPASS_RSM)
    {
        GLuint diffusetex = slot.sts.empty() ? notexture->id : slot.sts[0].t->id;
        if(cur.textures[cur.diffusetmu]!=diffusetex)
            glBindTexture(GL_TEXTURE_2D, cur.textures[cur.diffusetmu] = diffusetex);
    }

    if(cur.alphaing)
    {
        float alpha = cur.alphaing > 1 ? vslot.alphafront : vslot.alphaback;
        if(cur.colorscale != vslot.colorscale || cur.alphascale != alpha) 
        {
            cur.colorscale = vslot.colorscale;
            cur.alphascale = alpha;
            GLOBALPARAM(colorparams, (alpha*vslot.colorscale.x, alpha*vslot.colorscale.y, alpha*vslot.colorscale.z, alpha));
        }
        if(cur.alphaing > 1 && vslot.refractscale > 0 && (cur.refractscale != vslot.refractscale || cur.refractcolor != vslot.refractcolor))
        {
            cur.refractscale = vslot.refractscale;
            cur.refractcolor = vslot.refractcolor;
            float refractscale = 0.5f/ldrscale*(1-alpha);
            GLOBALPARAM(refractparams, (vslot.refractcolor.x*refractscale, vslot.refractcolor.y*refractscale, vslot.refractcolor.z*refractscale, vslot.refractscale*viewh));
        }
    }
    else if(cur.colorscale != vslot.colorscale)
    {
        cur.colorscale = vslot.colorscale;
        GLOBALPARAM(colorparams, (vslot.colorscale.x, vslot.colorscale.y, vslot.colorscale.z, 1));
    }
    int tmu = cur.diffusetmu+1, envmaptmu = -1;
    if(slot.shader->type&SHADER_ENVMAP) envmaptmu = tmu++;
    loopvj(slot.sts)
    {
        Slot::Tex &t = slot.sts[j];
        if(t.type==TEX_DIFFUSE || t.combined>=0) continue;
        if(t.type==TEX_ENVMAP)
        {
            if(envmaptmu>=0 && cur.textures[envmaptmu]!=t.t->id)
            {
                glActiveTexture_(GL_TEXTURE0_ARB+envmaptmu);
                glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, cur.textures[envmaptmu] = t.t->id);
            }
            continue;
        }
        else if(cur.textures[tmu]!=t.t->id)
        {  
            glActiveTexture_(GL_TEXTURE0_ARB+tmu);
            glBindTexture(GL_TEXTURE_2D, cur.textures[tmu] = t.t->id);
        }
        tmu++;
    }
    glActiveTexture_(GL_TEXTURE0_ARB+cur.diffusetmu);

    cur.slot = &slot;
    cur.vslot = &vslot;
}

static void changetexgen(renderstate &cur, int dim, Slot &slot, VSlot &vslot)
{
    if(cur.texgenslot != &slot || cur.texgenvslot != &vslot)
    {
        Texture *curtex = !cur.texgenslot || cur.texgenslot->sts.empty() ? notexture : cur.texgenslot->sts[0].t,
                *tex = slot.sts.empty() ? notexture : slot.sts[0].t;
        if(!cur.texgenvslot || slot.sts.empty() ||
            (curtex->xs != tex->xs || curtex->ys != tex->ys ||
             cur.texgenvslot->rotation != vslot.rotation || cur.texgenvslot->scale != vslot.scale ||
             cur.texgenvslot->xoffset != vslot.xoffset || cur.texgenvslot->yoffset != vslot.yoffset ||
             cur.texgenvslot->scrollS != vslot.scrollS || cur.texgenvslot->scrollT != vslot.scrollT))
        {
            float xs = vslot.rotation>=2 && vslot.rotation<=4 ? -tex->xs : tex->xs,
                  ys = (vslot.rotation>=1 && vslot.rotation<=2) || vslot.rotation==5 ? -tex->ys : tex->ys,
                  scrollS = vslot.scrollS, scrollT = vslot.scrollT;
            if((vslot.rotation&5)==1) swap(scrollS, scrollT);
            scrollS *= lastmillis*tex->xs/xs;
            scrollT *= lastmillis*tex->ys/ys;
            if(cur.texgenscrollS != scrollS || cur.texgenscrollT != scrollT)
            {
                cur.texgenscrollS = scrollS;
                cur.texgenscrollT = scrollT;
                cur.texgendim = -1;
            }
        }
        cur.texgenslot = &slot;
        cur.texgenvslot = &vslot;
    }

    if(cur.texgendim == dim) return;
    GLOBALPARAM(texgenscroll, (cur.texgenscrollS, cur.texgenscrollT));

    cur.texgendim = dim;
}

static inline void changeshader(renderstate &cur, int pass, geombatch &b)
{
    VSlot &vslot = b.vslot;
    Slot &slot = *vslot.slot;
    if(pass == RENDERPASS_RSM)
    {
        if(b.es.layer&LAYER_BOTTOM) rsmworldshader->setvariant(0, 0, slot, vslot);
        else rsmworldshader->set(slot, vslot);
    }
    else if(cur.alphaing > 1 && vslot.refractscale > 0) slot.shader->setvariant(0, 1, slot, vslot);
    else if(b.es.layer&LAYER_BOTTOM) slot.shader->setvariant(0, 0, slot, vslot);
    else slot.shader->set(slot, vslot);
}

static void renderbatch(renderstate &cur, int pass, geombatch &b)
{
    int rendered = -1;
    for(geombatch *curbatch = &b;; curbatch = &geombatches[curbatch->batch])
    {
        ushort len = curbatch->es.length;
        if(len) 
        {
            if(rendered < 0)
            {
                changeshader(cur, pass, b);
                rendered = 0;
                gbatches++;
            }
            drawtris(len, curbatch->edata, curbatch->es.minvert, curbatch->es.maxvert); 
            vtris += len/3;
        }
        if(curbatch->batch < 0) break;
    }
}

static void resetbatches()
{
    geombatches.setsize(0);
    firstbatch = -1;
    numbatches = 0;
}

static void renderbatches(renderstate &cur, int pass)
{
    cur.slot = NULL;
    cur.vslot = NULL;
    int curbatch = firstbatch;
    if(curbatch >= 0)
    {
        if(!cur.depthmask) { cur.depthmask = true; glDepthMask(GL_TRUE); }
        if(!cur.colormask) { cur.colormask = true; glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); }
    }        
    while(curbatch >= 0)
    {
        geombatch &b = geombatches[curbatch];
        curbatch = b.next;

        if(cur.vbuf != b.va->vbuf) changevbuf(cur, pass, b.va);
        if(cur.vslot != &b.vslot) 
        {
            changeslottmus(cur, pass, *b.vslot.slot, b.vslot);
            if(cur.texgendim != b.es.dim || (cur.texgendim <= 2 && cur.texgenvslot != &b.vslot)) changetexgen(cur, b.es.dim, *b.vslot.slot, b.vslot);
        }
        else if(cur.texgendim != b.es.dim) changetexgen(cur, b.es.dim, *b.vslot.slot, b.vslot);
        if(pass == RENDERPASS_GBUFFER || pass == RENDERPASS_RSM) changebatchtmus(cur, pass, b);

        renderbatch(cur, pass, b);
    }

    resetbatches();
}

void renderzpass(renderstate &cur, vtxarray *va)
{
    if(cur.vbuf!=va->vbuf) changevbuf(cur, RENDERPASS_Z, va);
    if(!cur.depthmask) { cur.depthmask = true; glDepthMask(GL_TRUE); }
    if(cur.colormask) { cur.colormask = false; glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); }

    int firsttex = 0, numtris = va->tris;
    ushort *edata = va->edata;
    if(cur.alphaing)
    {
        firsttex += va->texs + va->blends;
        edata += 3*(va->tris + va->blendtris);
        numtris = va->alphabacktris + va->alphafronttris + va->refracttris;
        xtravertsva += 3*numtris;
    }
    else xtravertsva += va->verts;
    nocolorshader->set();
    drawvatris(va, 3*numtris, edata);
}

#define startvaquery(va, flush) \
    do { \
        if(va->query) \
        { \
            flush; \
            startquery(va->query); \
        } \
    } while(0)


#define endvaquery(va, flush) \
    do { \
        if(va->query) \
        { \
            flush; \
            endquery(va->query); \
        } \
    } while(0)

VAR(batchgeom, 0, 1, 1);

void renderva(renderstate &cur, vtxarray *va, int pass = RENDERPASS_GBUFFER, bool doquery = false)
{
    switch(pass)
    {
        case RENDERPASS_GBUFFER:
            if(!cur.alphaing) vverts += va->verts;
            if(doquery) startvaquery(va, { if(geombatches.length()) renderbatches(cur, pass); });
            mergetexs(cur, va);
            if(doquery) endvaquery(va, { if(geombatches.length()) renderbatches(cur, pass); });
            else if(!batchgeom && geombatches.length()) renderbatches(cur, pass);
            break;

        case RENDERPASS_GBUFFER_BLEND:
            if(doquery) startvaquery(va, { if(geombatches.length()) renderbatches(cur, RENDERPASS_GBUFFER); });
            mergetexs(cur, va, &va->eslist[va->texs], va->blends, va->edata + 3*va->tris);
            if(doquery) endvaquery(va, { if(geombatches.length()) renderbatches(cur, RENDERPASS_GBUFFER); });
            else if(!batchgeom && geombatches.length()) renderbatches(cur, RENDERPASS_GBUFFER);
            break;

        case RENDERPASS_CAUSTICS:
            if(cur.vbuf!=va->vbuf) changevbuf(cur, pass, va);
            drawvatris(va, 3*va->tris, va->edata);
            xtravertsva += va->verts;
            break;
 
        case RENDERPASS_Z:
            if(doquery) startvaquery(va, );
            renderzpass(cur, va);
            if(doquery) endvaquery(va, );
            break;

        case RENDERPASS_RSM:
            mergetexs(cur, va);
            if(!batchgeom && geombatches.length()) renderbatches(cur, pass);
            break;

        case RENDERPASS_RSM_BLEND:
            mergetexs(cur, va, &va->eslist[va->texs], va->blends, va->edata + 3*va->tris);
            if(!batchgeom && geombatches.length()) renderbatches(cur, RENDERPASS_RSM);
            break;
    }
}

void cleanupva()
{
    clearvas(worldroot);
    clearqueries();
}

void setupTMUs(renderstate &cur)
{
    glActiveTexture_(GL_TEXTURE0_ARB);
    glClientActiveTexture_(GL_TEXTURE0_ARB);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    GLOBALPARAM(colorparams, (1, 1, 1, 1));
    GLOBALPARAM(blendlayer, (1.0f));
    glColor4fv(cur.color);
}

void cleanupTMUs(renderstate &cur)
{
    glActiveTexture_(GL_TEXTURE0_ARB);
    glClientActiveTexture_(GL_TEXTURE0_ARB);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
}

VAR(oqgeom, 0, 1, 1);

extern int rhinoq;
extern void renderradiancehints();

void rendergeom()
{
    bool mainpass = !envmapping,
         doOQ = hasOQ && oqfrags && oqgeom && mainpass,
         doZP = doOQ && zpass;
    renderstate cur;
    if(!doZP) 
    {
        setupTMUs(cur);
    }

    resetbatches();

    glEnableClientState(GL_VERTEX_ARRAY);

    int blends = 0;
    for(vtxarray *va = visibleva; va; va = va->next)
    {
        if(!va->texs) continue;
        if(doOQ && (zpass || va->distance > oqdist) && !insideva(va, camera1->o))
        {
            if(va->parent && va->parent->occluded >= OCCLUDE_BB)
            {
                va->query = NULL;
                va->occluded = OCCLUDE_PARENT;
                continue;
            }
            va->occluded = va->query && va->query->owner == va && checkquery(va->query) ? min(va->occluded+1, int(OCCLUDE_BB)) : OCCLUDE_NOTHING;
            va->query = newquery(va);
            if((!va->query && zpass) || !va->occluded)
                va->occluded = pvsoccluded(va->geommin, va->geommax) ? OCCLUDE_GEOM : OCCLUDE_NOTHING;
            if(va->occluded >= OCCLUDE_GEOM)
            {
                if(va->query) 
                {
                    if(!zpass && geombatches.length()) renderbatches(cur, RENDERPASS_GBUFFER);
                    renderquery(cur, va->query, va);
                }
                continue;
            }
        }
        else
        {
            va->query = NULL;
            va->occluded = pvsoccluded(va->geommin, va->geommax) ? OCCLUDE_GEOM : OCCLUDE_NOTHING;
            if(va->occluded >= OCCLUDE_GEOM) continue;
        }

        if(!doZP) blends += va->blends;
        renderva(cur, va, doZP ? RENDERPASS_Z : RENDERPASS_GBUFFER, doOQ);
    }

    if(geombatches.length()) renderbatches(cur, RENDERPASS_GBUFFER);

    bool multipassing = false;

    if(doZP)
    {
        glFlush();
        glDisableClientState(GL_VERTEX_ARRAY);
        if(cur.colormask) { cur.colormask = false; glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); }
        if(cur.depthmask) { cur.depthmask = false; glDepthMask(GL_FALSE); }
        collectlights();
        if(!cur.colormask) { cur.colormask = true; glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); }
        if(!cur.depthmask) { cur.depthmask = true; glDepthMask(GL_TRUE); }
        if(rhinoq) renderradiancehints();
        glFlush();
        glEnableClientState(GL_VERTEX_ARRAY);
        setupTMUs(cur);
        if(!multipassing) { multipassing = true; glDepthFunc(GL_LEQUAL); }
        cur.vbuf = 0;
        cur.texgendim = -1;

        for(vtxarray *va = visibleva; va; va = va->next)
        {
            if(!va->texs || va->occluded >= OCCLUDE_GEOM) continue;
            blends += va->blends;
            renderva(cur, va, RENDERPASS_GBUFFER);
        }
        if(geombatches.length()) renderbatches(cur, RENDERPASS_GBUFFER);
        for(vtxarray *va = visibleva; va; va = va->next)
        {
            if(!va->texs || va->occluded < OCCLUDE_GEOM) continue;
            else if((va->parent && va->parent->occluded >= OCCLUDE_BB) ||
                    (va->query && checkquery(va->query)))
            {
                va->occluded = OCCLUDE_BB;
                continue;
            }
            else
            {
                va->occluded = pvsoccluded(va->geommin, va->geommax) ? OCCLUDE_GEOM : OCCLUDE_NOTHING;
                if(va->occluded >= OCCLUDE_GEOM) continue;
            }

            blends += va->blends;
            renderva(cur, va, RENDERPASS_GBUFFER);
        }
        if(geombatches.length()) renderbatches(cur, RENDERPASS_GBUFFER);
    }

    if(!cur.colormask) { cur.colormask = true; glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); }
    if(!cur.depthmask) { cur.depthmask = true; glDepthMask(GL_TRUE); }

    if(blends)
    {
        if(!multipassing) { multipassing = true; glDepthFunc(GL_LEQUAL); }
        glDepthMask(GL_FALSE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        maskgbuffer("cng");

        GLOBALPARAM(blendlayer, (0.0f));
        cur.vbuf = 0;
        cur.texgendim = -1;
        for(vtxarray *va = visibleva; va; va = va->next)
        {
            if(!va->blends) continue;
            if(va->occluded >= OCCLUDE_GEOM) continue;
            if(va->curvfc==VFC_FOGGED) continue;
            renderva(cur, va, RENDERPASS_GBUFFER_BLEND);
        }
        if(geombatches.length()) renderbatches(cur, RENDERPASS_GBUFFER);

        maskgbuffer("cngd");
        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);
    }

    cleanupTMUs(cur);

    if(multipassing) glDepthFunc(GL_LESS);

    if(hasVBO)
    {
        glBindBuffer_(GL_ARRAY_BUFFER_ARB, 0);
        glBindBuffer_(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
    }
    glDisableClientState(GL_VERTEX_ARRAY);

    if(!doZP && !minimapping)
    {
        glFlush();
        if(cur.colormask) { cur.colormask = false; glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); }
        if(cur.depthmask) { cur.depthmask = false; glDepthMask(GL_FALSE); }
        collectlights();
        if(!cur.colormask) { cur.colormask = true; glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); }
        if(!cur.depthmask) { cur.depthmask = true; glDepthMask(GL_TRUE); }
        if(mainpass && rhinoq) renderradiancehints();
        glFlush();
    }
}

void renderrsmgeom()
{
    renderstate cur;
    setupTMUs(cur);
    resetbatches();

    glEnableClientState(GL_VERTEX_ARRAY);

    int blends = 0;
    for(vtxarray *va = shadowva; va; va = va->rnext)
    {
        if(!va->texs) continue;
        blends += va->blends;
        renderva(cur, va, RENDERPASS_RSM);
    }

    if(geombatches.length()) renderbatches(cur, RENDERPASS_RSM);

    bool multipassing = false;

    if(blends)
    {
        if(!multipassing) { multipassing = true; glDepthFunc(GL_LEQUAL); }
        glDepthMask(GL_FALSE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);

        GLOBALPARAM(blendlayer, (0.0f));
        cur.vbuf = 0;
        cur.texgendim = -1;
        for(vtxarray *va = shadowva; va; va = va->rnext)
        {
            if(!va->blends) continue;
            renderva(cur, va, RENDERPASS_RSM_BLEND);
        }
        if(geombatches.length()) renderbatches(cur, RENDERPASS_RSM);

        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);
    }

    cleanupTMUs(cur);

    if(multipassing) glDepthFunc(GL_LESS);

    if(hasVBO)
    {
        glBindBuffer_(GL_ARRAY_BUFFER_ARB, 0);
        glBindBuffer_(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
    }
    glDisableClientState(GL_VERTEX_ARRAY);
}

static vector<vtxarray *> alphavas;
static int alphabackvas = 0, alpharefractvas = 0;
float alphafrontsx1 = -1, alphafrontsx2 = 1, alphafrontsy1 = -1, alphafrontsy2 = -1,
      alphabacksx1 = -1, alphabacksx2 = 1, alphabacksy1 = -1, alphabacksy2 = -1,
      alpharefractsx1 = -1, alpharefractsx2 = 1, alpharefractsy1 = -1, alpharefractsy2 = 1;
uint alphatiles[LIGHTTILE_H];

int findalphavas()
{
    alphavas.setsize(0);
    alphafrontsx1 = alphafrontsy1 = alphabacksx1 = alphabacksy1 = alpharefractsx1 = alpharefractsy1 = 1;
    alphafrontsx2 = alphafrontsy2 = alphabacksx2 = alphabacksy2 = alpharefractsx2 = alpharefractsy2 = -1;
    alphabackvas = alpharefractvas = 0;
    memset(alphatiles, 0, sizeof(alphatiles));
    for(vtxarray *va = visibleva; va; va = va->next)
    {
        if(!va->alphabacktris && !va->alphafronttris && !va->refracttris) continue;
        if(va->occluded >= OCCLUDE_BB) continue;
        if(va->occluded >= OCCLUDE_GEOM && pvsoccluded(va->alphamin, va->alphamax)) continue;
        if(va->curvfc==VFC_FOGGED) continue;
        alphavas.add(va);
        float sx1 = -1, sx2 = 1, sy1 = -1, sy2 = 1;
        if(!calcbbscissor(va->alphamin, va->alphamax, sx1, sy1, sx2, sy2)) { sx1 = sy1 = -1; sx2 = sy2 = 1; }
        masktiles(alphatiles, sx1, sy1, sx2, sy2);
        alphafrontsx1 = min(alphafrontsx1, sx1);
        alphafrontsy1 = min(alphafrontsy1, sy1);
        alphafrontsx2 = max(alphafrontsx2, sx2);
        alphafrontsy2 = max(alphafrontsy2, sy2);
        if(va->alphabacktris) 
        {
            alphabackvas++;
            alphabacksx1 = min(alphabacksx1, sx1);
            alphabacksy1 = min(alphabacksy1, sy1);
            alphabacksx2 = max(alphabacksx2, sx2);
            alphabacksy2 = max(alphabacksy2, sy2);
        }
        if(va->refracttris)
        {
            alpharefractvas++;
            if(!calcbbscissor(va->refractmin, va->refractmax, sx1, sy1, sx2, sy2)) { sx1 = sy1 = -1; sx2 = sy2 = 1; }
            alpharefractsx1 = min(alpharefractsx1, sx1);
            alpharefractsy1 = min(alpharefractsy1, sy1);
            alpharefractsx2 = max(alpharefractsx2, sx2);
            alpharefractsy2 = max(alpharefractsy2, sy2);
        }
    }
    return (alpharefractvas ? 4 : 0) | (alphavas.length() ? 2 : 0) | (alphabackvas ? 1 : 0);
}

void renderrefractmask()
{
    glEnableClientState(GL_VERTEX_ARRAY);

    vtxarray *prev = NULL;
    loopv(alphavas)
    {
        vtxarray *va = alphavas[i];
        if(!va->refracttris) continue;

        if(!prev || va->vbuf != prev->vbuf)
        {
            if(hasVBO)
            {
                glBindBuffer_(GL_ARRAY_BUFFER_ARB, va->vbuf);
                glBindBuffer_(GL_ELEMENT_ARRAY_BUFFER_ARB, va->ebuf);
            }
            glVertexPointer(3, GL_FLOAT, VTXSIZE, va->vdata[0].pos.v);
        }

        drawvatris(va, 3*va->refracttris, &va->edata[3*(va->tris + va->blendtris + va->alphabacktris + va->alphafronttris)]);
        xtravertsva += 3*va->refracttris;
    }

    if(hasVBO)
    {
        glBindBuffer_(GL_ARRAY_BUFFER_ARB, 0);
        glBindBuffer_(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
    }
    glDisableClientState(GL_VERTEX_ARRAY);
}

void renderalphageom(int side)
{
    resetbatches();

    renderstate cur;
    cur.alphaing = side;
    cur.alphascale = -1;
    
    glEnableClientState(GL_VERTEX_ARRAY);

    setupTMUs(cur);

    if(side == 2)
    {
        loopv(alphavas) renderva(cur, alphavas[i], RENDERPASS_GBUFFER);
        if(geombatches.length()) renderbatches(cur, RENDERPASS_GBUFFER);
    }
    else
    {
        glCullFace(GL_FRONT);
        loopv(alphavas) if(alphavas[i]->alphabacktris) renderva(cur, alphavas[i], RENDERPASS_GBUFFER);
        if(geombatches.length()) renderbatches(cur, RENDERPASS_GBUFFER);
        glCullFace(GL_BACK);
    }
   
    cleanupTMUs(cur);

    if(hasVBO)
    {
        glBindBuffer_(GL_ARRAY_BUFFER_ARB, 0);
        glBindBuffer_(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
    }
    glDisableClientState(GL_VERTEX_ARRAY);
}

HVARP(explicitskycolour, 0, 0x800080, 0xFFFFFF);

bool renderexplicitsky(bool outline)
{
    vtxarray *prev = NULL;
    for(vtxarray *va = visibleva; va; va = va->next)
    {
        if(va->occluded >= OCCLUDE_BB || !va->sky) continue;
        if(!prev || va->vbuf != prev->vbuf)
        {
            if(!prev) 
            {
                glEnableClientState(GL_VERTEX_ARRAY);
                if(outline)
                {
                    notextureshader->set();
                    bvec color((explicitskycolour>>16)&0xFF, (explicitskycolour>>8)&0xFF, explicitskycolour&0xFF);
                    glColor3f(color.x*ldrscaleb, color.y*ldrscaleb, color.z*ldrscaleb);
                    glDepthMask(GL_FALSE);
                    enablepolygonoffset(GL_POLYGON_OFFSET_LINE);
                    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                }
                else
                {
                    nocolorshader->set();
                    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
                }
            }
            if(hasVBO)
            {
                glBindBuffer_(GL_ARRAY_BUFFER_ARB, va->vbuf);
                glBindBuffer_(GL_ELEMENT_ARRAY_BUFFER_ARB, va->skybuf);
            }
            glVertexPointer(3, GL_FLOAT, VTXSIZE, va->vdata[0].pos.v);
        }
        drawvatris(va, va->sky, va->skydata);
        xtraverts += va->sky/3;
        prev = va;
    }
    if(!prev) return false;
    if(outline)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        disablepolygonoffset(GL_POLYGON_OFFSET_LINE);
        glDepthMask(GL_TRUE);
    }
    else
    {
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    }
    glDisableClientState(GL_VERTEX_ARRAY);
    if(hasVBO)
    {
        glBindBuffer_(GL_ARRAY_BUFFER_ARB, 0);
        glBindBuffer_(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
    }
    return true;
}
 
