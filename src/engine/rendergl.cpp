// rendergl.cpp: core opengl rendering stuff

#include "engine.h"

#include "client_engine_additions.h" // INTENSITY
#include "targeting.h" // INTENSITY
#include "of_entities.h"

bool hasVBO = false, hasDRE = false, hasOQ = false, hasTR = false, hasT3D = false, hasFBO = false, hasAFBO = false, hasDS = false, hasTF = false, hasCBF = false, hasBE = false, hasBC = false, hasCM = false, hasNP2 = false, hasTC = false, hasMT = false, hasAF = false, hasMDA = false, hasGLSL = false, hasGM = false, hasNVFB = false, hasSGIDT = false, hasSGISH = false, hasDT = false, hasSH = false, hasNVPCF = false, hasPBO = false, hasFBB = false, hasUBO = false, hasBUE = false, hasDB = false, hasTG = false, hasT4 = false, hasTQ = false, hasPF = false, hasTRG = false, hasDBT = false, hasDC = false;
bool mesa = false, intel = false, ati = false, nvidia = false;

int hasstencil = 0;

VAR(renderpath, 1, 0, 0);
VAR(glversion, 1, 0, 0);
VAR(glslversion, 1, 0, 0);

// GL_ARB_vertex_buffer_object, GL_ARB_pixel_buffer_object
PFNGLGENBUFFERSARBPROC       glGenBuffers_       = NULL;
PFNGLBINDBUFFERARBPROC       glBindBuffer_       = NULL;
PFNGLMAPBUFFERARBPROC        glMapBuffer_        = NULL;
PFNGLUNMAPBUFFERARBPROC      glUnmapBuffer_      = NULL;
PFNGLBUFFERDATAARBPROC       glBufferData_       = NULL;
PFNGLBUFFERSUBDATAARBPROC    glBufferSubData_    = NULL;
PFNGLDELETEBUFFERSARBPROC    glDeleteBuffers_    = NULL;
PFNGLGETBUFFERSUBDATAARBPROC glGetBufferSubData_ = NULL;

// GL_ARB_multitexture
PFNGLACTIVETEXTUREARBPROC       glActiveTexture_       = NULL;
PFNGLCLIENTACTIVETEXTUREARBPROC glClientActiveTexture_ = NULL;
PFNGLMULTITEXCOORD2FARBPROC     glMultiTexCoord2f_     = NULL;
PFNGLMULTITEXCOORD3FARBPROC     glMultiTexCoord3f_     = NULL;
PFNGLMULTITEXCOORD4FARBPROC     glMultiTexCoord4f_     = NULL;

// GL_ARB_occlusion_query
PFNGLGENQUERIESARBPROC        glGenQueries_        = NULL;
PFNGLDELETEQUERIESARBPROC     glDeleteQueries_     = NULL;
PFNGLBEGINQUERYARBPROC        glBeginQuery_        = NULL;
PFNGLENDQUERYARBPROC          glEndQuery_          = NULL;
PFNGLGETQUERYIVARBPROC        glGetQueryiv_        = NULL;
PFNGLGETQUERYOBJECTIVARBPROC  glGetQueryObjectiv_  = NULL;
PFNGLGETQUERYOBJECTUIVARBPROC glGetQueryObjectuiv_ = NULL;

// GL_EXT_timer_query
PFNGLGETQUERYOBJECTI64VEXTPROC glGetQueryObjecti64v_  = NULL;
PFNGLGETQUERYOBJECTUI64VEXTPROC glGetQueryObjectui64v_ = NULL;

// GL_EXT_framebuffer_object
PFNGLBINDRENDERBUFFEREXTPROC        glBindRenderbuffer_        = NULL;
PFNGLDELETERENDERBUFFERSEXTPROC     glDeleteRenderbuffers_     = NULL;
PFNGLGENFRAMEBUFFERSEXTPROC         glGenRenderbuffers_        = NULL;
PFNGLRENDERBUFFERSTORAGEEXTPROC     glRenderbufferStorage_     = NULL;
PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC  glCheckFramebufferStatus_  = NULL;
PFNGLBINDFRAMEBUFFEREXTPROC         glBindFramebuffer_         = NULL;
PFNGLDELETEFRAMEBUFFERSEXTPROC      glDeleteFramebuffers_      = NULL;
PFNGLGENFRAMEBUFFERSEXTPROC         glGenFramebuffers_         = NULL;
PFNGLFRAMEBUFFERTEXTURE1DEXTPROC    glFramebufferTexture1D_    = NULL;
PFNGLFRAMEBUFFERTEXTURE2DEXTPROC    glFramebufferTexture2D_    = NULL;
PFNGLFRAMEBUFFERTEXTURE3DEXTPROC    glFramebufferTexture3D_    = NULL;
PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC glFramebufferRenderbuffer_ = NULL;
PFNGLGENERATEMIPMAPEXTPROC          glGenerateMipmap_          = NULL;

// GL_ARB_draw_buffers
PFNGLDRAWBUFFERSARBPROC glDrawBuffers_ = NULL;

// GL_EXT_framebuffer_blit
PFNGLBLITFRAMEBUFFEREXTPROC         glBlitFramebuffer_         = NULL;

// GL_ARB_shading_language_100, GL_ARB_shader_objects, GL_ARB_fragment_shader, GL_ARB_vertex_shader
PFNGLCREATEPROGRAMOBJECTARBPROC       glCreateProgramObject_      = NULL;
PFNGLDELETEOBJECTARBPROC              glDeleteObject_             = NULL;
PFNGLUSEPROGRAMOBJECTARBPROC          glUseProgramObject_         = NULL; 
PFNGLCREATESHADEROBJECTARBPROC        glCreateShaderObject_       = NULL;
PFNGLSHADERSOURCEARBPROC              glShaderSource_             = NULL;
PFNGLCOMPILESHADERARBPROC             glCompileShader_            = NULL;
PFNGLGETOBJECTPARAMETERIVARBPROC      glGetObjectParameteriv_     = NULL;
PFNGLATTACHOBJECTARBPROC              glAttachObject_             = NULL;
PFNGLGETINFOLOGARBPROC                glGetInfoLog_               = NULL;
PFNGLLINKPROGRAMARBPROC               glLinkProgram_              = NULL;
PFNGLGETUNIFORMLOCATIONARBPROC        glGetUniformLocation_       = NULL;
PFNGLUNIFORM1FARBPROC                 glUniform1f_                = NULL;
PFNGLUNIFORM2FARBPROC                 glUniform2f_                = NULL;
PFNGLUNIFORM3FARBPROC                 glUniform3f_                = NULL;
PFNGLUNIFORM4FARBPROC                 glUniform4f_                = NULL;
PFNGLUNIFORM1FVARBPROC                glUniform1fv_               = NULL;
PFNGLUNIFORM2FVARBPROC                glUniform2fv_               = NULL;
PFNGLUNIFORM3FVARBPROC                glUniform3fv_               = NULL;
PFNGLUNIFORM4FVARBPROC                glUniform4fv_               = NULL;
PFNGLUNIFORM1IARBPROC                 glUniform1i_                = NULL;
PFNGLUNIFORMMATRIX2FVARBPROC          glUniformMatrix2fv_         = NULL;
PFNGLUNIFORMMATRIX3FVARBPROC          glUniformMatrix3fv_         = NULL;
PFNGLUNIFORMMATRIX4FVARBPROC          glUniformMatrix4fv_         = NULL;
PFNGLBINDATTRIBLOCATIONARBPROC        glBindAttribLocation_       = NULL;
PFNGLGETACTIVEUNIFORMARBPROC          glGetActiveUniform_         = NULL;
PFNGLENABLEVERTEXATTRIBARRAYARBPROC   glEnableVertexAttribArray_  = NULL;
PFNGLDISABLEVERTEXATTRIBARRAYARBPROC  glDisableVertexAttribArray_ = NULL;
PFNGLVERTEXATTRIBPOINTERARBPROC       glVertexAttribPointer_      = NULL;

// GL_EXT_draw_range_elements
PFNGLDRAWRANGEELEMENTSEXTPROC glDrawRangeElements_ = NULL;

// GL_EXT_blend_minmax
PFNGLBLENDEQUATIONEXTPROC glBlendEquation_ = NULL;

// GL_EXT_blend_color
PFNGLBLENDCOLOREXTPROC glBlendColor_ = NULL;

// GL_EXT_multi_draw_arrays
PFNGLMULTIDRAWARRAYSEXTPROC   glMultiDrawArrays_   = NULL;
PFNGLMULTIDRAWELEMENTSEXTPROC glMultiDrawElements_ = NULL;

// GL_ARB_texture_compression
PFNGLCOMPRESSEDTEXIMAGE3DARBPROC    glCompressedTexImage3D_    = NULL;
PFNGLCOMPRESSEDTEXIMAGE2DARBPROC    glCompressedTexImage2D_    = NULL;
PFNGLCOMPRESSEDTEXIMAGE1DARBPROC    glCompressedTexImage1D_    = NULL;
PFNGLCOMPRESSEDTEXSUBIMAGE3DARBPROC glCompressedTexSubImage3D_ = NULL;
PFNGLCOMPRESSEDTEXSUBIMAGE2DARBPROC glCompressedTexSubImage2D_ = NULL;
PFNGLCOMPRESSEDTEXSUBIMAGE1DARBPROC glCompressedTexSubImage1D_ = NULL;
PFNGLGETCOMPRESSEDTEXIMAGEARBPROC   glGetCompressedTexImage_   = NULL;

// GL_ARB_uniform_buffer_object
PFNGLGETUNIFORMINDICESPROC       glGetUniformIndices_       = NULL;
PFNGLGETACTIVEUNIFORMSIVPROC     glGetActiveUniformsiv_     = NULL;
PFNGLGETUNIFORMBLOCKINDEXPROC    glGetUniformBlockIndex_    = NULL;
PFNGLGETACTIVEUNIFORMBLOCKIVPROC glGetActiveUniformBlockiv_ = NULL;
PFNGLUNIFORMBLOCKBINDINGPROC     glUniformBlockBinding_     = NULL;
PFNGLBINDBUFFERBASEPROC          glBindBufferBase_          = NULL;
PFNGLBINDBUFFERRANGEPROC         glBindBufferRange_         = NULL;

// GL_EXT_bindable_uniform
PFNGLUNIFORMBUFFEREXTPROC        glUniformBuffer_        = NULL;
PFNGLGETUNIFORMBUFFERSIZEEXTPROC glGetUniformBufferSize_ = NULL;
PFNGLGETUNIFORMOFFSETEXTPROC     glGetUniformOffset_     = NULL;

// GL_EXT_depth_bounds_test
PFNGLDEPTHBOUNDSEXTPROC glDepthBounds_ = NULL;

// GL_ARB_color_buffer_float
PFNGLCLAMPCOLORARBPROC glClampColor_ = NULL;

// GL_EXT_texture3D
PFNGLTEXIMAGE3DEXTPROC        glTexImage3D_ = NULL;
PFNGLTEXSUBIMAGE3DEXTPROC     glTexSubImage3D_ = NULL;
PFNGLCOPYTEXSUBIMAGE3DEXTPROC glCopyTexSubImage3D_ = NULL;

void *getprocaddress(const char *name)
{
    return SDL_GL_GetProcAddress(name);
}

void glerror(const char *file, int line, GLenum error)
{
    const char *desc = "unknown";
    switch(error)
    {
    case GL_NO_ERROR: desc = "no error"; break;
    case GL_INVALID_ENUM: desc = "invalid enum"; break;
    case GL_INVALID_VALUE: desc = "invalid value"; break;
    case GL_INVALID_OPERATION: desc = "invalid operation"; break;
    case GL_STACK_OVERFLOW: desc = "stack overflow"; break;
    case GL_STACK_UNDERFLOW: desc = "stack underflow"; break;
    case GL_OUT_OF_MEMORY: desc = "out of memory"; break;
    }
    printf("GL error: %s:%d: %s (%x)\n", file, line, desc, error);
}

VAR(ati_oq_bug, 0, 0, 1);
VAR(ati_minmax_bug, 0, 0, 1);
VAR(ati_cubemap_bug, 0, 0, 1);
VAR(ati_ubo_bug, 0, 0, 1);
VAR(sdl_backingstore_bug, -1, 0, 1);
VAR(usetexrect, 1, 0, 0);
VAR(useubo, 1, 0, 0);
VAR(usebue, 1, 0, 0);
VAR(usetexgather, 1, 0, 0);

#if 0
static bool checkseries(const char *s, int low, int high)
{
    while(*s && !isdigit(*s)) ++s;
    if(!*s) return false;
    int n = 0;
    while(isdigit(*s)) n = n*10 + (*s++ - '0');    
    return n >= low && n < high;
}
#endif

VAR(dbgexts, 0, 0, 1);

bool hasext(const char *exts, const char *ext)
{
    int len = strlen(ext);
    if(len) for(const char *cur = exts; (cur = strstr(cur, ext)); cur += len)
    {
        if((cur == exts || cur[-1] == ' ') && (cur[len] == ' ' || !cur[len])) return true;
    }
    return false;
}

void gl_checkextensions()
{
    const char *vendor = (const char *)glGetString(GL_VENDOR);
    const char *exts = (const char *)glGetString(GL_EXTENSIONS);
    const char *renderer = (const char *)glGetString(GL_RENDERER);
    const char *version = (const char *)glGetString(GL_VERSION);
    conoutf(CON_INIT, "Renderer: %s (%s)", renderer, vendor);
    conoutf(CON_INIT, "Driver: %s", version);

#ifdef __APPLE__
    extern int mac_osversion();
    int osversion = mac_osversion();  /* 0x1050 = 10.5 (Leopard) */
    sdl_backingstore_bug = -1;
#endif

    if(strstr(renderer, "Mesa") || strstr(version, "Mesa"))
    {
        mesa = true;
        if(strstr(renderer, "Intel")) intel = true;
    }
    else if(strstr(vendor, "NVIDIA"))
        nvidia = true;
    else if(strstr(vendor, "ATI") || strstr(vendor, "Advanced Micro Devices"))
        ati = true;
    else if(strstr(vendor, "Intel"))
        intel = true;

    uint glmajorversion, glminorversion;
    if(sscanf(version, " %u.%u", &glmajorversion, &glminorversion) != 2) glversion = 100;
    else glversion = glmajorversion*100 + glminorversion*10;

    //extern int shaderprecision;
    // default to low precision shaders on certain cards, can be overridden with -f3
    // char *weakcards[] = { "GeForce FX", "Quadro FX", "6200", "9500", "9550", "9600", "9700", "9800", "X300", "X600", "FireGL", "Intel", "Chrome", NULL } 
    // if(shaderprecision==2) for(char **wc = weakcards; *wc; wc++) if(strstr(renderer, *wc)) shaderprecision = 1;

    GLint val;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &val);
    hwtexsize = val;
    if(hwtexsize < 4096)
        fatal("Large texture support is required!");

    if(hasext(exts, "GL_ARB_multitexture"))
    {
        glActiveTexture_       = (PFNGLACTIVETEXTUREARBPROC)      getprocaddress("glActiveTextureARB");
        glClientActiveTexture_ = (PFNGLCLIENTACTIVETEXTUREARBPROC)getprocaddress("glClientActiveTextureARB");
        glMultiTexCoord2f_     = (PFNGLMULTITEXCOORD2FARBPROC)    getprocaddress("glMultiTexCoord2fARB");
        glMultiTexCoord3f_     = (PFNGLMULTITEXCOORD3FARBPROC)    getprocaddress("glMultiTexCoord3fARB");
        glMultiTexCoord4f_     = (PFNGLMULTITEXCOORD4FARBPROC)    getprocaddress("glMultiTexCoord4fARB");
        hasMT = true;
        if(dbgexts) conoutf(CON_INIT, "Using GL_ARB_multitexture extension.");
    }
    //else conoutf(CON_WARN, "WARNING: No multitexture extension!");
    else fatal("Multitexture support is required!");

    if(hasext(exts, "GL_ARB_vertex_buffer_object")) 
    {
        hasVBO = true;
        if(dbgexts) conoutf(CON_INIT, "Using GL_ARB_vertex_buffer_object extension.");
    }
    else conoutf(CON_WARN, "WARNING: No vertex_buffer_object extension! (geometry heavy maps will be SLOW)");
#ifdef __APPLE__
    /* VBOs over 256KB seem to destroy performance on 10.5, but not in 10.6 */
    extern int maxvbosize;
    if(osversion < 0x1060) maxvbosize = min(maxvbosize, 8192);  
#endif

    if(hasext(exts, "GL_ARB_pixel_buffer_object"))
    {
        hasPBO = true;
        if(dbgexts) conoutf(CON_INIT, "Using GL_ARB_pixel_buffer_object extension.");
    }

    if(hasVBO || hasPBO)
    {
        glGenBuffers_       = (PFNGLGENBUFFERSARBPROC)      getprocaddress("glGenBuffersARB");
        glBindBuffer_       = (PFNGLBINDBUFFERARBPROC)      getprocaddress("glBindBufferARB");
        glMapBuffer_        = (PFNGLMAPBUFFERARBPROC)       getprocaddress("glMapBufferARB");
        glUnmapBuffer_      = (PFNGLUNMAPBUFFERARBPROC)     getprocaddress("glUnmapBufferARB");
        glBufferData_       = (PFNGLBUFFERDATAARBPROC)      getprocaddress("glBufferDataARB");
        glBufferSubData_    = (PFNGLBUFFERSUBDATAARBPROC)   getprocaddress("glBufferSubDataARB");
        glDeleteBuffers_    = (PFNGLDELETEBUFFERSARBPROC)   getprocaddress("glDeleteBuffersARB");
        glGetBufferSubData_ = (PFNGLGETBUFFERSUBDATAARBPROC)getprocaddress("glGetBufferSubDataARB");
    }

    if(hasext(exts, "GL_EXT_draw_range_elements"))
    {
        glDrawRangeElements_ = (PFNGLDRAWRANGEELEMENTSEXTPROC)getprocaddress("glDrawRangeElementsEXT");
        hasDRE = true;
        if(dbgexts) conoutf(CON_INIT, "Using GL_EXT_draw_range_elements extension.");
    }

    if(hasext(exts, "GL_EXT_multi_draw_arrays"))
    {
        glMultiDrawArrays_   = (PFNGLMULTIDRAWARRAYSEXTPROC)  getprocaddress("glMultiDrawArraysEXT");
        glMultiDrawElements_ = (PFNGLMULTIDRAWELEMENTSEXTPROC)getprocaddress("glMultiDrawElementsEXT");
        hasMDA = true;
        if(dbgexts) conoutf(CON_INIT, "Using GL_EXT_multi_draw_arrays extension.");
    }

#ifdef __APPLE__
    // floating point FBOs not fully supported until 10.5
    if(osversion>=0x1050)
#endif
    if(hasext(exts, "GL_ARB_texture_float") || hasext(exts, "GL_ATI_texture_float"))
    {
        hasTF = true;
        if(dbgexts) conoutf(CON_INIT, "Using GL_ARB_texture_float extension.");
    }

    if(hasext(exts, "GL_NV_float_buffer")) 
    {
        hasNVFB = true;
        if(dbgexts) conoutf(CON_INIT, "Using GL_NV_float_buffer extension.");
    }

    if(hasext(exts, "GL_ARB_texture_rg"))
    {
        hasTRG = true;
        if(dbgexts) conoutf(CON_INIT, "Using GL_ARB_texture_rg extension.");
    }

    if(hasext(exts, "GL_EXT_framebuffer_object"))
    {
        glBindRenderbuffer_        = (PFNGLBINDRENDERBUFFEREXTPROC)       getprocaddress("glBindRenderbufferEXT");
        glDeleteRenderbuffers_     = (PFNGLDELETERENDERBUFFERSEXTPROC)    getprocaddress("glDeleteRenderbuffersEXT");
        glGenRenderbuffers_        = (PFNGLGENFRAMEBUFFERSEXTPROC)        getprocaddress("glGenRenderbuffersEXT");
        glRenderbufferStorage_     = (PFNGLRENDERBUFFERSTORAGEEXTPROC)    getprocaddress("glRenderbufferStorageEXT");
        glCheckFramebufferStatus_  = (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC) getprocaddress("glCheckFramebufferStatusEXT");
        glBindFramebuffer_         = (PFNGLBINDFRAMEBUFFEREXTPROC)        getprocaddress("glBindFramebufferEXT");
        glDeleteFramebuffers_      = (PFNGLDELETEFRAMEBUFFERSEXTPROC)     getprocaddress("glDeleteFramebuffersEXT");
        glGenFramebuffers_         = (PFNGLGENFRAMEBUFFERSEXTPROC)        getprocaddress("glGenFramebuffersEXT");
        glFramebufferTexture1D_    = (PFNGLFRAMEBUFFERTEXTURE1DEXTPROC)   getprocaddress("glFramebufferTexture1DEXT");
        glFramebufferTexture2D_    = (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC)   getprocaddress("glFramebufferTexture2DEXT");
        glFramebufferTexture3D_    = (PFNGLFRAMEBUFFERTEXTURE3DEXTPROC)   getprocaddress("glFramebufferTexture3DEXT");
        glFramebufferRenderbuffer_ = (PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC)getprocaddress("glFramebufferRenderbufferEXT");
        glGenerateMipmap_          = (PFNGLGENERATEMIPMAPEXTPROC)         getprocaddress("glGenerateMipmapEXT");
        hasFBO = true;
        if(dbgexts) conoutf(CON_INIT, "Using GL_EXT_framebuffer_object extension.");

        if(hasext(exts, "GL_ARB_framebuffer_object"))
        {
            hasAFBO = true;
            if(dbgexts) conoutf(CON_INIT, "Using GL_ARB_framebuffer_object extension.");
        } 
        if(hasext(exts, "GL_EXT_framebuffer_blit"))
        {
            glBlitFramebuffer_     = (PFNGLBLITFRAMEBUFFEREXTPROC)        getprocaddress("glBlitFramebufferEXT");
            hasFBB = true;
            if(dbgexts) conoutf(CON_INIT, "Using GL_EXT_framebuffer_blit extension.");
        }
    }
    //else conoutf(CON_WARN, "WARNING: No framebuffer object support. (reflective water may be slow)");
    else fatal("Framebuffer object support is required!");

    if(hasext(exts, "GL_ARB_draw_buffers"))
    {
        glDrawBuffers_ = (PFNGLDRAWBUFFERSARBPROC)getprocaddress("glDrawBuffersARB");
        hasDB = true;
        if(dbgexts) conoutf(CON_INIT, "Using GL_ARB_draw_buffers extension.");
        GLint maxbufs = 0;
        glGetIntegerv(GL_MAX_DRAW_BUFFERS_ARB, &maxbufs);
        if(maxbufs < 3) fatal("Hardware does not support at least 3 draw buffers.");
    }
    else fatal("Draw buffers support is required!");

    if(hasext(exts, "GL_ARB_color_buffer_float"))
    {
        glClampColor_ = (PFNGLCLAMPCOLORARBPROC)getprocaddress("glClampColorARB");
        hasCBF = true;
        if(dbgexts) conoutf(CON_INIT, "Using GL_ARB_color_buffer_float extension.");
    }

#ifdef __APPLE__
    // Intel HD3000 broke occlusion queries - either causing software fallback, or returning wrong results
    if(!intel)
#endif	   
    if(hasext(exts, "GL_ARB_occlusion_query"))
    {
        GLint bits;
        glGetQueryiv_ = (PFNGLGETQUERYIVARBPROC)getprocaddress("glGetQueryivARB");
        glGetQueryiv_(GL_SAMPLES_PASSED_ARB, GL_QUERY_COUNTER_BITS_ARB, &bits);
        if(bits)
        {
            glGenQueries_ =        (PFNGLGENQUERIESARBPROC)       getprocaddress("glGenQueriesARB");
            glDeleteQueries_ =     (PFNGLDELETEQUERIESARBPROC)    getprocaddress("glDeleteQueriesARB");
            glBeginQuery_ =        (PFNGLBEGINQUERYARBPROC)       getprocaddress("glBeginQueryARB");
            glEndQuery_ =          (PFNGLENDQUERYARBPROC)         getprocaddress("glEndQueryARB");
            glGetQueryObjectiv_ =  (PFNGLGETQUERYOBJECTIVARBPROC) getprocaddress("glGetQueryObjectivARB");
            glGetQueryObjectuiv_ = (PFNGLGETQUERYOBJECTUIVARBPROC)getprocaddress("glGetQueryObjectuivARB");
            hasOQ = true;
            if(dbgexts) conoutf(CON_INIT, "Using GL_ARB_occlusion_query extension.");
#if defined(__APPLE__) && SDL_BYTEORDER == SDL_BIG_ENDIAN
            if(ati && (osversion<0x1050)) ati_oq_bug = 1;
#endif
            //if(ati_oq_bug) conoutf(CON_WARN, "WARNING: Using ATI occlusion query bug workaround. (use \"/ati_oq_bug 0\" to disable if unnecessary)");
        }
    }
    if(!hasOQ)
    {
        conoutf(CON_WARN, "WARNING: No occlusion query support! (large maps may be SLOW)");
        extern int vacubesize;
        vacubesize = 64;
    }

    if(hasext(exts, "GL_EXT_timer_query"))
    {
        glGetQueryiv_ =          (PFNGLGETQUERYIVARBPROC)       getprocaddress("glGetQueryivARB");
        glGenQueries_ =          (PFNGLGENQUERIESARBPROC)       getprocaddress("glGenQueriesARB");
        glDeleteQueries_ =       (PFNGLDELETEQUERIESARBPROC)    getprocaddress("glDeleteQueriesARB");
        glBeginQuery_ =          (PFNGLBEGINQUERYARBPROC)       getprocaddress("glBeginQueryARB");
        glEndQuery_ =            (PFNGLENDQUERYARBPROC)         getprocaddress("glEndQueryARB");
        glGetQueryObjectiv_ =    (PFNGLGETQUERYOBJECTIVARBPROC) getprocaddress("glGetQueryObjectivARB");
        glGetQueryObjectuiv_ =   (PFNGLGETQUERYOBJECTUIVARBPROC)getprocaddress("glGetQueryObjectuivARB");
        glGetQueryObjecti64v_ =  (PFNGLGETQUERYOBJECTI64VEXTPROC)  getprocaddress("glGetQueryObjecti64vEXT");
        glGetQueryObjectui64v_ = (PFNGLGETQUERYOBJECTUI64VEXTPROC) getprocaddress("glGetQueryObjectui64vEXT");
        hasTQ = true;
        if(dbgexts) conoutf(CON_INIT, "Using GL_EXT_timer_query extension.");
    }

    extern int gdepthstencil, glineardepth, lighttilebatch, batchsunlight;
    if(ati)
    {
        //conoutf(CON_WARN, "WARNING: ATI cards may show garbage in skybox. (use \"/ati_skybox_bug 1\" to fix)");
        if(!hasext(exts, "GL_ARB_gpu_shader5")) gdepthstencil = 0; // some older ATI GPUs do not support reading from depth-stencil textures, so only use depth-stencil renderbuffer for now
    }
    else if(nvidia)
    {
    }
    else if(intel)
    {
#ifdef WIN32
        gdepthstencil = 0; // workaround for buggy stencil on windows ivy bridge driver
#endif
        glineardepth = 1; // causes massive slowdown in windows driver (and sometimes in linux driver) if not using linear depth
        lighttilebatch = 4;
        if(mesa) batchsunlight = 0; // causes massive slowdown in linux driver
    }
    else
    {
        // silence warnings
        (void)mesa;
    }

    if(hasext(exts, "GL_ARB_shading_language_100") && hasext(exts, "GL_ARB_shader_objects") && hasext(exts, "GL_ARB_vertex_shader") && hasext(exts, "GL_ARB_fragment_shader"))
    {
        glCreateProgramObject_ =        (PFNGLCREATEPROGRAMOBJECTARBPROC)     getprocaddress("glCreateProgramObjectARB");
        glDeleteObject_ =               (PFNGLDELETEOBJECTARBPROC)            getprocaddress("glDeleteObjectARB");
        glUseProgramObject_ =           (PFNGLUSEPROGRAMOBJECTARBPROC)        getprocaddress("glUseProgramObjectARB");
        glCreateShaderObject_ =         (PFNGLCREATESHADEROBJECTARBPROC)      getprocaddress("glCreateShaderObjectARB");
        glShaderSource_ =               (PFNGLSHADERSOURCEARBPROC)            getprocaddress("glShaderSourceARB");
        glCompileShader_ =              (PFNGLCOMPILESHADERARBPROC)           getprocaddress("glCompileShaderARB");
        glGetObjectParameteriv_ =       (PFNGLGETOBJECTPARAMETERIVARBPROC)    getprocaddress("glGetObjectParameterivARB");
        glAttachObject_ =               (PFNGLATTACHOBJECTARBPROC)            getprocaddress("glAttachObjectARB");
        glGetInfoLog_ =                 (PFNGLGETINFOLOGARBPROC)              getprocaddress("glGetInfoLogARB");
        glLinkProgram_ =                (PFNGLLINKPROGRAMARBPROC)             getprocaddress("glLinkProgramARB");
        glGetUniformLocation_ =         (PFNGLGETUNIFORMLOCATIONARBPROC)      getprocaddress("glGetUniformLocationARB");
        glUniform1f_ =                  (PFNGLUNIFORM1FARBPROC)               getprocaddress("glUniform1fARB");
        glUniform2f_ =                  (PFNGLUNIFORM2FARBPROC)               getprocaddress("glUniform2fARB");
        glUniform3f_ =                  (PFNGLUNIFORM3FARBPROC)               getprocaddress("glUniform3fARB");
        glUniform4f_ =                  (PFNGLUNIFORM4FARBPROC)               getprocaddress("glUniform4fARB");
        glUniform1fv_ =                 (PFNGLUNIFORM1FVARBPROC)              getprocaddress("glUniform1fvARB");
        glUniform2fv_ =                 (PFNGLUNIFORM2FVARBPROC)              getprocaddress("glUniform2fvARB");
        glUniform3fv_ =                 (PFNGLUNIFORM3FVARBPROC)              getprocaddress("glUniform3fvARB");
        glUniform4fv_ =                 (PFNGLUNIFORM4FVARBPROC)              getprocaddress("glUniform4fvARB");
        glUniform1i_ =                  (PFNGLUNIFORM1IARBPROC)               getprocaddress("glUniform1iARB");
        glUniformMatrix2fv_ =           (PFNGLUNIFORMMATRIX2FVARBPROC)        getprocaddress("glUniformMatrix2fvARB");
        glUniformMatrix3fv_ =           (PFNGLUNIFORMMATRIX3FVARBPROC)        getprocaddress("glUniformMatrix3fvARB");          
        glUniformMatrix4fv_ =           (PFNGLUNIFORMMATRIX4FVARBPROC)        getprocaddress("glUniformMatrix4fvARB");          
        glBindAttribLocation_ =         (PFNGLBINDATTRIBLOCATIONARBPROC)      getprocaddress("glBindAttribLocationARB");
        glGetActiveUniform_ =           (PFNGLGETACTIVEUNIFORMARBPROC)        getprocaddress("glGetActiveUniformARB");
        glEnableVertexAttribArray_ =    (PFNGLENABLEVERTEXATTRIBARRAYARBPROC) getprocaddress("glEnableVertexAttribArrayARB");
        glDisableVertexAttribArray_ =   (PFNGLDISABLEVERTEXATTRIBARRAYARBPROC)getprocaddress("glDisableVertexAttribArrayARB");
        glVertexAttribPointer_ =        (PFNGLVERTEXATTRIBPOINTERARBPROC)     getprocaddress("glVertexAttribPointerARB");

        extern bool checkglslsupport();
        if(checkglslsupport()) hasGLSL = true;
        
        const char *str = (const char *)glGetString(GL_SHADING_LANGUAGE_VERSION_ARB);
        conoutf(CON_INIT, "GLSL: %s", str ? str : "unknown");

        uint majorversion, minorversion;
        if(!str || sscanf(str, " %u.%u", &majorversion, &minorversion) != 2) glslversion = 100;
        else glslversion = majorversion*100 + minorversion; 
    }
    if(!hasGLSL || glslversion < 100) fatal("GLSL support is required!");
 
    if(hasext(exts, "GL_ARB_uniform_buffer_object"))
    {
        glGetUniformIndices_       = (PFNGLGETUNIFORMINDICESPROC)      getprocaddress("glGetUniformIndices");
        glGetActiveUniformsiv_     = (PFNGLGETACTIVEUNIFORMSIVPROC)    getprocaddress("glGetActiveUniformsiv");
        glGetUniformBlockIndex_    = (PFNGLGETUNIFORMBLOCKINDEXPROC)   getprocaddress("glGetUniformBlockIndex");
        glGetActiveUniformBlockiv_ = (PFNGLGETACTIVEUNIFORMBLOCKIVPROC)getprocaddress("glGetActiveUniformBlockiv");
        glUniformBlockBinding_     = (PFNGLUNIFORMBLOCKBINDINGPROC)    getprocaddress("glUniformBlockBinding");
        glBindBufferBase_          = (PFNGLBINDBUFFERBASEPROC)         getprocaddress("glBindBufferBase");
        glBindBufferRange_         = (PFNGLBINDBUFFERRANGEPROC)        getprocaddress("glBindBufferRange");

        useubo = 1;
        hasUBO = true;
        if(ati) ati_ubo_bug = 1;
        if(dbgexts) conoutf(CON_INIT, "Using GL_ARB_uniform_buffer_object extension.");
    }
    else if(hasext(exts, "GL_EXT_bindable_uniform"))
    {
        glUniformBuffer_        = (PFNGLUNIFORMBUFFEREXTPROC)       getprocaddress("glUniformBufferEXT");
        glGetUniformBufferSize_ = (PFNGLGETUNIFORMBUFFERSIZEEXTPROC)getprocaddress("glGetUniformBufferSizeEXT");
        glGetUniformOffset_     = (PFNGLGETUNIFORMOFFSETEXTPROC)    getprocaddress("glGetUniformOffsetEXT");

        usebue = 1;
        hasBUE = true;
        if(ati) ati_ubo_bug = 1;
        if(dbgexts) conoutf(CON_INIT, "Using GL_EXT_bindable_uniform extension.");
    }

    if(hasext(exts, "GL_ARB_texture_rectangle"))
    {
        usetexrect = 1;
        hasTR = true;
        if(dbgexts) conoutf(CON_INIT, "Using GL_ARB_texture_rectangle extension.");
    }
    //else if(hasMT) conoutf(CON_WARN, "WARNING: No texture rectangle support. (no full screen shaders)");
    else fatal("Texture rectangle support is required!");

    if(hasext(exts, "GL_EXT_texture3D"))
    {
        glTexImage3D_ =         (PFNGLTEXIMAGE3DEXTPROC)       getprocaddress("glTexImage3DEXT");
        glTexSubImage3D_ =      (PFNGLTEXSUBIMAGE3DEXTPROC)    getprocaddress("glTexSubImage3DEXT");
        glCopyTexSubImage3D_ =  (PFNGLCOPYTEXSUBIMAGE3DEXTPROC)getprocaddress("glCopyTexSubImage3DEXT");
        hasT3D = true;
        if(dbgexts) conoutf(CON_INIT, "Using GL_EXT_texture3D extension.");
    }
    else if(glversion >= 120)
    {
        glTexImage3D_ =         (PFNGLTEXIMAGE3DEXTPROC)       getprocaddress("glTexImage3D");
        glTexSubImage3D_ =      (PFNGLTEXSUBIMAGE3DEXTPROC)    getprocaddress("glTexSubImage3D");
        glCopyTexSubImage3D_ =  (PFNGLCOPYTEXSUBIMAGE3DEXTPROC)getprocaddress("glCopyTexSubImage3D");
        hasT3D = true;
        if(dbgexts) conoutf(CON_INIT, "Using OpenGL 1.2 3D texture support.");
    }
    else fatal("3D texture support is required!");

    if(hasext(exts, "GL_EXT_packed_depth_stencil") || hasext(exts, "GL_NV_packed_depth_stencil"))
    {
        hasDS = true;
        if(dbgexts) conoutf(CON_INIT, "Using GL_EXT_packed_depth_stencil extension.");
    }

    if(hasext(exts, "GL_EXT_packed_float"))
    {
        hasPF = true;
        if(dbgexts) conoutf(CON_INIT, "Using GL_EXT_packed_float extension.");
    }

    if(hasext(exts, "GL_EXT_blend_minmax"))
    {
        glBlendEquation_ = (PFNGLBLENDEQUATIONEXTPROC) getprocaddress("glBlendEquationEXT");
        hasBE = true;
        if(ati) ati_minmax_bug = 1;
        if(dbgexts) conoutf(CON_INIT, "Using GL_EXT_blend_minmax extension.");
    }

    if(hasext(exts, "GL_EXT_blend_color"))
    {
        glBlendColor_ = (PFNGLBLENDCOLOREXTPROC) getprocaddress("glBlendColorEXT");
        hasBC = true;
        if(dbgexts) conoutf(CON_INIT, "Using GL_EXT_blend_color extension.");
    }

    if(hasext(exts, "GL_ARB_texture_cube_map"))
    {
        GLint val;
        glGetIntegerv(GL_MAX_CUBE_MAP_TEXTURE_SIZE_ARB, &val);
        hwcubetexsize = val;
        hasCM = true;
        // On Catalyst 10.2, issuing an occlusion query on the first draw using a given cubemap texture causes a nasty crash
        if(ati) ati_cubemap_bug = 1;
        if(dbgexts) conoutf(CON_INIT, "Using GL_ARB_texture_cube_map extension.");
    }
    else conoutf(CON_WARN, "WARNING: No cube map texture support. (no reflective glass)");

    if(hasext(exts, "GL_ARB_texture_non_power_of_two"))
    {
        hasNP2 = true;
        if(dbgexts) conoutf(CON_INIT, "Using GL_ARB_texture_non_power_of_two extension.");
    }
    else fatal("Non-power-of-two texture support is required!");

    if(hasext(exts, "GL_ARB_texture_compression") && hasext(exts, "GL_EXT_texture_compression_s3tc"))
    {
        glCompressedTexImage3D_ =    (PFNGLCOMPRESSEDTEXIMAGE3DARBPROC)   getprocaddress("glCompressedTexImage3DARB");
        glCompressedTexImage2D_ =    (PFNGLCOMPRESSEDTEXIMAGE2DARBPROC)   getprocaddress("glCompressedTexImage2DARB");
        glCompressedTexImage1D_ =    (PFNGLCOMPRESSEDTEXIMAGE1DARBPROC)   getprocaddress("glCompressedTexImage1DARB");
        glCompressedTexSubImage3D_ = (PFNGLCOMPRESSEDTEXSUBIMAGE3DARBPROC)getprocaddress("glCompressedTexSubImage3DARB");
        glCompressedTexSubImage2D_ = (PFNGLCOMPRESSEDTEXSUBIMAGE2DARBPROC)getprocaddress("glCompressedTexSubImage2DARB");
        glCompressedTexSubImage1D_ = (PFNGLCOMPRESSEDTEXSUBIMAGE1DARBPROC)getprocaddress("glCompressedTexSubImage1DARB");
        glGetCompressedTexImage_ =   (PFNGLGETCOMPRESSEDTEXIMAGEARBPROC)  getprocaddress("glGetCompressedTexImageARB");

        hasTC = true;
        if(dbgexts) conoutf(CON_INIT, "Using GL_EXT_texture_compression_s3tc extension.");
    }

    if(hasext(exts, "GL_EXT_texture_filter_anisotropic"))
    {
       GLint val;
       glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &val);
       hwmaxaniso = val;
       hasAF = true;
       if(dbgexts) conoutf(CON_INIT, "Using GL_EXT_texture_filter_anisotropic extension.");
    }

    if(hasext(exts, "GL_SGIS_generate_mipmap"))
    {
        hasGM = true;
        if(dbgexts) conoutf(CON_INIT, "Using GL_SGIS_generate_mipmap extension.");
    }

    if(hasext(exts, "GL_ARB_depth_texture"))
    {
        hasSGIDT = hasDT = true;
        if(dbgexts) conoutf(CON_INIT, "Using GL_ARB_depth_texture extension.");
    }
    else if(hasext(exts, "GL_SGIX_depth_texture"))
    {
        hasSGIDT = true;
        if(dbgexts) conoutf(CON_INIT, "Using GL_SGIX_depth_texture extension.");
    }
    if(!hasSGIDT && !hasDT) fatal("Depth texture support is required!");

    if(hasext(exts, "GL_ARB_shadow"))
    {
        hasSGISH = hasSH = true;
        if(nvidia || (ati && strstr(renderer, "Radeon HD"))) hasNVPCF = true;
        if(dbgexts) conoutf(CON_INIT, "Using GL_ARB_shadow extension.");
    }
    else if(hasext(exts, "GL_SGIX_shadow"))
    {
        hasSGISH = true;
        if(dbgexts) conoutf(CON_INIT, "Using GL_SGIX_shadow extension.");
    }
    if(!hasSGISH && !hasSH) fatal("Shadow mapping support is required!");

    if(hasext(exts, "GL_ARB_texture_gather"))
    {
        hasTG = true;
        if(dbgexts) conoutf(CON_INIT, "Using GL_ARB_texture_gather extension.");
    }
    else if(hasext(exts, "GL_AMD_texture_texture4"))
    {
        if(dbgexts) conoutf(CON_INIT, "Using GL_AMD_texture_texture4 extension.");
    }
    if(hasTG || hasT4) usetexgather = 1;

    if(hasext(exts, "GL_EXT_depth_bounds_test"))
    {
        glDepthBounds_ = (PFNGLDEPTHBOUNDSEXTPROC) getprocaddress("glDepthBoundsEXT");
        hasDBT = true;
        if(dbgexts) conoutf(CON_INIT, "Using GL_EXT_depth_bounds_test extension.");
    }

    if(hasext(exts, "GL_ARB_depth_clamp"))
    {
        hasDC = true;
        if(dbgexts) conoutf(CON_INIT, "Using GL_ARB_depth_clamp extension.");
    }
    else if(hasext(exts, "GL_NV_depth_clamp"))
    {
        hasDC = true;
        if(dbgexts) conoutf(CON_INIT, "Using GL_NV_depth_clamp extension");
    }
}

bool glext(const char *ext)
{
    const char *exts = (const char *)glGetString(GL_EXTENSIONS);
    return (hasext(exts, ext) ? true : false);
}

// GPU-side timing information will use OGL timers
enum { TIMER_SM = 0, TIMER_GBUFFER, TIMER_SHADING, TIMER_HDR, TIMER_AO, TIMER_RH, TIMER_TRANSPARENT, TIMER_SMAA, TIMER_SPLITTING, TIMER_MERGING, TIMER_CPU_SM, TIMER_CPU_GBUFFER, TIMER_CPU_SHADING, TIMER_N };
#define TIMER_GPU_N TIMER_CPU_SM
static const char *timer_string[] = {
    "shadow map",
    "g-buffer",
    "deferred shading",
    "hdr processing",
    "ambient obscurance",
    "radiance hints",
    "transparent",
    "smaa",
    "buffer splitting",
    "buffer merging",

    "shadow map",
    "g-buffer",
    "deferred shading"
};
// gpu
static const int timer_query_n = 4;
static int timer_curr = 0;
static GLuint timers[timer_query_n][TIMER_GPU_N];
static uint timer_used[timer_query_n];
// cpu
static int timer_began[TIMER_N];

static uint timer_available = 0;
static float timer_results[TIMER_N];
static float timer_prints[TIMER_N];
static int timer_last_print = 0;
VAR(timer, 0, 0, 1);

static void timer_sync()
{
    if(!timer) return;
    if(hasTQ) 
    {
        timer_curr = (timer_curr+1) % timer_query_n;
        loopi(TIMER_GPU_N)
        {
            if(timer_used[timer_curr]&(1<<i))
            {
                GLint available = 0;
                while(!available)
                    glGetQueryObjectiv_(timers[timer_curr][i], GL_QUERY_RESULT_AVAILABLE_ARB, &available);
                GLuint64EXT result = 0;
                glGetQueryObjectui64v_(timers[timer_curr][i], GL_QUERY_RESULT_ARB, &result);
                timer_results[i] = float(result) * 1e-6f;
                timer_available |= 1<<i;
            }
        }
        timer_used[timer_curr] = 0;
    }
}
static inline void timer_begin(int whichone)
{
    if(!timer || inbetweenframes) return;
    if(whichone < TIMER_GPU_N)
    {
        if(!hasTQ) return;
        deferquery++;
        glBeginQuery_(GL_TIME_ELAPSED_EXT, timers[timer_curr][whichone]);
    }
    else timer_began[whichone] = getclockmillis();
}
static inline void timer_end(int whichone)
{
    if(!timer || inbetweenframes) return;
    if(whichone < TIMER_GPU_N)
    {
        if(!hasTQ) return; 
        glEndQuery_(GL_TIME_ELAPSED_EXT);
        timer_used[timer_curr] |= 1<<whichone;
        deferquery--;
    }
    else 
    {
        timer_results[whichone] = float(getclockmillis() - timer_began[whichone]);
        timer_available |= 1<<whichone;
    }
}
static inline void timer_nextframe()
{
    timer_available = 0;
}
static void timer_print(int conw, int conh)
{
    if(!timer || !timer_available) return;
    if(totalmillis - timer_last_print >= 200) // keep the timer read-outs from looking spastic on the hud
    {
        memcpy(timer_prints, timer_results, sizeof(timer_prints));
        timer_last_print = totalmillis;
    }        
    int offset = 0;
    loopi(TIMER_N)
    {
        if(!(timer_available&(1<<i))) continue;
        draw_textf("%s%s %5.2f ms", conw-20*FONTH, conh-FONTH*3/2-offset*9*FONTH/8, timer_string[i], (i < TIMER_GPU_N)?"":" (cpu)", timer_prints[i]);
        offset++;
    }
}
static void timer_setup()
{
    memset(timer_used, 0, sizeof(timer_used));
    memset(timer_results, 0, sizeof(timer_results));
    memset(timer_prints, 0, sizeof(timer_prints));
    if(hasTQ) loopi(timer_query_n) glGenQueries_(TIMER_GPU_N, timers[i]);
}
void cleanuptimer() 
{ 
    if(hasTQ) loopi(timer_query_n) glDeleteQueries_(TIMER_GPU_N, timers[i]); 
}
static void frametimer_print(int conw, int conh)
{
    extern int framemillis, frametimer;
    if(!frametimer) return;
    static int lastprint = 0, printmillis = 0;
    if(totalmillis - lastprint >= 200) 
    {
        printmillis = framemillis;
        lastprint = totalmillis;
    }
    draw_textf("frame time %i ms", conw-40*FONTH, conh-FONTH*3/2, printmillis);
}

void gl_init(int w, int h, int bpp, int depth, int fsaa)
{
    glViewport(0, 0, w, h);
    glClearColor(0, 0, 0, 0);
    glClearDepth(1);
    glClearStencil(0);
    glDepthFunc(GL_LESS);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 0, ~0);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glShadeModel(GL_SMOOTH);
    
    glEnable(GL_LINE_SMOOTH);
    //glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    glFrontFace(GL_CW);
    glCullFace(GL_BACK);
    glDisable(GL_CULL_FACE);

#ifdef __APPLE__
    if(sdl_backingstore_bug)
    {
        if(fsaa)
        {
            sdl_backingstore_bug = 1;
            // since SDL doesn't add kCGLPFABackingStore to the pixelformat and so it isn't guaranteed to be preserved - only manifests when using fsaa?
            //conoutf(CON_WARN, "WARNING: Using SDL backingstore workaround. (use \"/sdl_backingstore_bug 0\" to disable if unnecessary)");
        }
        else sdl_backingstore_bug = -1;
    }
#endif

    renderpath = R_GLSLANG;

    static const char * const rpnames[1] = { "GLSL shader" };
    conoutf(CON_INIT, "Rendering using the OpenGL %s path.", rpnames[renderpath]);

    extern void initgbuffer();
    initgbuffer();

    setuptexcompress();
    timer_setup();

    vieww = w;
    viewh = h;
}

static void cleanuplightsphere();

void cleanupgl()
{
    extern void cleanupmotionblur();
    cleanupmotionblur();

    extern void clearminimap();
    clearminimap();

    extern void cleanupgbuffer();
    cleanupgbuffer();

    extern void cleanupbloom();
    cleanupbloom();

    extern void cleanupao();
    cleanupao();

    extern void cleanupsmaa();
    cleanupsmaa();

    extern void cleanupshadowatlas();
    cleanupshadowatlas();

    extern void cleanupradiancehints();
    cleanupradiancehints();

    extern void cleanuptimer();
    cleanuptimer();

    cleanuplightsphere();
}

#define VARRAY_INTERNAL
#include "varray.h"

VAR(wireframe, 0, 0, 1);

vec worldpos, camdir, camright, camup;

void findorientation()
{
    TargetingControl::setupOrientation();
}

void transplayer()
{
    // move from RH to Z-up LH quake style worldspace
    glLoadMatrixf(viewmatrix.v);

    glRotatef(camera1->roll, 0, 1, 0);
    glRotatef(camera1->pitch, -1, 0, 0);
    glRotatef(camera1->yaw, 0, 0, -1);

    glTranslatef(-camera1->o.x, -camera1->o.y, -camera1->o.z);   
}

int vieww = -1, viewh = -1;
float curfov = 100, curavatarfov = 65, fovy, aspect;
int farplane;
VARP(zoominvel, 0, 250, 5000);
VARP(zoomoutvel, 0, 100, 5000);
VARP(zoomfov, 10, 35, 60);
VARP(fov, 10, 100, 150);
VAR(avatarzoomfov, 10, 25, 60);
VAR(avatarfov, 10, 65, 150);
FVAR(avatardepth, 0, 0.5f, 1);

static int zoommillis = 0;
VARF(zoom, -1, 0, 1,
    if(zoom) zoommillis = totalmillis;
);

void disablezoom()
{
    zoom = 0;
    zoommillis = totalmillis;
}

void computezoom()
{
    extern float forcedCameraFov; // INTENSITY: forced camera stuff
    if (forcedCameraFov > 0)
    {
        curfov = forcedCameraFov;
        forcedCameraFov = -1; // Prepare for next frame
        return;
    } // INTENSITY: end forced camera stuff

    if(!zoom) { curfov = fov; curavatarfov = avatarfov; return; }
    if(zoom < 0 && curfov >= fov) { zoom = 0; curfov = fov; curavatarfov = avatarfov; return; } // don't zoom-out if not zoomed-in
    int zoomvel = zoom > 0 ? zoominvel : zoomoutvel,
        oldfov = zoom > 0 ? fov : zoomfov,
        newfov = zoom > 0 ? zoomfov : fov,
        oldavatarfov = zoom > 0 ? avatarfov : avatarzoomfov,
        newavatarfov = zoom > 0 ? avatarzoomfov : avatarfov;
    float t = zoomvel ? float(zoomvel - (totalmillis - zoommillis)) / zoomvel : 0;
    if(t <= 0) 
    {
        if(!zoomvel && fabs(newfov - curfov) >= 1) 
        {
            curfov = newfov;
            curavatarfov = newavatarfov;
        }
        zoom = max(zoom, 0);
    }
    else 
    {
        curfov = oldfov*t + newfov*(1 - t);
        curavatarfov = oldavatarfov*t + newavatarfov*(1 - t);
    }
}

FVARP(zoomsens, 1e-3f, 1, 1000);
FVARP(zoomaccel, 0, 0, 1000);
VARP(zoomautosens, 0, 1, 1);
FVARP(sensitivity, 1e-3f, 3, 1000);
FVARP(sensitivityscale, 1e-3f, 1, 1000);
VARP(invmouse, 0, 0, 1);
FVARP(mouseaccel, 0, 0, 1000);
 
VAR(thirdperson, 0, 0, 2);
FVAR(thirdpersondistance, 0, 20, 1000);
physent *camera1 = NULL;
bool detachedcamera = false;
bool isthirdperson() { return player!=camera1 || detachedcamera; }

void fixcamerarange()
{
    const float MAXPITCH = 90.0f;
    if(camera1->pitch>MAXPITCH) camera1->pitch = MAXPITCH;
    if(camera1->pitch<-MAXPITCH) camera1->pitch = -MAXPITCH;
    while(camera1->yaw<0.0f) camera1->yaw += 360.0f;
    while(camera1->yaw>=360.0f) camera1->yaw -= 360.0f;
}

void mousemove(int dx, int dy)
{
    float cursens = sensitivity, curaccel = mouseaccel;
    if(zoom)
    {
        if(zoomautosens) 
        {
            cursens = float(sensitivity*zoomfov)/fov;
            curaccel = float(mouseaccel*zoomfov)/fov;
        }
        else 
        {
            cursens = zoomsens;
            curaccel = zoomaccel;
        }
    }
    if(curaccel && curtime && (dx || dy)) cursens += curaccel * sqrtf(dx*dx + dy*dy)/curtime;
    cursens /= 33.0f*sensitivityscale;

    // INTENSITY: Let scripts customize mousemoving
    if (lapi::state.state())
    {
        lua::Table t = lapi::state.get<lua::Function>(
            "LAPI", "Input", "Events", "Client", "mouse_move"
        ).call<lua::Object>(
            (dx * cursens),
            (-dy * cursens * (invmouse ? -1 : 1))
        );
        camera1->yaw   += t["yaw"  ].to<double>();
        camera1->pitch += t["pitch"].to<double>();

        fixcamerarange();
        if(camera1!=player && !detachedcamera)
        {
            player->yaw   = camera1->yaw;
            player->pitch = camera1->pitch;
        }
    }
}

void recomputecamera()
{
    game::setupcamera();
    computezoom();

    bool shoulddetach = thirdperson > 1 || game::detachcamera();
    if(!thirdperson && !shoulddetach)
    {
        camera1 = player;
        detachedcamera = false;
    }
    else
    {
        static physent tempcamera;
        camera1 = &tempcamera;
        if(detachedcamera && shoulddetach) camera1->o = player->o;
        else
        {
          // INTENSITY: If we are not character viewing, align with the player
          if (!GuiControl::isCharacterViewing())
            *camera1 = *player;

            detachedcamera = shoulddetach;
        }
        camera1->reset();
        camera1->type = ENT_CAMERA;
        camera1->collidetype = COLLIDE_AABB;
        camera1->move = -1;
        camera1->eyeheight = camera1->aboveeye = camera1->radius = camera1->xradius = camera1->yradius = 2;

        CameraControl::positionCamera(camera1);
    }

    setviewcell(camera1->o);
}

extern const glmatrixf viewmatrix(vec4(-1, 0, 0, 0), vec4(0, 0, 1, 0), vec4(0, -1, 0, 0));
extern const glmatrixf invviewmatrix(vec4(-1, 0, 0, 0), vec4(0, 0, -1, 0), vec4(0, 1, 0, 0));
glmatrixf mvmatrix, projmatrix, mvpmatrix, invmvmatrix, invmvpmatrix, invprojmatrix, eyematrix, worldmatrix, linearworldmatrix;

void readmatrices()
{
    glGetFloatv(GL_MODELVIEW_MATRIX, mvmatrix.v);
    glGetFloatv(GL_PROJECTION_MATRIX, projmatrix.v);

    mvpmatrix.mul(projmatrix, mvmatrix);
    invmvmatrix.invert(mvmatrix);
    invmvpmatrix.invert(mvpmatrix);
    invprojmatrix.invert(projmatrix);
}

FVAR(nearplane, 1e-3f, 0.54f, 1e3f);

void project(float fovy, float aspect, int farplane, float zscale = 1)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if(zscale!=1) glScalef(1, 1, zscale);
    GLdouble ydist = nearplane * tan(fovy/2*RAD), xdist = ydist * aspect;
    glFrustum(-xdist, xdist, -ydist, ydist, nearplane, farplane);
    glMatrixMode(GL_MODELVIEW);
}


vec calcavatarpos(const vec &pos, float dist)
{
    vec eyepos;
    mvmatrix.transform(pos, eyepos);
    GLdouble ydist = nearplane * tan(curavatarfov/2*RAD), xdist = ydist * aspect;
    vec4 scrpos;
    scrpos.x = eyepos.x*nearplane/xdist;
    scrpos.y = eyepos.y*nearplane/ydist;
    scrpos.z = (eyepos.z*(farplane + nearplane) - 2*nearplane*farplane) / (farplane - nearplane);
    scrpos.w = -eyepos.z;

    vec worldpos = invmvpmatrix.perspectivetransform(scrpos);
    vec dir = vec(worldpos).sub(camera1->o).rescale(dist);
    return dir.add(camera1->o);
}

VAR(reflectclip, 0, 6, 64);
VAR(reflectclipavatar, -64, 0, 64);

static const glmatrixf dummymatrix;
static int projectioncount = 0;
void pushprojection(const glmatrixf &m = dummymatrix)
{
    glMatrixMode(GL_PROJECTION);
    if(projectioncount <= 0) glPushMatrix();
    if(&m != &dummymatrix) glLoadMatrixf(m.v);
    glMatrixMode(GL_MODELVIEW);
    projectioncount++;
}

void popprojection()
{
    --projectioncount;
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    if(projectioncount > 0)
    {
        glPushMatrix();
    }
    glMatrixMode(GL_MODELVIEW);
}

FVAR(polygonoffsetfactor, -1e4f, -3.0f, 1e4f);
FVAR(polygonoffsetunits, -1e4f, -3.0f, 1e4f);
FVAR(depthoffset, -1e4f, 0.01f, 1e4f);

void enablepolygonoffset(GLenum type)
{
    if(!depthoffset)
    {
        glPolygonOffset(polygonoffsetfactor, polygonoffsetunits);
        glEnable(type);
        return;
    }
    
    glmatrixf offsetmatrix = projmatrix;
    offsetmatrix[14] += depthoffset * projmatrix[10];

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadMatrixf(offsetmatrix.v);
    glMatrixMode(GL_MODELVIEW);
}

void disablepolygonoffset(GLenum type)
{
    if(!depthoffset)
    {
        glDisable(type);
        return;
    }
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

bool calcspherescissor(const vec &center, float size, float &sx1, float &sy1, float &sx2, float &sy2, float &sz1, float &sz2)
{
    vec e(mvmatrix.transformx(center),
          mvmatrix.transformy(center),
          mvmatrix.transformz(center));
    if(e.z > 2*size) { sx1 = sy1 = sz1 = 1; sx2 = sy2 = sz2 = -1; return false; }
    float zzrr = e.z*e.z - size*size,
          dx = e.x*e.x + zzrr, dy = e.y*e.y + zzrr,
          focaldist = 1.0f/tan(fovy*0.5f*RAD);
    sx1 = sy1 = -1;
    sx2 = sy2 = 1;
    #define CHECKPLANE(c, dir, focaldist, low, high) \
    do { \
        float nzc = (cz*cz + 1) / (cz dir drt) - cz, \
              pz = (d##c)/(nzc*e.c - e.z); \
        if(pz > 0) \
        { \
            float c = (focaldist)*nzc, \
                  pc = pz*nzc; \
            if(pc < e.c) low = c; \
            else if(pc > e.c) high = c; \
        } \
    } while(0)
    if(dx > 0)
    {
        float cz = e.x/e.z, drt = sqrtf(dx)/size;
        CHECKPLANE(x, -, focaldist/aspect, sx1, sx2);
        CHECKPLANE(x, +, focaldist/aspect, sx1, sx2);
    }
    if(dy > 0)
    {
        float cz = e.y/e.z, drt = sqrtf(dy)/size;
        CHECKPLANE(y, -, focaldist, sy1, sy2);
        CHECKPLANE(y, +, focaldist, sy1, sy2);
    }
    float z1 = min(e.z + size, -1e-3f - nearplane), z2 = min(e.z - size, -1e-3f - nearplane);
    sz1 = (z1*projmatrix.v[10] + projmatrix.v[14]) / (z1*projmatrix.v[11] + projmatrix.v[15]);
    sz2 = (z2*projmatrix.v[10] + projmatrix.v[14]) / (z2*projmatrix.v[11] + projmatrix.v[15]);
    return sx1 < sx2 && sy1 < sy2 && sz1 < sz2;
}

bool calcbbscissor(const ivec &bbmin, const ivec &bbmax, float &sx1, float &sy1, float &sx2, float &sy2)
{
#define ADDXYSCISSOR(p) do { \
        if(p.z >= -p.w) \
        { \
            float x = p.x / p.w, y = p.y / p.w; \
            sx1 = min(sx1, x); \
            sy1 = min(sy1, y); \
            sx2 = max(sx2, x); \
            sy2 = max(sy2, y); \
        } \
    } while(0)
    vec4 v[8];
    sx1 = sy1 = 1;
    sx2 = sy2 = -1;
    mvpmatrix.transform(vec(bbmin.x, bbmin.y, bbmin.z), v[0]);
    ADDXYSCISSOR(v[0]);
    mvpmatrix.transform(vec(bbmax.x, bbmin.y, bbmin.z), v[1]);
    ADDXYSCISSOR(v[1]);
    mvpmatrix.transform(vec(bbmin.x, bbmax.y, bbmin.z), v[2]);
    ADDXYSCISSOR(v[2]);
    mvpmatrix.transform(vec(bbmax.x, bbmax.y, bbmin.z), v[3]);
    ADDXYSCISSOR(v[3]);
    mvpmatrix.transform(vec(bbmin.x, bbmin.y, bbmax.z), v[4]);
    ADDXYSCISSOR(v[4]);
    mvpmatrix.transform(vec(bbmax.x, bbmin.y, bbmax.z), v[5]);
    ADDXYSCISSOR(v[5]);
    mvpmatrix.transform(vec(bbmin.x, bbmax.y, bbmax.z), v[6]);
    ADDXYSCISSOR(v[6]);
    mvpmatrix.transform(vec(bbmax.x, bbmax.y, bbmax.z), v[7]);
    ADDXYSCISSOR(v[7]);
    if(sx1 > sx2 || sy1 > sy2) return false;
    loopi(8)
    {
        const vec4 &p = v[i];
        if(p.z >= -p.w) continue;
        loopj(3)
        {
            const vec4 &o = v[i^(1<<j)];
            if(o.z <= -o.w) continue;
#define INTERPXYSCISSOR(p, o) do { \
            float t = (p.z + p.w)/(p.z + p.w - o.z - o.w), \
                  w = p.w + t*(o.w - p.w), \
                  x = (p.x + t*(o.x - p.x))/w, \
                  y = (p.y + t*(o.y - p.y))/w; \
            sx1 = min(sx1, x); \
            sy1 = min(sy1, y); \
            sx2 = max(sx2, x); \
            sy2 = max(sy2, y); \
        } while(0)
            INTERPXYSCISSOR(p, o);
        }
    }
    sx1 = max(sx1, -1.0f);
    sy1 = max(sy1, -1.0f);
    sx2 = min(sx2, 1.0f);
    sy2 = min(sy2, 1.0f);
    return true;
}

bool calcspotscissor(const vec &origin, float radius, const vec &dir, int spot, const vec &spotx, const vec &spoty, float &sx1, float &sy1, float &sx2, float &sy2, float &sz1, float &sz2)
{
    const vec2 &sc = sincos360[spot];
    float spotscale = radius*sc.y/sc.x;
    vec up = vec(spotx).rescale(spotscale), right = vec(spoty).rescale(spotscale), center = vec(dir).mul(radius).add(origin);
#define ADDXYZSCISSOR(p) do { \
        if(p.z >= -p.w) \
        { \
            float x = p.x / p.w, y = p.y / p.w, z = p.z / p.w; \
            sx1 = min(sx1, x); \
            sy1 = min(sy1, y); \
            sz1 = min(sz1, z); \
            sx2 = max(sx2, x); \
            sy2 = max(sy2, y); \
            sz2 = max(sz2, z); \
        } \
    } while(0)
    vec4 v[5];
    sx1 = sy1 = sz1 = 1;
    sx2 = sy2 = sz2 = -1;
    mvpmatrix.transform(vec(center).sub(right).sub(up), v[0]);
    ADDXYZSCISSOR(v[0]);
    mvpmatrix.transform(vec(center).add(right).sub(up), v[1]);
    ADDXYZSCISSOR(v[1]);
    mvpmatrix.transform(vec(center).sub(right).add(up), v[2]);
    ADDXYZSCISSOR(v[2]);
    mvpmatrix.transform(vec(center).add(right).add(up), v[3]);
    ADDXYZSCISSOR(v[3]);
    mvpmatrix.transform(origin, v[4]); 
    ADDXYZSCISSOR(v[4]);
    if(sx1 > sx2 || sy1 > sy2 || sz1 > sz2) return false;
    loopi(4)
    {
        const vec4 &p = v[i];
        if(p.z >= -p.w) continue;
        loopj(2)
        {
            const vec4 &o = v[i^(1<<j)];
            if(o.z <= -o.w) continue;
#define INTERPXYZSCISSOR(p, o) do { \
            float t = (p.z + p.w)/(p.z + p.w - o.z - o.w), \
                  w = p.w + t*(o.w - p.w), \
                  x = (p.x + t*(o.x - p.x))/w, \
                  y = (p.y + t*(o.y - p.y))/w; \
            sx1 = min(sx1, x); \
            sy1 = min(sy1, y); \
            sz1 = min(sz1, -1.0f); \
            sx2 = max(sx2, x); \
            sy2 = max(sy2, y); \
        } while(0)
            INTERPXYZSCISSOR(p, o);
        }
        if(v[4].z > -v[4].w) INTERPXYZSCISSOR(p, v[4]);
    }
    if(v[4].z < -v[4].w) loopj(4)
    {
        const vec4 &o = v[j];
        if(o.z <= -o.w) continue;
        INTERPXYZSCISSOR(v[4], o);
    }
    sx1 = max(sx1, -1.0f);
    sy1 = max(sy1, -1.0f);
    sz1 = max(sz1, -1.0f);
    sx2 = min(sx2, 1.0f);
    sy2 = min(sy2, 1.0f);
    sz2 = min(sz2, 1.0f);
    return true;
}

static int scissoring = 0;
static GLint oldscissor[4];

int pushscissor(float sx1, float sy1, float sx2, float sy2)
{
    scissoring = 0;

    if(sx1 <= -1 && sy1 <= -1 && sx2 >= 1 && sy2 >= 1) return 0;

    sx1 = max(sx1, -1.0f);
    sy1 = max(sy1, -1.0f);
    sx2 = min(sx2, 1.0f);
    sy2 = min(sy2, 1.0f);

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    int sx = viewport[0] + int(floor((sx1+1)*0.5f*viewport[2])),
        sy = viewport[1] + int(floor((sy1+1)*0.5f*viewport[3])),
        sw = viewport[0] + int(ceil((sx2+1)*0.5f*viewport[2])) - sx,
        sh = viewport[1] + int(ceil((sy2+1)*0.5f*viewport[3])) - sy;
    if(sw <= 0 || sh <= 0) return 0;

    if(glIsEnabled(GL_SCISSOR_TEST))
    {
        glGetIntegerv(GL_SCISSOR_BOX, oldscissor);
        sw += sx;
        sh += sy;
        sx = max(sx, int(oldscissor[0]));
        sy = max(sy, int(oldscissor[1]));
        sw = min(sw, int(oldscissor[0] + oldscissor[2])) - sx;
        sh = min(sh, int(oldscissor[1] + oldscissor[3])) - sy;
        if(sw <= 0 || sh <= 0) return 0;
        scissoring = 2;
    }
    else scissoring = 1;

    glScissor(sx, sy, sw, sh);
    if(scissoring<=1) glEnable(GL_SCISSOR_TEST);
    
    return scissoring;
}

void popscissor()
{
    if(scissoring>1) glScissor(oldscissor[0], oldscissor[1], oldscissor[2], oldscissor[3]);
    else if(scissoring) glDisable(GL_SCISSOR_TEST);
    scissoring = 0;
}

VARR(fog, 16, 4000, 1000024);
bvec fogcolor(0x80, 0x99, 0xB3);
HVARFR(fogcolour, 0, 0x8099B3, 0xFFFFFF,
{
    fogcolor = bvec((fogcolour>>16)&0xFF, (fogcolour>>8)&0xFF, fogcolour&0xFF);
});
VAR(fogoverlay, 0, 1, 1);

static float findsurface(int fogmat, const vec &v, int &abovemat)
{
    ivec o(v), co;
    int csize;
    do
    {
        cube &c = lookupcube(o.x, o.y, o.z, 0, co, csize);
        int mat = c.material&MATF_VOLUME;
        if(mat != fogmat)
        {
            abovemat = isliquid(mat) ? mat : MAT_AIR;
            return o.z;
        }
        o.z = co.z + csize;
    }
    while(o.z < worldsize);
    abovemat = MAT_AIR;
    return worldsize;
}

static void blendfog(int fogmat, float below, float blend, float logblend, float &start, float &end, vec &fogc)
{
    switch(fogmat)
    {
        case MAT_WATER:
        {
            float deepfade = clamp(below/max(waterdeep, waterfog), 0.0f, 1.0f);
            vec color;
            color.lerp(watercolor.tocolor(), waterdeepcolor.tocolor(), deepfade);
            fogc.add(vec(color).mul(blend));
            end += logblend*min(fog, max(waterfog*2, 16));
            break;
        }

        case MAT_LAVA:
            fogc.add(lavacolor.tocolor().mul(blend));
            end += logblend*min(fog, max(lavafog*2, 16));
            break;

        default:
            fogc.add(fogcolor.tocolor().mul(blend));
            start += logblend*(fog+64)/8;
            end += logblend*fog;
            break;
    }
}

vec curfogcolor(0, 0, 0);

void setfogcolor(const vec &v)
{
    GLOBALPARAM(fogcolor, (v));
}

void zerofogcolor()
{
    setfogcolor(vec(0, 0, 0));
}

void resetfogcolor()
{
    setfogcolor(curfogcolor);
}

FVAR(fogscale, 0, 1.5f, 1e3f);

static void setfog(int fogmat, float below = 0, float blend = 1, int abovemat = MAT_AIR)
{
    float start = 0, end = 0;
    float logscale = 256, logblend = log(1 + (logscale - 1)*blend) / log(logscale);

    curfogcolor = vec(0, 0, 0);
    blendfog(fogmat, below, blend, logblend, start, end, curfogcolor);
    if(blend < 1) blendfog(abovemat, 0, 1-blend, 1-logblend, start, end, curfogcolor);
    curfogcolor.mul(ldrscale);

    GLOBALPARAM(fogcolor, (curfogcolor));
    GLOBALPARAM(fogparams, (start, end, 1/(end - start)));
}

static void blendfogoverlay(int fogmat, float below, float blend, float *overlay)
{
    float maxc;
    switch(fogmat)
    {
        case MAT_WATER:
        {
            float deepfade = clamp(below/max(waterdeep, waterfog), 0.0f, 1.0f);
            vec color;
            loopk(3) color[k] = watercolor[k]*(1-deepfade) + waterdeepcolor[k]*deepfade;
            maxc = max(color[0], max(color[1], color[2]));
            loopk(3) overlay[k] += blend*max(0.4f, color[k]/min(32.0f + maxc*7.0f/8.0f, 255.0f));
            break;
        }

        case MAT_LAVA:
            maxc = max(lavacolor[0], max(lavacolor[1], lavacolor[2]));
            loopk(3) overlay[k] += blend*max(0.4f, lavacolor[k]/min(32.0f + maxc*7.0f/8.0f, 255.0f));
            break;

        default:
            loopk(3) overlay[k] += blend;
            break;
    }
}

void drawfogoverlay(int fogmat, float fogbelow, float fogblend, int abovemat)
{
    notextureshader->set();
    glDisable(GL_TEXTURE_2D);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ZERO, GL_SRC_COLOR);
    float overlay[3] = { 0, 0, 0 };
    blendfogoverlay(fogmat, fogbelow, fogblend, overlay);
    blendfogoverlay(abovemat, 0, 1-fogblend, overlay);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glColor3fv(overlay);
    glBegin(GL_TRIANGLE_STRIP);
    glVertex2f(-1, -1);
    glVertex2f(1, -1);
    glVertex2f(-1, 1);
    glVertex2f(1, 1);
    glEnd();
    glDisable(GL_BLEND);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glEnable(GL_TEXTURE_2D);
    defaultshader->set();
}

bool envmapping = false;
int minimapping = 0;

GLuint minimaptex = 0;
vec minimapcenter(0, 0, 0), minimapradius(0, 0, 0), minimapscale(0, 0, 0);

void clearminimap()
{
    if(minimaptex) { glDeleteTextures(1, &minimaptex); minimaptex = 0; }
}

VARR(minimapheight, 0, 0, 2<<16);
bvec minimapcolor(0, 0, 0);
HVARFR(minimapcolour, 0, 0, 0xFFFFFF,
{
    minimapcolor = bvec((minimapcolour>>16)&0xFF, (minimapcolour>>8)&0xFF, minimapcolour&0xFF);
});
VARR(minimapclip, 0, 0, 1);
VARFP(minimapsize, 7, 8, 10, { if(minimaptex) drawminimap(); });

void bindminimap()
{
    glBindTexture(GL_TEXTURE_2D, minimaptex);
}

void clipminimap(ivec &bbmin, ivec &bbmax, cube *c = worldroot, int x = 0, int y = 0, int z = 0, int size = worldsize>>1)
{
    loopi(8)
    {
        ivec o(i, x, y, z, size);
        if(c[i].children) clipminimap(bbmin, bbmax, c[i].children, o.x, o.y, o.z, size>>1);
        else if(!isentirelysolid(c[i]) && (c[i].material&MATF_CLIP)!=MAT_CLIP)
        {
            loopk(3) bbmin[k] = min(bbmin[k], o[k]);
            loopk(3) bbmax[k] = max(bbmax[k], o[k] + size);
        }
    }
}

GLuint motiontex = 0;
int motionw = 0, motionh = 0, lastmotion = 0;

void cleanupmotionblur()
{
    if(motiontex) { glDeleteTextures(1, &motiontex); motiontex = 0; }
    motionw = motionh = 0;
    lastmotion = 0;
}

VARFP(motionblur, 0, 0, 1, { if(!motionblur) cleanupmotionblur(); });
VARP(motionblurmillis, 1, 5, 1000);
FVARP(motionblurscale, 0, 0.5f, 1);

void addmotionblur()
{
    if(!motionblur || !hasTR || max(screen->w, screen->h) > hwtexsize) return;

    if(paused || game::ispaused()) { lastmotion = 0; return; }

    if(!motiontex || motionw != screen->w || motionh != screen->h)
    {
        if(!motiontex) glGenTextures(1, &motiontex);
        motionw = screen->w;
        motionh = screen->h;
        lastmotion = 0;
        createtexture(motiontex, motionw, motionh, NULL, 3, 0, GL_RGB, GL_TEXTURE_RECTANGLE_ARB);
    }

    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, motiontex);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDisable(GL_TEXTURE_2D);
    glEnable(GL_TEXTURE_RECTANGLE_ARB);

    rectshader->set();

    glColor4f(1, 1, 1, lastmotion ? pow(motionblurscale, max(float(lastmillis - lastmotion)/motionblurmillis, 1.0f)) : 0);
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(      0,       0); glVertex2f(-1, -1);
    glTexCoord2f(motionw,       0); glVertex2f( 1, -1);
    glTexCoord2f(      0, motionh); glVertex2f(-1,  1);
    glTexCoord2f(motionw, motionh); glVertex2f( 1,  1);
    glEnd();

    glDisable(GL_TEXTURE_RECTANGLE_ARB);
    glEnable(GL_TEXTURE_2D);

    glDisable(GL_BLEND);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
 
    if(lastmillis - lastmotion >= motionblurmillis)
    {
        lastmotion = lastmillis - lastmillis%motionblurmillis;

        glCopyTexSubImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, 0, screen->w, screen->h);
    }
}

bool dopostfx = false;

void invalidatepostfx()
{
    dopostfx = false;
}

void gl_drawhud(int w, int h);

int xtraverts, xtravertsva;

void screenquad(float sw, float sh)
{
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(sw, 0); glVertex2f(1, -1);
    glTexCoord2f(0, 0); glVertex2f(-1, -1);
    glTexCoord2f(sw, sh); glVertex2f(1, 1);
    glTexCoord2f(0, sh); glVertex2f(-1, 1);
    glEnd();
}

void screenquad(float sw, float sh, float sw2, float sh2)
{
    glBegin(GL_TRIANGLE_STRIP);
    glMultiTexCoord2f_(GL_TEXTURE0_ARB, sw, 0); glMultiTexCoord2f_(GL_TEXTURE1_ARB, sw2, 0); glVertex2f(1, -1);
    glMultiTexCoord2f_(GL_TEXTURE0_ARB, 0, 0); glMultiTexCoord2f_(GL_TEXTURE1_ARB, 0, 0); glVertex2f(-1, -1);
    glMultiTexCoord2f_(GL_TEXTURE0_ARB, sw, sh); glMultiTexCoord2f_(GL_TEXTURE1_ARB, sw2, sh2); glVertex2f(1, 1);
    glMultiTexCoord2f_(GL_TEXTURE0_ARB, 0, sh); glMultiTexCoord2f_(GL_TEXTURE1_ARB, 0, sh2); glVertex2f(-1, 1);
    glEnd();
}

int gw = -1, gh = -1, bloomw = -1, bloomh = -1, lasthdraccum = 0;
GLuint gfbo = 0, gdepthtex = 0, gcolortex = 0, gnormaltex = 0, gglowtex = 0, gdepthrb = 0, gstencilrb = 0;
GLuint smaaareatex = 0, smaasearchtex = 0, smaafbo[3] = { 0, 0, 0 };
GLuint hdrfbo = 0, hdrtex = 0, bloomfbo[6] = { 0, 0, 0, 0, 0 }, bloomtex[6] = { 0, 0, 0, 0, 0 };
int hdrclear = 0;
GLuint refractfbo = 0, refracttex = 0;
GLenum bloomformat = 0, hdrformat = 0;
bool hdrfloat = false;

int aow = -1, aoh = -1;
GLuint aofbo[3] = { 0, 0, 0 }, aotex[3] = { 0, 0, 0 }, aonoisetex = 0;

extern int bloomsize, bloomprec;

void setupbloom(int w, int h)
{
    int maxsize = ((1<<bloomsize)*5)/4;
    while(w >= maxsize || h >= maxsize) 
    {
        w /= 2;
        h /= 2;
    }
    if(w == bloomw && h == bloomh) return;
    bloomw = w;
    bloomh = h;

    loopi(5) if(!bloomtex[i]) glGenTextures(1, &bloomtex[i]);

    loopi(5) if(!bloomfbo[i]) glGenFramebuffers_(1, &bloomfbo[i]);

    bloomformat = bloomprec >= 3 && hasTF ? GL_RGB16F_ARB : (bloomprec >= 2 && hasPF ? GL_R11F_G11F_B10F_EXT : (bloomprec >= 1 ? GL_RGB10 : GL_RGB));
    createtexture(bloomtex[0], max(gw/2, bloomw), max(gh/2, bloomh), NULL, 3, 1, bloomformat, GL_TEXTURE_RECTANGLE_ARB);
    createtexture(bloomtex[1], max(gw/4, bloomw), max(gh/4, bloomh), NULL, 3, 1, bloomformat, GL_TEXTURE_RECTANGLE_ARB);
    createtexture(bloomtex[2], bloomw, bloomh, NULL, 3, 1, GL_RGB, GL_TEXTURE_RECTANGLE_ARB);
    createtexture(bloomtex[3], bloomw, bloomh, NULL, 3, 1, GL_RGB, GL_TEXTURE_RECTANGLE_ARB);
    if(bloomformat != GL_RGB)
    {
        if(!bloomtex[5]) glGenTextures(1, &bloomtex[5]);
        if(!bloomfbo[5]) glGenFramebuffers_(1, &bloomfbo[5]);
        createtexture(bloomtex[5], bloomw, bloomh, NULL, 3, 1, bloomformat, GL_TEXTURE_RECTANGLE_ARB); 
    }

    static uchar gray[3] = { 32, 32, 32 };
    static float grayf[3] = { 0.125f, 0.125f, 0.125f };
    createtexture(bloomtex[4], 1, 1, hasTF ? (void *)grayf : (void *)gray, 3, 1, hasTF ? GL_RGB16F_ARB : GL_RGB16, GL_TEXTURE_RECTANGLE_ARB);

    loopi(5 + (bloomformat != GL_RGB ? 1 : 0))
    {
        glBindFramebuffer_(GL_FRAMEBUFFER_EXT, bloomfbo[i]);
        glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_ARB, bloomtex[i], 0);

        if(glCheckFramebufferStatus_(GL_FRAMEBUFFER_EXT) != GL_FRAMEBUFFER_COMPLETE_EXT)
            fatal("failed allocating bloom buffer!");
    }

    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, 0);
}

void cleanupbloom()
{
    loopi(6) if(bloomfbo[i]) { glDeleteFramebuffers_(1, &bloomfbo[i]); bloomfbo[i] = 0; }
    loopi(6) if(bloomtex[i]) { glDeleteTextures(1, &bloomtex[i]); bloomtex[i] = 0; }
    bloomw = bloomh = -1;
    lasthdraccum = 0;
}

extern int ao, aotaps, aoreduce, aoreducedepth, aonoise, aobilateral, aopackdepth;

static Shader *bilateralshader[2] = { NULL, NULL };

Shader *loadbilateralshader(int pass)
{
    if(!aobilateral) return nullshader;

    string opts;
    int optslen = 0;

    if(aoreduce) opts[optslen++] = 'r';
    bool linear = hasTRG && hasTF && (aopackdepth || (aoreducedepth && (aoreduce || aoreducedepth > 1)));
    if(linear)
    {
        opts[optslen++] = 'l';
        if(aopackdepth) opts[optslen++] = 'p';
    }
    opts[optslen] = '\0';

    defformatstring(name)("bilateral%c%s%d", 'x' + pass, opts, aobilateral);
    return generateshader(name, "bilateralshader(\"%s\", %d)", opts, aobilateral);
}

void loadbilateralshaders()
{
    loopk(2) bilateralshader[k] = loadbilateralshader(k);
}

void clearbilateralshaders()
{
    loopk(2) bilateralshader[k] = NULL;
}

static Shader *ambientobscuranceshader = NULL;

Shader *loadambientobscuranceshader()
{
    string opts;
    int optslen = 0;

    if(aoreduce) opts[optslen++] = 'r';
    bool linear = hasTRG && hasTF && (aoreducedepth && (aoreduce || aoreducedepth > 1));
    if(linear) opts[optslen++] = 'l';
    opts[optslen] = '\0';

    defformatstring(name)("ambientobscurance%s%d", opts, aotaps);
    return generateshader(name, "ambientobscuranceshader(\"%s\", %d)", opts, aotaps);
}

void loadaoshaders()
{
    ambientobscuranceshader = loadambientobscuranceshader();
}

void clearaoshaders()
{
    ambientobscuranceshader = NULL;
}

void setupao(int w, int h)
{
    w >>= aoreduce;
    h >>= aoreduce;

    if(w == aow && h == aoh) return;

    aow = w;
    aoh = h;

    if(!aonoisetex) glGenTextures(1, &aonoisetex);
    bvec *noise = new bvec[(1<<aonoise)*(1<<aonoise)];
    loopk((1<<aonoise)*(1<<aonoise)) noise[k] = bvec(vec(rndscale(2)-1, rndscale(2)-1, 0).normalize());
    createtexture(aonoisetex, 1<<aonoise, 1<<aonoise, noise, 0, 0, GL_RGB, GL_TEXTURE_2D);
    delete[] noise;

    loopi(2)
    {
        if(!aotex[i]) glGenTextures(1, &aotex[i]);
        if(!aofbo[i]) glGenFramebuffers_(1, &aofbo[i]);
        createtexture(aotex[i], w, h, NULL, 3, 1, aobilateral && aopackdepth && hasTRG && hasTF ? GL_RG16F : GL_RGBA8 , GL_TEXTURE_RECTANGLE_ARB);
        glBindFramebuffer_(GL_FRAMEBUFFER_EXT, aofbo[i]);
        glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_ARB, aotex[i], 0);
        if(glCheckFramebufferStatus_(GL_FRAMEBUFFER_EXT) != GL_FRAMEBUFFER_COMPLETE_EXT)
            fatal("failed allocating AO buffer!");
    }

    if(hasTRG && hasTF && aoreducedepth && (aoreduce || aoreducedepth > 1))
    {
        if(!aotex[2]) glGenTextures(1, &aotex[2]);
        if(!aofbo[2]) glGenFramebuffers_(1, &aofbo[2]);
        createtexture(aotex[2], w, h, NULL, 3, 0, GL_R16F, GL_TEXTURE_RECTANGLE_ARB);
        glBindFramebuffer_(GL_FRAMEBUFFER_EXT, aofbo[2]);
        glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_ARB, aotex[2], 0);
        if(glCheckFramebufferStatus_(GL_FRAMEBUFFER_EXT) != GL_FRAMEBUFFER_COMPLETE_EXT)
            fatal("failed allocating AO buffer!");
    }

    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, 0);

    loadaoshaders();
    loadbilateralshaders();
}

void cleanupao()
{
    loopi(3) if(aofbo[i]) { glDeleteFramebuffers_(1, &aofbo[i]); aofbo[i] = 0; }
    loopi(3) if(aotex[i]) { glDeleteTextures(1, &aotex[i]); aotex[i] = 0; }
    if(aonoisetex) { glDeleteTextures(1, &aonoisetex); aonoisetex = 0; }
    aow = bloomh = -1;
    
    clearaoshaders();
    clearbilateralshaders();
}

void viewao()
{
    if(!ao) return;
    int w = min(screen->w, screen->h)/2, h = (w*screen->h)/screen->w;
    rectshader->set();
    glColor3f(1, 1, 1);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, aotex[0]);
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(0, aoh); glVertex2i(0, 0);
    glTexCoord2f(aow, aoh); glVertex2i(w, 0);
    glTexCoord2f(0, 0); glVertex2i(0, h);
    glTexCoord2f(aow, 0); glVertex2i(w, h);
    glEnd();
    notextureshader->set();
}

#include "AreaTex.h"
#include "SearchTex.h"

extern int smaaquality, hdrprec, gdepthformat, gstencil, gdepthstencil, glineardepth;

static Shader *smaalumaedgeshader = NULL, *smaacoloredgeshader = NULL, *smaablendweightshader = NULL, *smaaneighborhoodshader = NULL;

void loadsmaashaders()
{
    defformatstring(lumaedgename)("SMAALumaEdgeDetection%d", smaaquality);
    defformatstring(coloredgename)("SMAAColorEdgeDetection%d", smaaquality);
    defformatstring(blendweightname)("SMAABlendingWeightCalculation%d", smaaquality);
    defformatstring(neighborhoodname)("SMAANeighborhoodBlending%d", smaaquality);
    smaalumaedgeshader = lookupshaderbyname(lumaedgename);
    smaacoloredgeshader = lookupshaderbyname(coloredgename);
    smaablendweightshader = lookupshaderbyname(blendweightname);
    smaaneighborhoodshader = lookupshaderbyname(neighborhoodname);
    if(smaalumaedgeshader && smaacoloredgeshader && smaablendweightshader && smaaneighborhoodshader) return;
    generateshader(NULL, "smaashaders(%d)", smaaquality);
    smaalumaedgeshader = lookupshaderbyname(lumaedgename);
    if(!smaalumaedgeshader) smaalumaedgeshader = nullshader;
    smaacoloredgeshader = lookupshaderbyname(coloredgename);
    if(!smaacoloredgeshader) smaacoloredgeshader = nullshader;
    smaablendweightshader = lookupshaderbyname(blendweightname);
    if(!smaablendweightshader) smaablendweightshader = nullshader;
    smaaneighborhoodshader = lookupshaderbyname(neighborhoodname);
    if(!smaaneighborhoodshader) smaaneighborhoodshader = nullshader;
}

void clearsmaashaders()
{
    smaalumaedgeshader = NULL;
    smaacoloredgeshader = NULL;
    smaablendweightshader = NULL;
    smaaneighborhoodshader = NULL;
}

void setupsmaa()
{
    if(!smaaareatex) glGenTextures(1, &smaaareatex);
    if(!smaasearchtex) glGenTextures(1, &smaasearchtex);
    createtexture(smaaareatex, AREATEX_WIDTH, AREATEX_HEIGHT, areaTexBytes, 3, 1, GL_LUMINANCE_ALPHA, GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, false);
    createtexture(smaasearchtex, SEARCHTEX_WIDTH, SEARCHTEX_HEIGHT, searchTexBytes, 3, 0, GL_LUMINANCE, GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, false);
    loopi(3) 
    {
        if(!smaafbo[i]) glGenFramebuffers_(1, &smaafbo[i]);
        glBindFramebuffer_(GL_FRAMEBUFFER_EXT, smaafbo[i]);
        GLuint tex = 0;
        switch(i)
        {
            case 0: tex = gcolortex; break;
            case 1: tex = gnormaltex; break;
            case 2: tex = gglowtex; break;
        }
        glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_ARB, tex, 0);
        if(i > 0)
        {
            if(gdepthformat)
            {
                glFramebufferRenderbuffer_(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, gdepthrb);
                if(gdepthstencil && hasDS) glFramebufferRenderbuffer_(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, gdepthrb);
            }
            else
            {
                glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_RECTANGLE_ARB, gdepthtex, 0);
                if(gdepthstencil && hasDS) glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_TEXTURE_RECTANGLE_ARB, gdepthtex, 0);
                else if(gstencil) glFramebufferRenderbuffer_(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, gstencilrb);
            }
        }
        if(glCheckFramebufferStatus_(GL_FRAMEBUFFER_EXT) != GL_FRAMEBUFFER_COMPLETE_EXT)
            fatal("failed allocating SMAA buffer!");
    }
    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, 0);

    loadsmaashaders();
}

void cleanupsmaa()
{
    if(smaaareatex) { glDeleteTextures(1, &smaaareatex); smaaareatex = 0; }
    if(smaasearchtex) { glDeleteTextures(1, &smaasearchtex); smaasearchtex = 0; }
    loopi(3) if(smaafbo[i]) { glDeleteFramebuffers_(1, &smaafbo[i]); smaafbo[i] = 0; }

    clearsmaashaders();
}

VARFP(smaa, 0, 0, 1, cleanupsmaa());
VARFP(smaaquality, 0, 2, 3, cleanupsmaa());
VARP(smaacoloredge, 0, 0, 1);
VAR(smaadepthmask, 0, 1, 1);
VAR(smaastencil, 0, 1, 1);
VAR(debugsmaa, 0, 0, 5);

void viewsmaa()
{
    int w = min(screen->w, screen->h)*1.0f, h = (w*screen->h)/screen->w, tw = gw, th = gh;
    rectshader->set();
    glColor3f(1, 1, 1);
    switch(debugsmaa)
    {
        case 1: glBindTexture(GL_TEXTURE_RECTANGLE_ARB, gcolortex); break;
        case 2: glBindTexture(GL_TEXTURE_RECTANGLE_ARB, gnormaltex); break;
        case 3: glBindTexture(GL_TEXTURE_RECTANGLE_ARB, gglowtex); break;
        case 4: glBindTexture(GL_TEXTURE_RECTANGLE_ARB, smaaareatex); tw = AREATEX_WIDTH; th = AREATEX_HEIGHT; break;
        case 5: glBindTexture(GL_TEXTURE_RECTANGLE_ARB, smaasearchtex); tw = SEARCHTEX_WIDTH; th = SEARCHTEX_HEIGHT; break;
        
    }
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(0, th); glVertex2i(0, 0);
    glTexCoord2f(tw, th); glVertex2i(w, 0);
    glTexCoord2f(0, 0); glVertex2i(0, h);
    glTexCoord2f(tw, 0); glVertex2i(w, h);
    glEnd();
    notextureshader->set();
}

VAR(gdepthformat, 1, 0, 0);

void maskgbuffer(const char *mask)
{
    GLenum drawbufs[4];
    int numbufs = 0;
    while(*mask) switch(*mask++)
    {
        case 'c': drawbufs[numbufs++] =  GL_COLOR_ATTACHMENT0_EXT; break;
        case 'n': drawbufs[numbufs++] =  GL_COLOR_ATTACHMENT1_EXT; break;
        case 'g': drawbufs[numbufs++] =  GL_COLOR_ATTACHMENT2_EXT; break;
        case 'd': if(gdepthformat) drawbufs[numbufs++] =  GL_COLOR_ATTACHMENT3_EXT; break;
    }
    glDrawBuffers_(numbufs, drawbufs);
}

void initgbuffer()
{
    if(glineardepth >= 2 && (!hasAFBO || !hasTF || !hasTRG)) gdepthformat = 1;
    else gdepthformat = glineardepth;
}

void setupgbuffer(int w, int h)
{
    if(gw == w && gh == h) return;

    gw = w;
    gh = h;

    if(!gdepthtex) glGenTextures(1, &gdepthtex);
    if(!gcolortex) glGenTextures(1, &gcolortex);
    if(!gnormaltex) glGenTextures(1, &gnormaltex);
    if(!gglowtex) glGenTextures(1, &gglowtex);
    if(!gfbo) glGenFramebuffers_(1, &gfbo);
    
    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, gfbo);

    maskgbuffer("cngd");

    static const GLenum depthformats[] = { GL_RGBA8, GL_R16F, GL_R32F };
    GLenum depthformat = gdepthformat ? depthformats[gdepthformat-1] : (gdepthstencil && hasDS ? GL_DEPTH24_STENCIL8_EXT : GL_DEPTH_COMPONENT);
    createtexture(gdepthtex, gw, gh, NULL, 3, 0, depthformat, GL_TEXTURE_RECTANGLE_ARB);

    createtexture(gcolortex, gw, gh, NULL, 3, 0, GL_RGBA8, GL_TEXTURE_RECTANGLE_ARB);
    createtexture(gnormaltex, gw, gh, NULL, 3, 0, GL_RGBA8, GL_TEXTURE_RECTANGLE_ARB);
    createtexture(gglowtex, gw, gh, NULL, 3, 0, GL_RGBA8, GL_TEXTURE_RECTANGLE_ARB);
 
    if(gdepthformat)
    {
        if(!gdepthrb) glGenRenderbuffers_(1, &gdepthrb);
        glBindRenderbuffer_(GL_RENDERBUFFER_EXT, gdepthrb);
        glRenderbufferStorage_(GL_RENDERBUFFER_EXT, gdepthstencil && hasDS ? GL_DEPTH24_STENCIL8_EXT : GL_DEPTH_COMPONENT, gw, gh);
        glBindRenderbuffer_(GL_RENDERBUFFER_EXT, 0);
    } 
    else if(gstencil && (!gdepthstencil || !hasDS))
    {
        if(!gstencilrb) glGenRenderbuffers_(1, &gstencilrb);
        glBindRenderbuffer_(GL_RENDERBUFFER_EXT, gstencilrb);
        glRenderbufferStorage_(GL_RENDERBUFFER_EXT, GL_STENCIL_INDEX8_EXT, gw, gh);
        glBindRenderbuffer_(GL_RENDERBUFFER_EXT, 0);
    }
 
    if(gdepthformat)
    {
        glFramebufferRenderbuffer_(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, gdepthrb);
        if(gdepthstencil && hasDS) glFramebufferRenderbuffer_(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, gdepthrb);
        glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT3_EXT, GL_TEXTURE_RECTANGLE_ARB, gdepthtex, 0);
    }
    else
    {
        glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_RECTANGLE_ARB, gdepthtex, 0);
        if(gdepthstencil && hasDS) glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_TEXTURE_RECTANGLE_ARB, gdepthtex, 0);
        else if(gstencil) glFramebufferRenderbuffer_(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, gstencilrb);
    }
    glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_ARB, gcolortex, 0);
    glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_RECTANGLE_ARB, gnormaltex, 0);
    glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT2_EXT, GL_TEXTURE_RECTANGLE_ARB, gglowtex, 0);

    if(glCheckFramebufferStatus_(GL_FRAMEBUFFER_EXT) != GL_FRAMEBUFFER_COMPLETE_EXT) 
        fatal("failed allocating g-buffer!");

    if(!hdrtex) glGenTextures(1, &hdrtex);

    if(!hdrfbo) glGenFramebuffers_(1, &hdrfbo);

    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, hdrfbo);

    hdrformat = hdr ? (hdrprec >= 3 && hasTF ? GL_RGB16F_ARB : (hdrprec >= 2 && hasPF ? GL_R11F_G11F_B10F_EXT : (hdrprec >= 1 ? GL_RGB10 : GL_RGB))) : GL_RGB;
    hdrfloat = floatformat(hdrformat);
    createtexture(hdrtex, gw, gh, NULL, 3, 1, hdrformat, GL_TEXTURE_RECTANGLE_ARB);

    if(gdepthformat)
    {
        glFramebufferRenderbuffer_(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, gdepthrb);
        if(gdepthstencil && hasDS) glFramebufferRenderbuffer_(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, gdepthrb);
        glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT3_EXT, GL_TEXTURE_RECTANGLE_ARB, gdepthtex, 0);
    }
    else
    {
        glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_RECTANGLE_ARB, gdepthtex, 0);
        if(gdepthstencil && hasDS) glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_TEXTURE_RECTANGLE_ARB, gdepthtex, 0);
        else if(gstencil) glFramebufferRenderbuffer_(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, gstencilrb);
    }

    glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_ARB, hdrtex, 0);

    if(glCheckFramebufferStatus_(GL_FRAMEBUFFER_EXT) != GL_FRAMEBUFFER_COMPLETE_EXT)
        fatal("failed allocating HDR buffer!");

    hdrclear = 3;

    if(!refracttex) glGenTextures(1, &refracttex);
    if(!refractfbo) glGenFramebuffers_(1, &refractfbo);

    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, refractfbo);

    createtexture(refracttex, gw, gh, NULL, 3, 0, GL_RGBA8, GL_TEXTURE_RECTANGLE_ARB);

    if(gdepthformat)
    {
        glFramebufferRenderbuffer_(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, gdepthrb);
        if(gdepthstencil && hasDS) glFramebufferRenderbuffer_(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, gdepthrb);
        glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT3_EXT, GL_TEXTURE_RECTANGLE_ARB, gdepthtex, 0);
    }
    else
    {
        glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_RECTANGLE_ARB, gdepthtex, 0);
        if(gdepthstencil && hasDS) glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_TEXTURE_RECTANGLE_ARB, gdepthtex, 0);
        else if(gstencil) glFramebufferRenderbuffer_(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, gstencilrb);
    }

    glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_ARB, refracttex, 0);

    if(glCheckFramebufferStatus_(GL_FRAMEBUFFER_EXT) != GL_FRAMEBUFFER_COMPLETE_EXT)
        fatal("failed allocating refraction buffer!");
 
    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, 0);

    cleanupbloom();
    cleanupao();
    cleanupsmaa();
}

void cleanupgbuffer()
{
    if(gfbo) { glDeleteFramebuffers_(1, &gfbo); gfbo = 0; }
    if(gdepthtex) { glDeleteTextures(1, &gdepthtex); gdepthtex = 0; }
    if(gcolortex) { glDeleteTextures(1, &gcolortex); gcolortex = 0; }
    if(gnormaltex) { glDeleteTextures(1, &gnormaltex); gnormaltex = 0; }
    if(gglowtex) { glDeleteTextures(1, &gglowtex); gglowtex = 0; }
    if(gstencilrb) { glDeleteRenderbuffers_(1, &gstencilrb); gstencilrb = 0; }
    if(gdepthrb) { glDeleteRenderbuffers_(1, &gdepthrb); gdepthrb = 0; }
    if(hdrfbo) { glDeleteFramebuffers_(1, &hdrfbo); hdrfbo = 0; }
    if(hdrtex) { glDeleteTextures(1, &hdrtex); hdrtex = 0; }
    if(refractfbo) { glDeleteFramebuffers_(1, &refractfbo); refractfbo = 0; }
    if(refracttex) { glDeleteTextures(1, &refracttex); refracttex = 0; }
    gw = gh = -1;
}

VAR(debugdepth, 0, 0, 1);

void viewdepth()
{
    int w = min(screen->w, screen->h)/2, h = (w*screen->h)/screen->w;
    rectshader->set();
    glColor3f(1, 1, 1);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, gdepthtex);
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(0, gh); glVertex2i(0, 0);
    glTexCoord2f(gw, gh); glVertex2i(w, 0);
    glTexCoord2f(0, 0); glVertex2i(0, h);
    glTexCoord2f(gw, 0); glVertex2i(w, h);
    glEnd();
    notextureshader->set();
}

VAR(debugrefract, 0, 0, 1);

void viewrefract()
{
    int w = min(screen->w, screen->h)/2, h = (w*screen->h)/screen->w;
    rectshader->set();
    glColor3f(1, 1, 1);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, refracttex);
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(0, gh); glVertex2i(0, 0);
    glTexCoord2f(gw, gh); glVertex2i(w, 0);
    glTexCoord2f(0, 0); glVertex2i(0, h);
    glTexCoord2f(gw, 0); glVertex2i(w, h);
    glEnd();
    notextureshader->set();
}

VARF(gstencil, 0, 0, 1, cleanupgbuffer());
VARF(gdepthstencil, 0, 1, 1, cleanupgbuffer());
VARF(glineardepth, 0, 0, 3, initwarning("g-buffer setup"))
FVAR(bloomthreshold, 1e-3f, 0.8f, 1e3f);
FVARP(bloomscale, 0, 1.0f, 1e3f);
VARP(bloomblur, 0, 7, 7);
VARP(bloomiter, 0, 0, 4);
FVARP(bloomsigma, 0.005f, 0.5f, 2.0f);
VARFP(bloomsize, 6, 9, 11, cleanupbloom());
VARFP(bloomprec, 0, 2, 3, cleanupbloom());
FVAR(hdraccumscale, 0, 0.98f, 1);
VAR(hdraccummillis, 1, 33, 1000);
VAR(hdrreduce, 0, 2, 2);
VARFP(hdr, 0, 1, 1, cleanupgbuffer());
VARFP(hdrprec, 0, 2, 3, cleanupgbuffer());
FVARP(hdrgamma, 1e-3f, 2.2f, 1e3f);
FVARR(hdrbright, 1e-4f, 1.0f, 1e4f);
FVAR(hdrsaturate, 1e-3f, 0.8f, 1e3f);

float ldrscale = 1.0f, ldrscaleb = 1.0f/255;

#define RH_MAXSPLITS 4

GLuint rhtex[3] = { 0, 0, 0 }, rhfbo = 0;
GLuint rsmdepthtex = 0, rsmcolortex = 0, rsmnormaltex = 0, rsmfbo = 0;

extern int rhgrid, rhsplits, rhborder, rhprec, rhtaps, rsmprec, rsmsize;

static Shader *radiancehintsshader = NULL;

Shader *loadradiancehintsshader()
{
    defformatstring(name)("radiancehints%d", rhtaps);
    return generateshader(name, "radiancehintsshader(%d)", rhtaps);
}

void loadrhshaders()
{
    radiancehintsshader = loadradiancehintsshader();
}

void clearrhshaders()
{
    radiancehintsshader = NULL;
}

void setupradiancehints()
{
    loopi(3) if(!rhtex[i]) glGenTextures(1, &rhtex[i]);

    if(!rhfbo) glGenFramebuffers_(1, &rhfbo);

    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, rhfbo);

    GLenum rhformat = hasTF && rhprec >= 1 ? GL_RGBA16F_ARB : GL_RGBA8;

    loopi(3)
    {
        create3dtexture(rhtex[i], rhgrid+2*rhborder, rhgrid+2*rhborder, (rhgrid+2*rhborder)*rhsplits, NULL, 7, 1, rhformat);
        if(rhborder)
        {
            glTexParameteri(GL_TEXTURE_3D_EXT, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_3D_EXT, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_3D_EXT, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
            GLfloat border[4] = { 0.5f, 0.5f, 0.5f, 0 };
            glTexParameterfv(GL_TEXTURE_3D_EXT, GL_TEXTURE_BORDER_COLOR, border);
        }
        glFramebufferTexture3D_(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT + i, GL_TEXTURE_3D_EXT, rhtex[i], 0, 0);
    }

    static const GLenum drawbufs[3] = { GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_COLOR_ATTACHMENT2_EXT };
    glDrawBuffers_(3, drawbufs);

    if(glCheckFramebufferStatus_(GL_FRAMEBUFFER_EXT) != GL_FRAMEBUFFER_COMPLETE_EXT)
        fatal("failed allocating radiance hints buffer!");

    if(!rsmdepthtex) glGenTextures(1, &rsmdepthtex);
    if(!rsmcolortex) glGenTextures(1, &rsmcolortex);
    if(!rsmnormaltex) glGenTextures(1, &rsmnormaltex);

    if(!rsmfbo) glGenFramebuffers_(1, &rsmfbo);

    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, rsmfbo);

    GLenum rsmformat = rsmprec >= 3 && hasTF ? GL_RGB16F_ARB : (rsmprec >= 2 && hasPF ? GL_R11F_G11F_B10F_EXT : (rsmprec >= 1 ? GL_RGB10 : GL_RGBA8));

    createtexture(rsmdepthtex, rsmsize, rsmsize, NULL, 3, 0, GL_DEPTH_COMPONENT16_ARB, GL_TEXTURE_RECTANGLE_ARB);
    createtexture(rsmcolortex, rsmsize, rsmsize, NULL, 3, 0, rsmformat, GL_TEXTURE_RECTANGLE_ARB);
    createtexture(rsmnormaltex, rsmsize, rsmsize, NULL, 3, 0, rsmformat, GL_TEXTURE_RECTANGLE_ARB);

    glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_RECTANGLE_ARB, rsmdepthtex, 0);
    glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_ARB, rsmcolortex, 0);
    glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_RECTANGLE_ARB, rsmnormaltex, 0);

    glDrawBuffers_(2, drawbufs);

    if(glCheckFramebufferStatus_(GL_FRAMEBUFFER_EXT) != GL_FRAMEBUFFER_COMPLETE_EXT)
        fatal("failed allocating RSM buffer!");

    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, 0);

    loadrhshaders();
}
   
void cleanupradiancehints()
{
    loopi(3) if(rhtex[i]) { glDeleteTextures(1, &rhtex[i]); rhtex[i] = 0; }
    if(rhfbo) { glDeleteFramebuffers_(1, &rhfbo); rhfbo = 0; }
    if(rsmdepthtex) { glDeleteTextures(1, &rsmdepthtex); rsmdepthtex = 0; }
    if(rsmcolortex) { glDeleteTextures(1, &rsmcolortex); rsmcolortex = 0; }
    if(rsmnormaltex) { glDeleteTextures(1, &rsmnormaltex); rsmnormaltex = 0; }
    if(rsmfbo) { glDeleteFramebuffers_(1, &rsmfbo); rsmfbo = 0; }

    clearrhshaders();
}

VARF(rhsplits, 1, 2, RH_MAXSPLITS, { cleardeferredlightshaders(); cleanupradiancehints(); });
VARF(rhborder, 0, 1, 1, cleanupradiancehints());
VARF(rsmsize, 64, 384, 2048, cleanupradiancehints());
VAR(rhfarplane, 64, 1024, 16384);
FVAR(rsmpradiustweak, 1e-3f, 1, 1e3f);
FVAR(rhpradiustweak, 1e-3f, 1, 1e3f);
FVAR(rsmdepthrange, 0, 1024, 1e6f);
FVAR(rsmdepthmargin, 0, 0.1f, 1e3f);
VARF(rhprec, 0, 0, 1, cleanupradiancehints());
VARF(rsmprec, 0, 0, 3, cleanupradiancehints());
FVAR(rhnudge, 0, 0.5f, 4);
FVAR(rhsplitweight, 0.20f, 0.6f, 0.95f);
VARF(rhgrid, 3, 27, 128, cleanupradiancehints());
FVAR(rsmspread, 0, 0.2f, 1);
VAR(rhclipgrid, 0, 1, 1);
VAR(rsmcull, 0, 1, 1);
VARFP(rhtaps, 0, 20, 32, cleanupradiancehints());
VARFR(gidist, 0, 384, 1024, { cleardeferredlightshaders(); if(!gidist) cleanupradiancehints(); });
FVARFR(giscale, 0, 1.5f, 1e3f, { cleardeferredlightshaders(); if(!giscale) cleanupradiancehints(); });
VARFP(gi, 0, 1, 1, { cleardeferredlightshaders(); cleanupradiancehints(); });

VAR(debugrsm, 0, 0, 2);
void viewrsm()
{
    int w = min(screen->w, screen->h)/2, h = (w*screen->h)/screen->w;
    rectshader->set();
    glColor3f(1, 1, 1);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, debugrsm == 2 ? rsmnormaltex : rsmcolortex);
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(0, 0); glVertex2i(screen->w-w, screen->h-h);
    glTexCoord2f(rsmsize, 0); glVertex2i(screen->w, screen->h-h);
    glTexCoord2f(0, rsmsize); glVertex2i(screen->w-w, screen->h);
    glTexCoord2f(rsmsize, rsmsize); glVertex2i(screen->w, screen->h);
    glEnd();
    notextureshader->set();
}

VAR(debugrh, 0, 0, RH_MAXSPLITS*(128 + 2));
void viewrh()
{
    int w = min(screen->w, screen->h)/2, h = (w*screen->h)/screen->w;
    SETSHADER(tex3d);
    glColor3f(1, 1, 1);
    glBindTexture(GL_TEXTURE_3D, rhtex[1]);
    float z = (debugrh-1+0.5f)/float((rhgrid+2*rhborder)*rhsplits);
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord3f(0, 0, z); glVertex2i(screen->w-w, screen->h-h);
    glTexCoord3f(1, 0, z); glVertex2i(screen->w, screen->h-h);
    glTexCoord3f(0, 1, z); glVertex2i(screen->w-w, screen->h);
    glTexCoord3f(1, 1, z); glVertex2i(screen->w, screen->h);
    glEnd();
    notextureshader->set();
}
 
#define SHADOWATLAS_SIZE 4096

PackNode shadowatlaspacker(0, 0, SHADOWATLAS_SIZE, SHADOWATLAS_SIZE);

enum { L_NOSHADOW = 1<<0, L_NODYNSHADOW = 1<<1 };

struct lightinfo
{
    float sx1, sy1, sx2, sy2, sz1, sz2;
    int shadowmap, flags;
    vec o, color;
    float radius;
    vec dir, spotx, spoty;
    int spot;
    float dist;
    occludequery *query;
};

struct shadowcachekey
{
    vec o;
    float radius;
    vec dir;
    int spot;

    shadowcachekey() {}
    shadowcachekey(const lightinfo &l) : o(l.o), radius(l.radius), dir(l.dir), spot(l.spot) {}
};

static inline uint hthash(const shadowcachekey &k)
{
    return hthash(k.o);
}

static inline bool htcmp(const shadowcachekey &x, const shadowcachekey &y)
{
    return x.o == y.o && x.radius == y.radius && x.dir == y.dir && x.spot == y.spot;
}

struct shadowcacheval;

struct shadowmapinfo
{
    ushort x, y, size, sidemask;
    int light;
    shadowcacheval *cached;
};

struct shadowcacheval
{
    ushort x, y, size, sidemask;

    shadowcacheval() {}
    shadowcacheval(const shadowmapinfo &sm) : x(sm.x), y(sm.y), size(sm.size), sidemask(sm.sidemask) {}
};

struct shadowcache : hashtable<shadowcachekey, shadowcacheval>
{
    shadowcache() : hashtable<shadowcachekey, shadowcacheval>(256) {}

    void reset()
    {
        clear();
    } 
};

extern int smcache, smfilter, smgather;

#define SHADOWCACHE_EVICT 2

GLuint shadowatlastex = 0, shadowatlasfbo = 0;
GLenum shadowatlastarget = GL_NONE;
shadowcache shadowcache;
bool shadowcachefull = false;
int evictshadowcache = 0;

static inline void setsmnoncomparemode() // use texture gather
{
    glTexParameteri(shadowatlastarget, GL_TEXTURE_COMPARE_MODE_ARB, GL_NONE);
    glTexParameteri(shadowatlastarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(shadowatlastarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

static inline void setsmcomparemode() // use embedded shadow cmp
{
    glTexParameteri(shadowatlastarget, GL_TEXTURE_COMPARE_MODE_ARB, GL_COMPARE_R_TO_TEXTURE_ARB);
    glTexParameteri(shadowatlastarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(shadowatlastarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

static inline bool usegatherforsm() { return smfilter > 1 && smgather && (hasTG || hasT4); }

void viewshadowatlas()
{
    int w = min(screen->w, screen->h)/2, h = (w*screen->h)/screen->w;
    float tw = 1, th = 1;
    if(shadowatlastarget == GL_TEXTURE_RECTANGLE_ARB)
    {
        tw = SHADOWATLAS_SIZE;
        th = SHADOWATLAS_SIZE;
        rectshader->set();
    }
    else defaultshader->set();
    glColor3f(1, 1, 1);
    glBindTexture(shadowatlastarget, shadowatlastex);
    if(!usegatherforsm()) setsmnoncomparemode(); // "normal" mode
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(0, 0); glVertex2i(screen->w-w, screen->h-h);
    glTexCoord2f(tw, 0); glVertex2i(screen->w, screen->h-h);
    glTexCoord2f(0, th); glVertex2i(screen->w-w, screen->h);
    glTexCoord2f(tw, th); glVertex2i(screen->w, screen->h);
    glEnd();
    if(!usegatherforsm()) setsmcomparemode(); // "gather" mode basically
    notextureshader->set();
}
VAR(debugshadowatlas, 0, 0, 1);

void setupshadowatlas()
{
    if(!shadowatlastex) glGenTextures(1, &shadowatlastex);

    shadowatlastarget = usegatherforsm() ? GL_TEXTURE_2D : GL_TEXTURE_RECTANGLE_ARB;
    createtexture(shadowatlastex, SHADOWATLAS_SIZE, SHADOWATLAS_SIZE, NULL, 3, 1, GL_DEPTH_COMPONENT16_ARB, shadowatlastarget);
    glTexParameteri(shadowatlastarget, GL_TEXTURE_COMPARE_MODE_ARB, GL_COMPARE_R_TO_TEXTURE_ARB);
    glTexParameteri(shadowatlastarget, GL_TEXTURE_COMPARE_FUNC_ARB, GL_LEQUAL);
    glTexParameteri(shadowatlastarget, GL_DEPTH_TEXTURE_MODE_ARB, GL_LUMINANCE);

    if(!shadowatlasfbo) glGenFramebuffers_(1, &shadowatlasfbo);

    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, shadowatlasfbo);

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    glFramebufferTexture2D_(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, shadowatlastarget, shadowatlastex, 0);

    if(glCheckFramebufferStatus_(GL_FRAMEBUFFER_EXT) != GL_FRAMEBUFFER_COMPLETE_EXT)
        fatal("failed allocating shadow atlas!");

    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, 0);
}

void cleanupshadowatlas()
{
    if(shadowatlastex) { glDeleteTextures(1, &shadowatlastex); shadowatlastex = 0; }
    if(shadowatlasfbo) { glDeleteFramebuffers_(1, &shadowatlasfbo); shadowatlasfbo = 0; }
    clearshadowcache();
}

const float cubeshadowviewmatrix[6][16] =
{
    // sign-preserving cubemap projections
    { // +X
         0, 0,-1, 0,
         0, 1, 0, 0,
         1, 0, 0, 0,
         0, 0, 0, 1,
    },
    { // -X
         0, 0, 1, 0,
         0, 1, 0, 0,
         1, 0, 0, 0,
         0, 0, 0, 1,
    },
    { // +Y
         1, 0, 0, 0,
         0, 0,-1, 0,
         0, 1, 0, 0,
         0, 0, 0, 1,
    },
    { // -Y
         1, 0, 0, 0,
         0, 0, 1, 0,
         0, 1, 0, 0,
         0, 0, 0, 1,
    },
    { // +Z
         1, 0, 0, 0,
         0, 1, 0, 0,
         0, 0,-1, 0,
         0, 0, 0, 1,
    },
    { // -Z
         1, 0, 0, 0,
         0, 1, 0, 0,
         0, 0, 1, 0,
         0, 0, 0, 1,
    },
};

const float tetrashadowviewmatrix[4][16] =
{
    { // +1, +1, +1
        1, 0,-1, 0,
        0, 1,-1, 0,
        0, 0,-1, 0,
        0, 0, 0, 1
    },
    { // -1, -1, +1
        1, 0, 1, 0,
        0, 1, 1, 0,
        0, 0,-1, 0,
        0, 0, 0, 1
    },
    { // +1, -1, -1
        1, 0,-1, 0,
        0, 1, 1, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    },
    { // -1, +1, -1
        1, 0, 1, 0,
        0, 1,-1, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    }
};

FVAR(smpolyfactor, -1e3f, 1, 1e3f);
FVAR(smpolyoffset, -1e3f, 0, 1e3f);
FVAR(smbias, -1e6f, 0.01f, 1e6f);
FVAR(smprec, 1e-3f, 1, 1e3f);
FVAR(smtetraprec, 1e-3f, SQRT3, 1e3f);
FVAR(smcubeprec, 1e-3f, 1, 1e3f);
FVAR(smspotprec, 1e-3f, 1, 1e3f);

VAR(smsidecull, 0, 1, 1);
VAR(smviscull, 0, 1, 1);
VAR(smborder, 0, 3, 16);
VAR(smminradius, 0, 16, 10000);
VAR(smminsize, 1, 96, 1024);
VAR(smmaxsize, 1, 384, 1024);
//VAR(smmaxsize, 1, 4096, 4096);
VAR(smused, 1, 0, 0);
VAR(smquery, 0, 1, 1);
VARFP(smtetra, 0, 0, 1, { cleardeferredlightshaders(); clearshadowcache(); });
VAR(smtetraclip, 0, 1, 1);
VAR(smtetraclear, 0, 1, 1);
FVAR(smtetraborder, 0, 0, 1e3f);
VAR(smcullside, 0, 1, 1);
VARF(smcache, 0, 1, 2, cleanupshadowatlas());
VARFP(smfilter, 0, 2, 2, { cleardeferredlightshaders(); cleanupshadowatlas(); });
VARFP(smgather, 0, 0, 1, { cleardeferredlightshaders(); cleanupshadowatlas(); });
VAR(smnoshadow, 0, 0, 2);
VAR(lighttilesused, 1, 0, 0);

int shadowmapping = 0;

plane smtetraclipplane;

vector<lightinfo> lights;
vector<int> lightorder;
vector<int> lighttiles[LIGHTTILE_H][LIGHTTILE_W];
vector<shadowmapinfo> shadowmaps;

void clearshadowcache()
{
    shadowmaps.setsize(0);
}

static shadowmapinfo *addshadowmap(ushort x, ushort y, int size, int &idx)
{
    idx = shadowmaps.length();
    shadowmapinfo *sm = &shadowmaps.add();
    sm->x = x;
    sm->y = y;
    sm->size = size;
    sm->light = -1;
    sm->sidemask = 0;
    sm->cached = NULL;
    return sm;
}

static const uint csmmaxsplitn = 8, csmminsplitn = 1;
VARF(csmmaxsize, 256, 768, 2048, clearshadowcache());
VARF(csmsplitn, csmminsplitn, 3, csmmaxsplitn, { cleardeferredlightshaders(); clearshadowcache(); });
FVAR(csmsplitweight, 0.20f, 0.75f, 0.95f);
VARF(csmshadowmap, 0, 1, 1, cleardeferredlightshaders());

// cascaded shadow maps
struct cascaded_shadow_map
{
    struct splitinfo
    {
        float nearplane;     // split distance to near plane
        float farplane;      // split distance to farplane
        glmatrixf proj;      // one projection per split
        vec scale, offset;   // scale and offset of the projection
        int idx;             // shadowmapinfo indices
        vec center, bounds;  // max extents of shadowmap in sunlight model space
        plane cull[4];       // world space culling planes of the split's projected sides
    };
    glmatrixf model;                // model view is shared by all splits
    splitinfo splits[csmmaxsplitn]; // per-split parameters
    vec lightview;                  // view vector for light
    void setup();                   // insert shadowmaps for each split frustum if there is sunlight
    void updatesplitdist();         // compute split frustum distances
    void getmodelmatrix();          // compute the shared model matrix
    void getprojmatrix();           // compute each cropped projection matrix
    void gencullplanes();           // generate culling planes for the mvp matrix
    void bindparams();              // bind any shader params necessary for lighting
};

void cascaded_shadow_map::setup()
{
    loopi(csmsplitn)
    {
        ushort smx = USHRT_MAX, smy = USHRT_MAX;
        splits[i].idx = -1;
        if(shadowatlaspacker.insert(smx, smy, csmmaxsize, csmmaxsize))
            addshadowmap(smx, smy, csmmaxsize, splits[i].idx);
    }
    getmodelmatrix();
    getprojmatrix();
    gencullplanes();
}

static float calcfrustumboundsphere(float nearplane, float farplane,  const vec &pos, const vec &view, vec &center)
{
    if(minimapping)
    {
        center = minimapcenter;
        return minimapradius.magnitude();
    }

    float width = tan(fov/2.0f*RAD), height = width / aspect,
          cdist = ((nearplane + farplane)/2)*(1 + width*width + height*height);
    if(cdist <= farplane)
    {
        center = vec(view).mul(cdist).add(pos);
        return vec(width*nearplane, height*nearplane, cdist-nearplane).magnitude();
    }
    else
    {
        center = vec(view).mul(farplane).add(pos);
        return vec(width*farplane, height*farplane, 0).magnitude();
    }
}

VAR(csmfarplane, 64, 1024, 16384);
FVAR(csmpradiustweak, 1e-3f, 1, 1e3f);
FVAR(csmdepthrange, 0, 1024, 1e6f);
FVAR(csmdepthmargin, 0, 0.1f, 1e3f);
VAR(debugcsm, 0, 0, csmmaxsplitn);
FVAR(csmpolyfactor, -1e3f, 2, 1e3f);
FVAR(csmpolyoffset, -1e4f, 0, 1e4f);
FVAR(csmbias, -1e6f, 1e-4f, 1e6f);
VAR(csmcull, 0, 1, 1);

void cascaded_shadow_map::updatesplitdist()
{
    float lambda = csmsplitweight, nd = nearplane, fd = csmfarplane, ratio = fd/nd;
    splits[0].nearplane = nd;
    for(int i = 1; i < csmsplitn; ++i)
    {
        float si = i / float(csmsplitn);
        splits[i].nearplane = lambda*(nd*pow(ratio, si)) + (1-lambda)*(nd + (fd - nd)*si);
        splits[i-1].farplane = splits[i].nearplane * 1.005f;
    }
    splits[csmsplitn-1].farplane = fd;
}

void cascaded_shadow_map::getmodelmatrix()
{
    model = viewmatrix;
    model.rotate_around_x(sunlightpitch*RAD);
    model.rotate_around_z((180-sunlightyaw)*RAD);
}

void cascaded_shadow_map::getprojmatrix()
{
    lightview = vec(sunlightdir).neg();

    // compute the split frustums
    updatesplitdist();

    // find z extent
    float minz = lightview.project_bb(worldmin, worldmax), maxz = lightview.project_bb(worldmax, worldmin), 
          zmargin = max((maxz - minz)*csmdepthmargin, 0.5f*(csmdepthrange - (maxz - minz)));
    minz -= zmargin;
    maxz += zmargin;

    // compute each split projection matrix
    loopi(csmsplitn)
    {
        splitinfo &split = this->splits[i];
        if(split.idx < 0) continue;
        const shadowmapinfo &sm = shadowmaps[split.idx];

        vec c;
        float radius = calcfrustumboundsphere(split.nearplane, split.farplane, camera1->o, camdir, c);

        // compute the projected bounding box of the sphere
        vec tc;
        this->model.transform(c, tc);
        const float pradius = radius * csmpradiustweak, step = (2*pradius) / (sm.size - 2*smborder);
        vec2 offset = vec2(tc).sub(pradius).div(step);
        offset.x = floor(offset.x);
        offset.y = floor(offset.y);
        split.center = vec(vec2(offset).mul(step).add(pradius), -0.5f*(minz + maxz));
        split.bounds = vec(pradius, pradius, 0.5f*(maxz - minz));

        // modify mvp with a scale and offset
        // now compute the update model view matrix for this split
        split.scale = vec(1/step, 1/step, -1/(maxz - minz));
        split.offset = vec(smborder - offset.x, smborder - offset.y, -minz/(maxz - minz));

        split.proj.identity();
        split.proj.scale(2*split.scale.x/sm.size, 2*split.scale.y/sm.size, 2*split.scale.z);
        split.proj.translate(2*split.offset.x/sm.size - 1, 2*split.offset.y/sm.size - 1, 2*split.offset.z - 1);

        const float bias = csmbias * (-512.0f / sm.size) * (split.farplane - split.nearplane) / (splits[0].farplane - splits[0].nearplane);
        split.offset.add(vec(sm.x, sm.y, bias));
    }
}

void cascaded_shadow_map::gencullplanes()
{
    loopi(csmsplitn)
    {
        splitinfo &split = splits[i];
        glmatrixf mvp;
        mvp.mul(split.proj, model);
        vec4 px = mvp.getrow(0), py = mvp.getrow(1), pw = mvp.getrow(3);
        split.cull[0] = plane(vec4(pw).add(px)).normalize(); // left plane
        split.cull[1] = plane(vec4(pw).sub(px)).normalize(); // right plane
        split.cull[2] = plane(vec4(pw).add(py)).normalize(); // bottom plane
        split.cull[3] = plane(vec4(pw).sub(py)).normalize(); // top plane
    }        
}

void cascaded_shadow_map::bindparams()
{
    static GlobalShaderParam splitcenter("splitcenter"), splitbounds("splitbounds"), splitscale("splitscale"), splitoffset("splitoffset");
    vec *splitcenterv = splitcenter.reserve<vec>(csmsplitn),
        *splitboundsv = splitbounds.reserve<vec>(csmsplitn),
        *splitscalev = splitscale.reserve<vec>(csmsplitn),
        *splitoffsetv = splitoffset.reserve<vec>(csmsplitn);
    loopi(csmsplitn)
    {
        cascaded_shadow_map::splitinfo &split = splits[i];
        if(split.idx < 0) continue;
        splitcenterv[i] = split.center;
        splitboundsv[i] = split.bounds;
        splitscalev[i] = split.scale;
        splitoffsetv[i] = split.offset;
    }
    glMatrixMode(GL_TEXTURE);
    glActiveTexture_(GL_TEXTURE1_ARB);
    glLoadMatrixf(model.v);
    glActiveTexture_(GL_TEXTURE0_ARB);
    glMatrixMode(GL_MODELVIEW);
}

cascaded_shadow_map csm;

int calcbbcsmsplits(const ivec &bbmin, const ivec &bbmax)
{
    int mask = (1<<csmsplitn)-1;
    if(!csmcull) return mask;
    loopi(csmsplitn)
    {
        const cascaded_shadow_map::splitinfo &split = csm.splits[i];
        int k; 
        for(k = 0; k < 4; k++)
        {
            const plane &p = split.cull[k];
            ivec omin, omax;
            if(p.x > 0) { omin.x = bbmin.x; omax.x = bbmax.x; } else { omin.x = bbmax.x; omax.x = bbmin.x; }
            if(p.y > 0) { omin.y = bbmin.y; omax.y = bbmax.y; } else { omin.y = bbmax.y; omax.y = bbmin.y; }
            if(p.z > 0) { omin.z = bbmin.z; omax.z = bbmax.z; } else { omin.z = bbmax.z; omax.z = bbmin.z; }
            if(omax.dist(p) < 0) { mask &= ~(1<<i); goto nextsplit; }
            if(omin.dist(p) < 0) goto notinside;
        }
        mask &= (2<<i)-1;
        break;
    notinside:
        while(++k < 4)
        {
            const plane &p = split.cull[k];
            ivec omax(p.x > 0 ? bbmax.x : bbmin.x, p.y > 0 ? bbmax.y : bbmin.y, p.z > 0 ? bbmax.z : bbmin.z);
            if(omax.dist(p) < 0) { mask &= ~(1<<i); break; }
        }
    nextsplit:;
    }
    return mask;
}

int calcspherecsmsplits(const vec &center, float radius)
{
    int mask = (1<<csmsplitn)-1;
    if(!csmcull) return mask;
    loopi(csmsplitn)
    {
        const cascaded_shadow_map::splitinfo &split = csm.splits[i];
        int k;
        for(k = 0; k < 4; k++)
        {
            const plane &p = split.cull[k];
            float dist = p.dist(center);
            if(dist < -radius) { mask &= ~(1<<i); goto nextsplit; }
            if(dist < radius) goto notinside;
        }
        mask &= (2<<i)-1;
        break;
    notinside:
        while(++k < 4)
        {
            const plane &p = split.cull[k];
            if(p.dist(center) < -radius) { mask &= ~(1<<i); break; }
        }
    nextsplit:;
    }
    return mask;
}

struct reflectiveshadowmap
{
    glmatrixf model, proj;
    vec lightview;
    plane cull[4];
    vec scale, offset;
    vec center, bounds;
    void setup();
    void getmodelmatrix();
    void getprojmatrix();
    void gencullplanes();
} rsm;

void reflectiveshadowmap::setup()
{
    getmodelmatrix();
    getprojmatrix();
    gencullplanes();
}

void reflectiveshadowmap::getmodelmatrix()
{
    model = viewmatrix;
    model.rotate_around_x(sunlightpitch*RAD);
    model.rotate_around_z((180-sunlightyaw)*RAD);
}

void reflectiveshadowmap::getprojmatrix()
{
    lightview = vec(sunlightdir).neg();

    // find z extent
    float minz = lightview.project_bb(worldmin, worldmax), maxz = lightview.project_bb(worldmax, worldmin),
          zmargin = max((maxz - minz)*rsmdepthmargin, 0.5f*(rsmdepthrange - (maxz - minz)));
    minz -= zmargin;
    maxz += zmargin;

    vec c;
    float radius = calcfrustumboundsphere(nearplane, rhfarplane, camera1->o, camdir, c);

    // compute the projected bounding box of the sphere
    vec tc;
    model.transform(c, tc);
    const float pradius = (radius + gidist) * rsmpradiustweak, step = (2*pradius) / rsmsize;
    vec2 tcoff = vec2(tc).sub(pradius).div(step);
    tcoff.x = floor(tcoff.x);
    tcoff.y = floor(tcoff.y);
    center = vec(vec2(tcoff).mul(step).add(pradius), -0.5f*(minz + maxz));
    bounds = vec(pradius, pradius, 0.5f*(maxz - minz));

    scale = vec(1/step, 1/step, -1/(maxz - minz));
    offset = vec(-tcoff.x, -tcoff.y, -minz/(maxz - minz));

    proj.identity();
    proj.scale(2*scale.x/rsmsize, 2*scale.y/rsmsize, 2*scale.z);
    proj.translate(2*offset.x/rsmsize - 1, 2*offset.y/rsmsize - 1, 2*offset.z - 1);
}

void reflectiveshadowmap::gencullplanes()
{
    glmatrixf mvp;
    mvp.mul(proj, model);
    vec4 px = mvp.getrow(0), py = mvp.getrow(1), pw = mvp.getrow(3);
    cull[0] = plane(vec4(pw).add(px)).normalize(); // left plane
    cull[1] = plane(vec4(pw).sub(px)).normalize(); // right plane
    cull[2] = plane(vec4(pw).add(py)).normalize(); // bottom plane
    cull[3] = plane(vec4(pw).sub(py)).normalize(); // top plane
}

int calcbbrsmsplits(const ivec &bbmin, const ivec &bbmax)
{
    if(!rsmcull) return 1;
    loopk(4)
    {
        const plane &p = rsm.cull[k];
        ivec omin, omax;
        if(p.x > 0) { omin.x = bbmin.x; omax.x = bbmax.x; } else { omin.x = bbmax.x; omax.x = bbmin.x; }
        if(p.y > 0) { omin.y = bbmin.y; omax.y = bbmax.y; } else { omin.y = bbmax.y; omax.y = bbmin.y; }
        if(p.z > 0) { omin.z = bbmin.z; omax.z = bbmax.z; } else { omin.z = bbmax.z; omax.z = bbmin.z; }
        if(omax.dist(p) < 0) return 0;
        if(omin.dist(p) < 0) while(++k < 4)
        {
            const plane &p = rsm.cull[k];
            ivec omax(p.x > 0 ? bbmax.x : bbmin.x, p.y > 0 ? bbmax.y : bbmin.y, p.z > 0 ? bbmax.z : bbmin.z);
            if(omax.dist(p) < 0) return 0;
        }
    }
    return 1;
}

int calcspherersmsplits(const vec &center, float radius)
{
    if(!rsmcull) return 1;
    loopk(4)
    {
        const plane &p = rsm.cull[k];
        float dist = p.dist(center);
        if(dist < -radius) return 0;
        if(dist < radius) while(++k < 4)
        {
            const plane &p = rsm.cull[k];
            if(p.dist(center) < -radius) return 0;
        }
    }
    return 1;
}

struct radiancehints
{
    struct splitinfo
    {
        float nearplane, farplane;
        vec offset, scale;
        vec center; float bounds;
    } splits[RH_MAXSPLITS];

    void setup();
    void updatesplitdist();
    void bindparams();
    void renderslices();
} rh;

void radiancehints::updatesplitdist()
{
    float lambda = rhsplitweight, nd = nearplane, fd = rhfarplane, ratio = fd/nd;
    splits[0].nearplane = nd;
    for(int i = 1; i < rhsplits; ++i)
    {
        float si = i / float(rhsplits);
        splits[i].nearplane = lambda*(nd*pow(ratio, si)) + (1-lambda)*(nd + (fd - nd)*si);
        splits[i-1].farplane = splits[i].nearplane * 1.005f;
    }
    splits[rhsplits-1].farplane = fd;
}

void radiancehints::setup()
{
    updatesplitdist();

    loopi(rhsplits)
    {
        splitinfo &split = splits[i];

        vec c;
        float radius = calcfrustumboundsphere(split.nearplane, split.farplane, camera1->o, camdir, c);

        // compute the projected bounding box of the sphere
        const float pradius = radius * rhpradiustweak, step = (2*pradius) / rhgrid;
        vec offset = vec(c).sub(pradius).div(step);
        offset.x = floor(offset.x);
        offset.y = floor(offset.y);
        offset.z = floor(offset.z);
        split.center = vec(offset).mul(step).add(pradius);
        split.bounds = pradius;

        // modify mvp with a scale and offset
        // now compute the update model view matrix for this split
        split.scale = vec(1/(step*(rhgrid+2*rhborder)), 1/(step*(rhgrid+2*rhborder)), 1/(step*(rhgrid+2*rhborder)*rhsplits));
        split.offset = vec(-(offset.x-rhborder)/(rhgrid+2*rhborder), -(offset.y-rhborder)/(rhgrid+2*rhborder), (i - (offset.z-rhborder)/(rhgrid+2*rhborder))/float(rhsplits));
    }
}

void radiancehints::bindparams()
{
    static GlobalShaderParam rhbb("rhbb"), rhscale("rhscale"), rhoffset("rhoffset");
    vec4 *rhbbv = rhbb.reserve<vec4>(rhsplits);
    vec *rhscalev = rhscale.reserve<vec>(rhsplits),
        *rhoffsetv = rhoffset.reserve<vec>(rhsplits);
    loopi(rhsplits)
    {
        splitinfo &split = splits[i];
        rhbbv[i] = vec4(split.center, split.bounds*(1 + rhborder*2*0.5f/rhgrid));
        rhscalev[i] = split.scale;
        rhoffsetv[i] = split.offset;
    }
    float step = 2*splits[0].bounds/rhgrid;
    GLOBALPARAM(rhnudge, (rhnudge*step));
}

void viewbuffersplitmerge()
{
    const int w = screen->w, h = screen->h;
#if 1
    Shader *splitshader = lookupshaderbyname("buffersplit");
    splitshader->set();
    const float split[] = {4.f, 4.f};
    const float tiledim[] = {float(w)/split[0], float(h)/split[1]};
    LOCALPARAM(tiledim, (tiledim[0], tiledim[1]));
    LOCALPARAM(rcptiledim, (1.f/tiledim[0], 1.f/tiledim[1]));
    LOCALPARAM(split, (split[0], split[1]));
    timer_begin(TIMER_SPLITTING);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, gnormaltex);
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(0,        0);        glVertex2i(-w, -h);
    glTexCoord2f(float(w), 0);        glVertex2i(w, -h);
    glTexCoord2f(0,        float(h)); glVertex2i(-w, h);
    glTexCoord2f(float(w), float(h)); glVertex2i(w, h);
    glEnd();
    timer_end(TIMER_SPLITTING);
    notextureshader->set();
#else
    Shader *mergeshader = lookupshaderbyname("buffermerge");
    mergeshader->set();
    const float split[] = {4.f, 4.f};
    const float tiledim[] = {float(w)/split[0], float(h)/split[1]};
    LOCALPARAM(tiledim, (tiledim[0], tiledim[1]));
    LOCALPARAM(rcptiledim, (1.f/tiledim[0], 1.f/tiledim[1]));
    LOCALPARAM(split, (split[0], split[1]));
    timer_begin(TIMER_MERGING);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, gnormaltex);
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(0,        0);        glVertex2i(-w, -h);
    glTexCoord2f(float(w), 0);        glVertex2i(w, -h);
    glTexCoord2f(0,        float(h)); glVertex2i(-w, h);
    glTexCoord2f(float(w), float(h)); glVertex2i(w, h);
    glEnd();
    timer_end(TIMER_MERGING);
    notextureshader->set();

#endif
}
VAR(debugbuffersplit, 0, 0, 1);

VARFP(ao, 0, 1, 1, { cleanupao(); cleardeferredlightshaders(); });
FVARR(aoradius, 0, 5, 256);
FVAR(aocutoff, 0, 2.0f, 1e3f);
FVARR(aodark, 1e-3f, 11.0f, 1e3f);
FVARR(aosharp, 1e-3f, 1, 1e3f);
FVARR(aomin, 0, 0.25f, 1);
VARFR(aosun, 0, 1, 1, cleardeferredlightshaders());
FVARR(aosunmin, 0, 0.5f, 1);
VARP(aoblur, 0, 4, 7);
FVARP(aosigma, 0.005f, 0.5f, 2.0f);
VARP(aoiter, 0, 0, 4);
VARFP(aoreduce, 0, 1, 2, cleanupao());
VARF(aoreducedepth, 0, 1, 2, cleanupao());
VARF(aonoise, 0, 5, 8, cleanupao());
VARFP(aobilateral, 0, 7, 10, cleanupao());
FVARP(aobilateralsigma, 0, 0.5f, 1e3);
FVARP(aobilateraldepth, 0, 4, 1e3);
VARF(aopackdepth, 0, 1, 1, cleanupao());
VARFP(aotaps, 1, 5, 12, cleanupao());
VAR(debugao, 0, 0, 1);

static Shader *deferredlightshader = NULL, *deferredminimapshader = NULL;

void cleardeferredlightshaders()
{
    deferredlightshader = NULL;
    deferredminimapshader = NULL;
}

Shader *loaddeferredlightshader(bool minimap = false)
{
    string common, shadow, sun;
    int commonlen = 0, shadowlen = 0, sunlen = 0;

    if(minimap) common[commonlen++] = 'm';
    if(usegatherforsm()) common[commonlen++] = 'g';
    else if(smfilter) common[commonlen++] = smfilter > 1 ? 'F' : 'f';
    common[commonlen] = '\0';
 
    shadow[shadowlen++] = 'p';
    if(smtetra && glslversion >= 130) shadow[shadowlen++] = 't';
    shadow[shadowlen] = '\0';

    int usecsm = 0, userh = 0;
    if(!minimap && ao) sun[sunlen++] = 'a';
    if(sunlight && csmshadowmap)
    {
        usecsm = csmsplitn;
        sun[sunlen++] = 'c';
        sun[sunlen++] = '0' + csmsplitn;
        if(!minimap)
        {
            if(ao && aosun) sun[sunlen++] = 'A';
            if(gi && giscale && gidist) 
            {
                userh = rhsplits;
                sun[sunlen++] = 'r'; 
                sun[sunlen++] = '0' + rhsplits;
            }
        }
    }
    sun[sunlen] = '\0';

    defformatstring(name)("deferredlight%s%s%s", common, shadow, sun);
    return generateshader(name, "deferredlightshader(\"%s\", \"%s\", \"%s\", %d, %d)", common, shadow, sun, usecsm, userh);
}

void loaddeferredlightshaders()
{
    deferredlightshader = loaddeferredlightshader();
}

static inline bool sortlights(int x, int y)
{
    const lightinfo &xl = lights[x], &yl = lights[y];
    if(!xl.spot) { if(yl.spot) return true; }
    else if(!yl.spot) return false;
    return xl.dist - xl.radius < yl.dist - yl.radius;
}

void resetlights()
{
    shadowcache.reset();
    if(smcache)
    {
        int evictx = ((evictshadowcache%SHADOWCACHE_EVICT)*SHADOWATLAS_SIZE)/SHADOWCACHE_EVICT, 
            evicty = ((evictshadowcache/SHADOWCACHE_EVICT)*SHADOWATLAS_SIZE)/SHADOWCACHE_EVICT,
            evictx2 = (((evictshadowcache%SHADOWCACHE_EVICT)+1)*SHADOWATLAS_SIZE)/SHADOWCACHE_EVICT,
            evicty2 = (((evictshadowcache/SHADOWCACHE_EVICT)+1)*SHADOWATLAS_SIZE)/SHADOWCACHE_EVICT; 
        bool tetra = smtetra && glslversion >= 130;
        loopv(shadowmaps)
        {
            shadowmapinfo &sm = shadowmaps[i];
            if(sm.light < 0) continue;
            lightinfo &l = lights[sm.light];
            if(sm.cached && shadowcachefull)
            {
                int smw = l.spot ? sm.size : (tetra ? sm.size*2 : sm.size*3), smh = l.spot ? sm.size : (tetra ? sm.size : sm.size*2);
                if(sm.x < evictx2 && sm.x + smw > evictx && sm.y < evicty2 && sm.y + smh > evicty) continue;
            }
            shadowcache[l] = sm;
        }
        if(shadowcachefull)
        {
            evictshadowcache = (evictshadowcache + 1)%(SHADOWCACHE_EVICT*SHADOWCACHE_EVICT);
            shadowcachefull = false;
        }
    }

    lights.setsize(0);
    lightorder.setsize(0);
    loopi(LIGHTTILE_H) loopj(LIGHTTILE_W) lighttiles[i][j].setsize(0);

    shadowmaps.setsize(0);
    shadowatlaspacker.reset();
}

static vec *lightsphereverts = NULL;
static GLushort *lightsphereindices = NULL;
static int lightspherenumverts = 0, lightspherenumindices = 0;
static GLuint lightspherevbuf = 0, lightsphereebuf = 0;

static void initlightsphere(int slices, int stacks)
{
    DELETEA(lightsphereverts);
    lightspherenumverts = (stacks+1)*(slices+1);
    lightsphereverts = new vec[lightspherenumverts];
    float ds = 1.0f/slices, dt = 1.0f/stacks, t = 1.0f;
    loopi(stacks+1)
    {
        float rho = M_PI*(1-t), s = 0.0f;
        loopj(slices+1)
        {
            float theta = j==slices ? 0 : 2*M_PI*s;
            lightsphereverts[i*(slices+1) + j] = vec(-sin(theta)*sin(rho), -cos(theta)*sin(rho), cos(rho));
            s += ds;
        }
        t -= dt;
    }

    DELETEA(lightsphereindices);
    lightspherenumindices = stacks*slices*3*2;
    lightsphereindices = new ushort[lightspherenumindices];
    GLushort *curindex = lightsphereindices;
    loopi(stacks)
    {
        loopk(slices)
        {
            int j = i%2 ? slices-k-1 : k;

            *curindex++ = i*(slices+1)+j;
            *curindex++ = i*(slices+1)+j+1;
            *curindex++ = (i+1)*(slices+1)+j;

            *curindex++ = i*(slices+1)+j+1;
            *curindex++ = (i+1)*(slices+1)+j+1;
            *curindex++ = (i+1)*(slices+1)+j;
        }
    }

    if(hasVBO)
    {
        if(!lightspherevbuf) glGenBuffers_(1, &lightspherevbuf);
        glBindBuffer_(GL_ARRAY_BUFFER_ARB, lightspherevbuf);
        glBufferData_(GL_ARRAY_BUFFER_ARB, lightspherenumverts*sizeof(vec), lightsphereverts, GL_STATIC_DRAW_ARB);
        DELETEA(lightsphereverts);

        if(!lightsphereebuf) glGenBuffers_(1, &lightsphereebuf);
        glBindBuffer_(GL_ELEMENT_ARRAY_BUFFER_ARB, lightsphereebuf);
        glBufferData_(GL_ELEMENT_ARRAY_BUFFER_ARB, lightspherenumindices*sizeof(GLushort), lightsphereindices, GL_STATIC_DRAW_ARB);
        DELETEA(lightsphereindices);
    }
}

static void cleanuplightsphere()
{
    if(lightspherevbuf) { glDeleteBuffers_(1, &lightspherevbuf); lightspherevbuf = 0; }
    if(lightsphereebuf) { glDeleteBuffers_(1, &lightsphereebuf); lightsphereebuf = 0; }
    DELETEA(lightsphereverts);
    DELETEA(lightsphereindices);
}

VAR(depthtestlights, 0, 2, 2);
VAR(depthclamplights, 0, 0, 1);
VAR(depthfaillights, 0, 1, 1);
VAR(lighttilebatch, 0, 8, 8);
VAR(batchsunlight, 0, 1, 1);
FVAR(lightradiustweak, 1, 1.11f, 2);

void renderlights(float bsx1 = -1, float bsy1 = -1, float bsx2 = 1, float bsy2 = 1, const uint *tilemask = NULL)
{
    Shader *s = minimapping ? deferredminimapshader : deferredlightshader;
    if(!s || s == nullshader) return;

    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glEnable(GL_SCISSOR_TEST);

    if(!depthtestlights) glDisable(GL_DEPTH_TEST);
    else glDepthMask(GL_FALSE);

    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, gcolortex);
    glActiveTexture_(GL_TEXTURE1_ARB);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, gnormaltex);
    glActiveTexture_(GL_TEXTURE2_ARB);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, gglowtex);
    glActiveTexture_(GL_TEXTURE3_ARB);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, gdepthtex);
    glActiveTexture_(GL_TEXTURE4_ARB);
    glBindTexture(shadowatlastarget, shadowatlastex);
    if(usegatherforsm()) setsmnoncomparemode(); else setsmcomparemode();
    if(ao)
    {
        glActiveTexture_(GL_TEXTURE5_ARB);
        glBindTexture(GL_TEXTURE_RECTANGLE_ARB, aotex[0]);
    }
    if(sunlight && csmshadowmap && gi && giscale && gidist) loopi(3)
    {
        glActiveTexture_(GL_TEXTURE6_ARB + i);
        glBindTexture(GL_TEXTURE_3D_EXT, rhtex[i]);
    }
    glActiveTexture_(GL_TEXTURE0_ARB);

    glMatrixMode(GL_TEXTURE);
    glLoadMatrixf(worldmatrix.v);
    glMatrixMode(GL_MODELVIEW);

    GLOBALPARAM(fogdir, (mvmatrix.getrow(2)));
    GLOBALPARAM(shadowatlasscale, (1.0f/SHADOWATLAS_SIZE, 1.0f/SHADOWATLAS_SIZE));
    if(ao) 
    {
        if((editmode && fullbright) || envmapping)
        {
            GLOBALPARAM(aoscale, (0.0f, 0.0f));
            GLOBALPARAM(aoparams, (1.0f, 0.0f, 1.0f, 0.0f));
        }
        else
        {
            GLOBALPARAM(aoscale, (float(aow)/vieww, float(aoh)/viewh));
            GLOBALPARAM(aoparams, (aomin, 1.0f-aomin, aosunmin, 1.0f-aosunmin));
        }
    }
    float lightscale = 2.0f*ldrscaleb;
    if(editmode && fullbright)
        GLOBALPARAM(lightscale, (fullbrightlevel*lightscale, fullbrightlevel*lightscale, fullbrightlevel*lightscale, 255*lightscale));
    else
        GLOBALPARAM(lightscale, (ambientcolor.x*lightscale*ambientscale, ambientcolor.y*lightscale*ambientscale, ambientcolor.z*lightscale*ambientscale, 255*lightscale));

    bool sunpass = !lighttilebatch;
    if(sunlight && csmshadowmap)
    {
        csm.bindparams();
        rh.bindparams();
        if(editmode && fullbright)
        {
            GLOBALPARAM(sunlightdir, (0, 0, 0));
            GLOBALPARAM(sunlightcolor, (0, 0, 0));
            GLOBALPARAM(giscale, (0));
        }
        else
        {
            GLOBALPARAM(sunlightdir, (sunlightdir));
            GLOBALPARAM(sunlightcolor, (sunlightcolor.x*lightscale*sunlightscale, sunlightcolor.y*lightscale*sunlightscale, sunlightcolor.z*lightscale*sunlightscale));
            GLOBALPARAM(giscale, (2*giscale));
        }
        if(!batchsunlight) sunpass = true; 
    }

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    int btx1 = max(int(floor((bsx1 + 1)*0.5f*LIGHTTILE_W)), 0), bty1 = max(int(floor((bsy1 + 1)*0.5f*LIGHTTILE_H)), 0),
        btx2 = min(int(ceil((bsx2 + 1)*0.5f*LIGHTTILE_W)), LIGHTTILE_W), bty2 = min(int(ceil((bsy2 + 1)*0.5f*LIGHTTILE_H)), LIGHTTILE_H);
    if(sunpass)
    {
        int tx1 = max(int(floor((bsx1*0.5f+0.5f)*vieww)), 0), ty1 = max(int(floor((bsy1*0.5f+0.5f)*viewh)), 0),
            tx2 = min(int(ceil((bsx2*0.5f+0.5f)*vieww)), vieww), ty2 = min(int(ceil((bsy2*0.5f+0.5f)*viewh)), viewh);
        s->set();
        glScissor(tx1, ty1, tx2-tx1, ty2-ty1);
        glBegin(GL_TRIANGLE_STRIP);
        glVertex3f( 1, -1, -1);
        glVertex3f(-1, -1, -1);
        glVertex3f( 1,  1, -1);
        glVertex3f(-1,  1, -1);
        glEnd();
    }

    if(hasDBT && depthtestlights > 1) glEnable(GL_DEPTH_BOUNDS_TEST_EXT);

    static LocalShaderParam lightpos("lightpos"), lightcolor("lightcolor"), spotparams("spotparams"), spotx("spotx"), spoty("spoty"), shadowparams("shadowparams"), shadowoffset("shadowoffset");
    static vec4 lightposv[8], spotparamsv[8], shadowparamsv[8];
    static vec lightcolorv[8], spotxv[8], spotyv[8];
    static vec2 shadowoffsetv[8];

    if(!lighttilebatch)
    {
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();

        if(!lightspherevbuf) initlightsphere(10, 5);
        if(hasVBO)
        {
            glBindBuffer_(GL_ARRAY_BUFFER_ARB, lightspherevbuf);
            glBindBuffer_(GL_ELEMENT_ARRAY_BUFFER_ARB, lightsphereebuf);
        }
        glVertexPointer(3, GL_FLOAT, sizeof(vec), lightsphereverts);
        glEnableClientState(GL_VERTEX_ARRAY);

        if(hasDC && depthclamplights) glEnable(GL_DEPTH_CLAMP_NV);

        bool outside = true;
        loopv(lightorder)
        {
            lightinfo &l = lights[lightorder[i]];
            float sx1 = max(bsx1, l.sx1), sy1 = max(bsy1, l.sy1), 
                  sx2 = min(bsx2, l.sx2), sy2 = min(bsy2, l.sy2);
            if(sx1 >= sx2 || sy1 >= sy2 || l.sz1 >= l.sz2) continue;

            bool shadowmap = l.shadowmap >= 0, spotlight = l.spot > 0;

            lightposv[0] = vec4(l.o.x, l.o.y, l.o.z, 1.0f/l.radius);
            lightcolorv[0] = vec(l.color.x*lightscale, l.color.y*lightscale, l.color.z*lightscale);
            if(spotlight)
            {
                float maxatten = sincos360[l.spot].x;
                spotparamsv[0] = vec4(l.dir, maxatten).div(1 - maxatten);
            }
            if(shadowmap)
            {
                shadowmapinfo &sm = shadowmaps[l.shadowmap];
                float smnearclip = SQRT3 / l.radius, smfarclip = SQRT3,
                      bias = (smcullside ? smbias : -smbias) * smnearclip * (1024.0f / sm.size);
                if(spotlight)
                {
                    spotxv[0] = l.spotx;
                    spotyv[0] = l.spoty;
                    float maxatten = sincos360[l.spot].x;
                    shadowparamsv[0] = vec4(
                        0.5f * sm.size / (1 - maxatten),
                        (-smnearclip * smfarclip / (smfarclip - smnearclip) - 0.5f*bias) / (1 - maxatten),
                        sm.size,
                        0.5f + 0.5f * (smfarclip + smnearclip) / (smfarclip - smnearclip));
                }
                else
                {
                    shadowparamsv[0] = vec4(
                        0.5f * (sm.size - smborder),
                        -smnearclip * smfarclip / (smfarclip - smnearclip) - 0.5f*bias,
                        sm.size,
                        0.5f + 0.5f * (smfarclip + smnearclip) / (smfarclip - smnearclip));
                }
                shadowoffsetv[0] = vec2(sm.x + 0.5f*sm.size, sm.y + 0.5f*sm.size);
            }

            s->setvariant(0, (shadowmap ? 1 : 0) + 2 + (spotlight ? 4 : 0));
            lightpos.set(lightposv, 1);
            lightcolor.set(lightcolorv, 1);
            if(spotlight) spotparams.set(spotparamsv, 1);
            if(shadowmap)
            {
                if(spotlight)
                {
                    spotx.set(spotxv, 1);
                    spoty.set(spotyv, 1);
                }
                shadowparams.set(shadowparamsv, 1);
                shadowoffset.set(shadowoffsetv, 1);
            }

            int tx1 = int(floor((sx1*0.5f+0.5f)*vieww)), ty1 = int(floor((sy1*0.5f+0.5f)*viewh)),
                tx2 = int(ceil((sx2*0.5f+0.5f)*vieww)), ty2 = int(ceil((sy2*0.5f+0.5f)*viewh));
            glScissor(tx1, ty1, tx2-tx1, ty2-ty1);

            if(hasDBT && depthtestlights > 1) glDepthBounds_(l.sz1*0.5f + 0.5f, l.sz2*0.5f + 0.5f);

            if(camera1->o.dist(l.o) <= l.radius*lightradiustweak + nearplane + 1 && depthfaillights)
            {
                if(outside)
                {
                    outside = false;
                    glDepthFunc(GL_GEQUAL);
                    glCullFace(GL_FRONT);
                }
            }
            else if(!outside)
            {
                outside = true;
                glDepthFunc(GL_LESS);
                glCullFace(GL_BACK);
            }

            glPushMatrix();
            glTranslatef(l.o.x, l.o.y, l.o.z);
            glScalef(l.radius*lightradiustweak, l.radius*lightradiustweak, l.radius*lightradiustweak);

            if(hasDRE) glDrawRangeElements_(GL_TRIANGLES, 0, lightspherenumverts-1, lightspherenumindices, GL_UNSIGNED_SHORT, lightsphereindices);
            else glDrawElements(GL_TRIANGLES, lightspherenumindices, GL_UNSIGNED_SHORT, lightsphereindices);
            xtraverts += lightspherenumindices;
            glde++;
            
            glPopMatrix();
        }

        if(!outside)
        {
            outside = true;
            glDepthFunc(GL_LESS);
            glCullFace(GL_BACK);
        }

        if(hasDC && depthclamplights) glDisable(GL_DEPTH_CLAMP_NV);

        glDisableClientState(GL_VERTEX_ARRAY);
        if(hasVBO)
        {
            glBindBuffer_(GL_ARRAY_BUFFER_ARB, 0);
            glBindBuffer_(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
        }
    }
    else for(int y = bty1; y < bty2; y++) if(!tilemask || tilemask[y]) for(int x = btx1; x < btx2; x++) if(!tilemask || tilemask[y]&(1<<x))
    {
        vector<int> &tile = lighttiles[y][x];

        if(sunpass && tile.empty()) continue;
        for(int i = 0;;)
        {
            int n = min(tile.length() - i, lighttilebatch);
            bool shadowmap = false, spotlight = false;
            if(n > 0)
            {
                lightinfo &l = lights[tile[i]];
                shadowmap = l.shadowmap >= 0;
                spotlight = l.spot > 0;
            }
            loopj(n)
            {
                lightinfo &l = lights[tile[i+j]];
                if((l.shadowmap >= 0) != shadowmap || (l.spot > 0) != spotlight) { n = j; break; }
            }

            float sx1 = 1, sy1 = 1, sx2 = -1, sy2 = -1, sz1 = 1, sz2 = -1;
            loopj(n)
            {
                lightinfo &l = lights[tile[i+j]];
                lightposv[j] = vec4(l.o.x, l.o.y, l.o.z, 1.0f/l.radius);
                lightcolorv[j] = vec(l.color.x*lightscale, l.color.y*lightscale, l.color.z*lightscale);
                if(spotlight)
                {
                    float maxatten = sincos360[l.spot].x;
                    spotparamsv[j] = vec4(l.dir, maxatten).div(1 - maxatten);
                }
                if(shadowmap)
                {
                    shadowmapinfo &sm = shadowmaps[l.shadowmap];
                    float smnearclip = SQRT3 / l.radius, smfarclip = SQRT3,
                          bias = (smcullside ? smbias : -smbias) * smnearclip * (1024.0f / sm.size);
                    if(spotlight)
                    {
                        spotxv[j] = l.spotx;
                        spotyv[j] = l.spoty;
                        float maxatten = sincos360[l.spot].x;
                        shadowparamsv[j] = vec4(
                            0.5f * sm.size / (1 - maxatten),
                            (-smnearclip * smfarclip / (smfarclip - smnearclip) - 0.5f*bias) / (1 - maxatten),
                            sm.size,
                            0.5f + 0.5f * (smfarclip + smnearclip) / (smfarclip - smnearclip));
                    }
                    else 
                    {
                        shadowparamsv[j] = vec4(
                            0.5f * (sm.size - smborder),
                            -smnearclip * smfarclip / (smfarclip - smnearclip) - 0.5f*bias,
                            sm.size,
                            0.5f + 0.5f * (smfarclip + smnearclip) / (smfarclip - smnearclip));
                    }
                    shadowoffsetv[j] = vec2(sm.x + 0.5f*sm.size, sm.y + 0.5f*sm.size);
                }
                sx1 = min(sx1, l.sx1);
                sy1 = min(sy1, l.sy1);
                sx2 = max(sx2, l.sx2);
                sy2 = max(sy2, l.sy2);
                sz1 = min(sz1, l.sz1);
                sz2 = max(sz2, l.sz2);
            }
            if(!i && !sunpass) { sx1 = sy1 = sz1 = -1; sx2 = sy2 = sz2 = 1; }
            else if(sx1 >= sx2 || sy1 >= sy2 || sz1 >= sz2) continue;

            if(n) 
            {
                s->setvariant(n-1, (shadowmap ? 1 : 0) + (i || sunpass ? 2 : 0) + (spotlight ? 4 : 0));
                lightpos.set(lightposv, n);
                lightcolor.set(lightcolorv, n);
                if(spotlight) spotparams.set(spotparamsv, n);
                if(shadowmap)   
                {
                    if(spotlight) 
                    { 
                        spotx.set(spotxv, n);
                        spoty.set(spotyv, n);
                    }
                    shadowparams.set(shadowparamsv, n);
                    shadowoffset.set(shadowoffsetv, n);
                }
            } 
            else s->set();

            sx1 = max(sx1, bsx1);
            sy1 = max(sy1, bsy1);
            sx2 = min(sx2, bsx2);
            sy2 = min(sy2, bsy2);
            if(sx1 < sx2 && sy1 < sy2)
            { 
                int tx1 = max(int(floor((sx1*0.5f+0.5f)*vieww)), (x*vieww)/LIGHTTILE_W), ty1 = max(int(floor((sy1*0.5f+0.5f)*viewh)), (y*viewh)/LIGHTTILE_H),
                    tx2 = min(int(ceil((sx2*0.5f+0.5f)*vieww)), ((x+1)*vieww)/LIGHTTILE_W), ty2 = min(int(ceil((sy2*0.5f+0.5f)*viewh)), ((y+1)*viewh)/LIGHTTILE_H);

                glScissor(tx1, ty1, tx2-tx1, ty2-ty1);

                if(hasDBT && depthtestlights > 1) glDepthBounds_(sz1*0.5f + 0.5f, sz2*0.5f + 0.5f);

                // FIXME: render light geometry here
                glBegin(GL_TRIANGLE_STRIP);
                glVertex3f( 1, -1, sz1);
                glVertex3f(-1, -1, sz1);
                glVertex3f( 1,  1, sz1);
                glVertex3f(-1,  1, sz1);
                glEnd();
            }

            i += n;
            if(i >= tile.length()) break;
        }
    }

    if(lighttilebatch)
    {
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
    }
        
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_BLEND);

    if(!depthtestlights) glEnable(GL_DEPTH_TEST);
    else
    {
        glDepthMask(GL_TRUE);
        if(hasDBT && depthtestlights > 1) glDisable(GL_DEPTH_BOUNDS_TEST_EXT);
    }

    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
}

VAR(oqlights, 0, 1, 1);
VAR(debuglightscissor, 0, 0, 1);

void viewlightscissor()
{
    loopv(entgroup)
    {
        int idx = entgroup[i];
        if(entities::storage.inrange(idx) && entities::storage[idx]->type == ET_LIGHT) 
        { 
            extentity &e = *entities::storage[idx];
            loopvj(lights) if(lights[j].o == e.o)
            {
                lightinfo &l = lights[j];
                if(l.sx1 >= l.sx2 || l.sy1 >= l.sy2 || l.sz1 >= l.sz2) break;
                glColor3f(l.color.x/255, l.color.y/255, l.color.z/255);
                float x1 = (l.sx1+1)/2*screen->w, x2 = (l.sx2+1)/2*screen->w,
                      y1 = (1-l.sy1)/2*screen->h, y2 = (1-l.sy2)/2*screen->h;
                glBegin(GL_TRIANGLE_STRIP);
                glVertex2f(x1, y1);
                glVertex2f(x2, y1);
                glVertex2f(x1, y2);
                glVertex2f(x2, y2);
                glEnd();
            }
        }
    }
}

static inline bool calclightscissor(lightinfo &l)
{
    float sx1 = -1, sy1 = -1, sx2 = 1, sy2 = 1, sz1 = -1, sz2 = 1;
    if(l.radius > 0)
    {
        if(l.spot > 0) calcspotscissor(l.o, l.radius, l.dir, l.spot, l.spotx, l.spoty, sx1, sy1, sx2, sy2, sz1, sz2);
        else calcspherescissor(l.o, l.radius, sx1, sy1, sx2, sy2, sz1, sz2);
    }
    l.sx1 = sx1;
    l.sx2 = sx2;
    l.sy1 = sy1;
    l.sy2 = sy2;
    l.sz1 = sz1;
    l.sz2 = sz2;
    return sx1 < sx2 && sy1 < sy2 && sz1 < sz2; 
}

void collectlights()
{
    // point lights processed here
    if(!editmode || !fullbright) loopv(entities::storage)
    {
        extentity *e = entities::get(i);
        if(e->type != ET_LIGHT) continue;

        if(e->attr1 > 0 && smviscull)
        {
            if(isfoggedsphere(e->attr1, e->o)) continue;
            if(pvsoccludedsphere(e->o, e->attr1)) continue;
        }

        lightinfo &l = lights.add();
        l.shadowmap = -1;
        l.flags = e->attr5;
        l.query = NULL;
        l.o = e->o;
        l.color = vec(e->attr2, e->attr3, e->attr4);
        l.radius = e->attr1 > 0 ? e->attr1 : 2*worldsize;
        if(e->attached && e->attached->type == ET_SPOTLIGHT)
        {
            l.dir = vec(e->attached->o).sub(e->o).normalize();
            l.spot = clamp(int(e->attached->attr1), 1, 89);
            vec adir(fabs(l.dir.x), fabs(l.dir.y), fabs(l.dir.z)), spotx(1, 0, 0), spoty(0, 1, 0);
            if(adir.x > adir.y) { if(adir.x > adir.z) spotx = vec(0, 0, 1); }
            else if(adir.y > adir.z) spoty = vec(0, 0, 1);
            l.dir.orthonormalize(spotx, spoty);
            const vec2 &sc = sincos360[l.spot];
            float spotscale = sc.x/sc.y;
            l.spotx = spotx.rescale(spotscale);
            l.spoty = spoty.rescale(spotscale);
        }
        else
        {
            l.dir = vec(0, 0, 0);
            l.spot = 0;
        }
        l.dist = camera1->o.dist(e->o);

        if(calclightscissor(l)) lightorder.add(lights.length()-1);
    }

    updatedynlights();
    int numdynlights = finddynlights();
    loopi(numdynlights)
    {
        vec o, color;
        float radius;
        if(!getdynlight(i, o, radius, color)) continue;

        lightinfo &l = lights.add();
        l.shadowmap = -1;
        l.flags = 0;
        l.query = NULL;
        l.o = o;
        l.color = vec(color).mul(255);
        l.radius = radius;
        l.dir = vec(0, 0, 0);
        l.spot = 0;
        l.dist = camera1->o.dist(o);

        if(calclightscissor(l)) lightorder.add(lights.length()-1);
    }

    lightorder.sort(sortlights);

    if(!envmapping && smquery && hasOQ && oqfrags && oqlights) loopv(lightorder)
    {
        int idx = lightorder[i];
        lightinfo &l = lights[idx];
        if(l.flags&L_NOSHADOW || l.radius <= smminradius || l.radius >= worldsize) continue;
        vec bbmin, bbmax;
        if(l.spot > 0)
        {
            const vec2 &sc = sincos360[l.spot];
            float spotscale = l.radius*sc.y/sc.x;
            vec up = vec(l.spotx).rescale(spotscale), right = vec(l.spoty).rescale(spotscale), center = vec(l.dir).mul(l.radius).add(l.o);
            up.x = fabs(up.x); up.y = fabs(up.y); up.z = fabs(up.z);
            right.x = fabs(right.x); right.y = fabs(right.y); right.z = fabs(right.z);
            bbmin = bbmax = center;
            bbmin.sub(up).sub(right);
            bbmax.add(up).add(right);
            bbmin.min(l.o);
            bbmax.max(l.o);
        }
        else
        {
            bbmin = vec(l.o).sub(l.radius);
            bbmax = vec(l.o).add(l.radius);
        }        
        if(camera1->o.x < bbmin.x - 2 || camera1->o.x > bbmax.x + 2 ||
           camera1->o.y < bbmin.y - 2 || camera1->o.y > bbmax.y + 2 ||
           camera1->o.z < bbmin.z - 2 || camera1->o.z > bbmax.z + 2)
        {
            l.query = newquery(&l);
            if(l.query)
            {
                startquery(l.query);
                ivec bo = bbmin, br = bbmax;
                br.sub(bo).add(1);
                drawbb(bo, br);
                endquery(l.query);
            }
        }
    }
}

static inline void addlighttiles(const lightinfo &l, int idx)
{
    int tx1 = max(int(floor((l.sx1 + 1)*0.5f*LIGHTTILE_W)), 0), ty1 = max(int(floor((l.sy1 + 1)*0.5f*LIGHTTILE_H)), 0),
        tx2 = min(int(ceil((l.sx2 + 1)*0.5f*LIGHTTILE_W)), LIGHTTILE_W), ty2 = min(int(ceil((l.sy2 + 1)*0.5f*LIGHTTILE_H)), LIGHTTILE_H);
    for(int y = ty1; y < ty2; y++) for(int x = tx1; x < tx2; x++) { lighttiles[y][x].add(idx); lighttilesused++; }
}
 
VAR(lightsvisible, 1, 0, 0);
VAR(lightsoccluded, 1, 0, 0);
 
void packlights()
{ 
    lightsvisible = lightsoccluded = 0;
    lighttilesused = 0;
    smused = 0;

    bool tetra = smtetra && glslversion >= 130;
    if(smcache && smnoshadow <= 1 && shadowcache.numelems) loopv(lightorder)
    {
        int idx = lightorder[i];
        lightinfo &l = lights[idx];
        if(l.flags&L_NOSHADOW || l.radius <= smminradius) continue;
        if(l.query && l.query->owner == &l && checkquery(l.query)) continue; 

        float prec = smprec, smlod;
        int smw, smh;
        if(l.spot) { smw = 1; smh = 1; const vec2 &sc = sincos360[l.spot]; prec = sc.y/sc.x; smlod = smspotprec; }
        else if(tetra) { smw = 2; smh = 1; smlod = smtetraprec; }
        else { smw = 3; smh = 2; smlod = smcubeprec; }
        smlod *= clamp(l.radius * prec / sqrtf(max(1.0f, l.dist/l.radius)), float(smminsize), float(smmaxsize));
        int smsize = clamp(int(ceil(smlod)), 1, SHADOWATLAS_SIZE / smw);
        smw *= smsize;
        smh *= smsize;
        ushort smx = USHRT_MAX, smy = USHRT_MAX;
        shadowmapinfo *sm = NULL;
        int smidx = -1;
        shadowcacheval *cached = shadowcache.access(l);
        if(!cached || cached->size != smsize) continue;
        smx = cached->x;
        smy = cached->y;
        shadowatlaspacker.reserve(smx, smy, smw, smh);
        sm = addshadowmap(smx, smy, smsize, smidx);
        sm->light = idx;
        sm->cached = cached;
        l.shadowmap = smidx;
        smused += smw*smh;

        addlighttiles(l, idx);
    }
    loopv(lightorder)
    {
        int idx = lightorder[i];
        lightinfo &l = lights[idx];
        if(l.shadowmap >= 0) continue;

        if(!(l.flags&L_NOSHADOW) && smnoshadow <= 1 && l.radius > smminradius)
        {
            if(l.query && l.query->owner == &l && checkquery(l.query)) { lightsoccluded++; continue; }
            float prec = smprec, smlod;
            int smw, smh;
            if(l.spot) { smw = 1; smh = 1; const vec2 &sc = sincos360[l.spot]; prec = sc.y/sc.x; smlod = smspotprec; }
            else if(tetra) { smw = 2; smh = 1; smlod = smtetraprec; }
            else { smw = 3; smh = 2; smlod = smcubeprec; }
            smlod *= clamp(l.radius * prec / sqrtf(max(1.0f, l.dist/l.radius)), float(smminsize), float(smmaxsize));
            int smsize = clamp(int(ceil(smlod)), 1, SHADOWATLAS_SIZE / smw);
            smw *= smsize;
            smh *= smsize;
            ushort smx = USHRT_MAX, smy = USHRT_MAX;
            shadowmapinfo *sm = NULL;
            int smidx = -1;
            if(shadowatlaspacker.insert(smx, smy, smw, smh))
            {
                sm = addshadowmap(smx, smy, smsize, smidx);
                sm->light = idx;
                l.shadowmap = smidx;
                smused += smw*smh;
            }
            else if(smcache) shadowcachefull = true;
        }
   
        addlighttiles(l, idx); 
    }

    lightsvisible = lightorder.length() - lightsoccluded;
}

void radiancehints::renderslices()
{
    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, rhfbo);
    glViewport(0, 0, rhgrid+2*rhborder, rhgrid+2*rhborder);

    GLOBALPARAM(rhatten, (1.0f/(gidist*gidist)));
    GLOBALPARAM(rsmspread, (gidist*rsmspread*rsm.scale.x, gidist*rsmspread*rsm.scale.y));

    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glTranslatef(rsm.offset.x, rsm.offset.y, rsm.offset.z);
    glScalef(rsm.scale.x, rsm.scale.y, rsm.scale.z);
    glMultMatrixf(rsm.model.v);
    glMatrixMode(GL_MODELVIEW);

    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, rsmdepthtex);
    glActiveTexture_(GL_TEXTURE1_ARB);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, rsmcolortex);
    glActiveTexture_(GL_TEXTURE2_ARB);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, rsmnormaltex);
    glActiveTexture_(GL_TEXTURE3_ARB);
    glBindTexture(GL_TEXTURE_3D_EXT, rhtex[0]);
    glActiveTexture_(GL_TEXTURE4_ARB);
    glBindTexture(GL_TEXTURE_3D_EXT, rhtex[1]);
    glActiveTexture_(GL_TEXTURE5_ARB);
    glBindTexture(GL_TEXTURE_3D_EXT, rhtex[2]);
    glActiveTexture_(GL_TEXTURE0_ARB);

    glClearColor(0.5f, 0.5f, 0.5f, 0);

    loopirev(rhsplits)
    {
        splitinfo &split = splits[i];

        float cellradius = split.bounds/rhgrid, step = 2*cellradius;
        GLOBALPARAM(rhspread, (cellradius));

        vec cmin, cmax, bmin(1e16f, 1e16f, 1e16f), bmax(-1e16f, -1e16f, -1e16f);
        loopk(3)
        {
            cmin[k] = floor((worldmin[k] - (split.center[k] - split.bounds))/step)*step + split.center[k] - split.bounds;
            cmax[k] = ceil((worldmax[k] - (split.center[k] - split.bounds))/step)*step + split.center[k] - split.bounds;
        }
        if(rhborder && i + 1 < rhsplits)
        {
            GLOBALPARAM(bordercenter, (0.5f, 0.5f, float(i+1 + 0.5f)/rhsplits));
            GLOBALPARAM(borderrange, (0.5f - 0.5f/(rhgrid+2), 0.5f - 0.5f/(rhgrid+2), (0.5f - 0.5f/(rhgrid+2))/rhsplits));
            GLOBALPARAM(borderscale, (rhgrid+2, rhgrid+2, (rhgrid+2)*rhsplits));
            splitinfo &next = splits[i+1];
            loopk(3)
            {
                bmin[k] = floor((max(float(worldmin[k]), next.center[k] - next.bounds) - (split.center[k] - split.bounds))/step)*step + split.center[k] - split.bounds;
                bmax[k] = ceil((min(float(worldmax[k]), next.center[k] + next.bounds) - (split.center[k] - split.bounds))/step)*step + split.center[k] - split.bounds;
            }
        }

        loopjrev(rhgrid+2*rhborder)
        {
            glFramebufferTexture3D_(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_3D_EXT, rhtex[0], 0, i*(rhgrid+2*rhborder) + j);
            glFramebufferTexture3D_(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_3D_EXT, rhtex[1], 0, i*(rhgrid+2*rhborder) + j);
            glFramebufferTexture3D_(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT2_EXT, GL_TEXTURE_3D_EXT, rhtex[2], 0, i*(rhgrid+2*rhborder) + j);

            float x1 = split.center.x - split.bounds, x2 = split.center.x + split.bounds,
                  y1 = split.center.y - split.bounds, y2 = split.center.y + split.bounds,
                  z = split.center.z - split.bounds + (j-rhborder+0.5f)*step,
                  vx1 = -1 + rhborder*2.0f/(rhgrid+2), vx2 = 1 - rhborder*2.0f/(rhgrid+2), vy1 = -1 + rhborder*2.0f/(rhgrid+2), vy2 = 1 - rhborder*2.0f/(rhgrid+2),
                  tx1 = x1, tx2 = x2, ty1 = y1, ty2 = y2;

            if(rhborder && i + 1 < rhsplits)
            {
                splitinfo &next = splits[i+1];
                float bx1 = x1-step, bx2 = x2+step, by1 = y1-step, by2 = y2+step, bz = z,
                      bvx1 = -1, bvx2 = 1, bvy1 = -1, bvy2 = 1,
                      btx1 = bx1, btx2 = bx2, bty1 = by1, bty2 = by2;

                if(rhclipgrid)
                {
                    if(bz < bmin.z || bz > bmax.z) goto noborder;
                    if(bx1 < bmin.x || bx2 > bmax.x || by1 < bmin.y || by2 > bmax.y)
                    {
                        btx1 = max(bx1, bmin.x);
                        btx2 = min(bx2, bmax.x);
                        bty1 = max(by1, bmin.y);
                        bty2 = min(by2, bmax.y);
                        if(btx1 > tx2 || bty1 > bty2) goto noborder;
                        bvx1 += 2*(btx1 - bx1)/(bx2 - bx1);
                        bvx2 += 2*(btx2 - bx2)/(bx2 - bx1);
                        bvy1 += 2*(bty1 - by1)/(by2 - by1);
                        bvy2 += 2*(bty2 - by2)/(by2 - by1);

                        glClear(GL_COLOR_BUFFER_BIT);
                    }
                }

                btx1 = btx1*next.scale.x + next.offset.x;
                btx2 = btx2*next.scale.x + next.offset.x;
                bty1 = bty1*next.scale.y + next.offset.y;
                bty2 = bty2*next.scale.y + next.offset.y;
                bz = bz*next.scale.z + next.offset.z;
                
                SETSHADER(radiancehintsborder);

                glBegin(GL_TRIANGLE_STRIP);
                glTexCoord3f(btx2, bty1, bz); glVertex2f(bvx2, bvy1);
                glTexCoord3f(btx1, bty1, bz); glVertex2f(bvx1, bvy1);
                glTexCoord3f(btx2, bty2, bz); glVertex2f(bvx2, bvy2);
                glTexCoord3f(btx1, bty2, bz); glVertex2f(bvx1, bvy2);
                glEnd();
            }
            else
            {
            noborder:
                if(rhborder) glClear(GL_COLOR_BUFFER_BIT);
            }

            if(rhclipgrid)
            {
                if(z < cmin.z || z > cmax.z) 
                {
                    if(!rhborder) glClear(GL_COLOR_BUFFER_BIT);
                    continue;
                }
                if(x1 < cmin.x || x2 > cmax.x || y1 < cmin.y || y2 > cmax.y)
                {
                    if(!rhborder) glClear(GL_COLOR_BUFFER_BIT);
                    tx1 = max(x1, cmin.x);
                    tx2 = min(x2, cmax.x);
                    ty1 = max(y1, cmin.y);
                    ty2 = min(y2, cmax.y);
                    if(tx1 > tx2 || ty1 > ty2) continue;
                    vx1 += 2*rhgrid/float(rhgrid+2*rhborder)*(tx1 - x1)/(x2 - x1);
                    vx2 += 2*rhgrid/float(rhgrid+2*rhborder)*(tx2 - x2)/(x2 - x1);
                    vy1 += 2*rhgrid/float(rhgrid+2*rhborder)*(ty1 - y1)/(y2 - y1);
                    vy2 += 2*rhgrid/float(rhgrid+2*rhborder)*(ty2 - y2)/(y2 - y1);
                }
            }

            radiancehintsshader->set();

            glBegin(GL_TRIANGLE_STRIP);
            glTexCoord3f(tx2, ty1, z); glVertex2f(vx2, vy1);
            glTexCoord3f(tx1, ty1, z); glVertex2f(vx1, vy1);
            glTexCoord3f(tx2, ty2, z); glVertex2f(vx2, vy2);
            glTexCoord3f(tx1, ty2, z); glVertex2f(vx1, vy2);
            glEnd();
        }
    }

    glClearColor(0, 0, 0, 0);
}

VAR(rhinoq, 0, 0, 1);

void renderradiancehints()
{
    if(!sunlight || !csmshadowmap || !gi || !giscale || !gidist) return;

    if(!rhinoq) timer_begin(TIMER_RH);
    else glFlush();

    rsm.setup();
    rh.setup();

    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, rsmfbo);

    shadowmapping = SM_REFLECT;
    shadoworigin = vec(0, 0, 0);
    shadowdir = rsm.lightview;
    shadowbias = rsm.lightview.project_bb(worldmin, worldmax);
    shadowradius = fabs(rsm.lightview.project_bb(worldmax, worldmin));

    GLOBALPARAM(rsmdir, (-rsm.lightview.x, -rsm.lightview.y, -rsm.lightview.z));

    findshadowvas();
    findshadowmms();

    shadowmaskbatchedmodels(false);
    batchshadowmapmodels();

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadMatrixf(rsm.proj.v);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadMatrixf(rsm.model.v);
    glViewport(0, 0, rsmsize, rsmsize);
    glClearColor(0, 0, 0, 0);
    glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);

    shadowside = 0;

    renderrsmgeom();
    rendermodelbatches();

    clearbatchedmapmodels();

    shadowmapping = 0;

    rh.renderslices();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    if(!rhinoq) timer_end(TIMER_RH);
    else
    {
        glBindFramebuffer_(GL_FRAMEBUFFER_EXT, gfbo);
        glViewport(0, 0, vieww, viewh);
    }
}

void rendercsmshadowmaps()
{
    shadowmapping = SM_CASCADE;
    shadoworigin = vec(0, 0, 0);
    shadowdir = csm.lightview;
    shadowbias = csm.lightview.project_bb(worldmin, worldmax);
    shadowradius = fabs(csm.lightview.project_bb(worldmax, worldmin));

    if(csmpolyfactor || csmpolyoffset)
    {
        glPolygonOffset(csmpolyfactor, csmpolyoffset);
        glEnable(GL_POLYGON_OFFSET_FILL);
    }

    findshadowvas();
    findshadowmms();

    shadowmaskbatchedmodels();
    batchshadowmapmodels();

    loopi(csmsplitn) if(csm.splits[i].idx >= 0)
    {
        const shadowmapinfo &sm = shadowmaps[csm.splits[i].idx];

        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf(csm.splits[i].proj.v);
        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixf(csm.model.v);
        glViewport(sm.x, sm.y, sm.size, sm.size);
        glScissor(sm.x, sm.y, sm.size, sm.size);
        glClear(GL_DEPTH_BUFFER_BIT);

        shadowside = i;

        rendershadowmapworld();
        rendermodelbatches();
    }

    clearbatchedmapmodels();

    if(csmpolyfactor || csmpolyoffset) glDisable(GL_POLYGON_OFFSET_FILL);

    shadowmapping = 0;
}

void rendershadowmaps()
{
    bool tetra = smtetra && glslversion >= 130;
    if(smpolyfactor || smpolyoffset)
    {
        glPolygonOffset(smpolyfactor, smpolyoffset);
        glEnable(GL_POLYGON_OFFSET_FILL);
    }

    loopv(shadowmaps)
    {
        shadowmapinfo &sm = shadowmaps[i];
        if(sm.light < 0) continue;

        lightinfo &l = lights[sm.light];

        int border, sidemask;
        if(l.spot) 
        {
            if(shadowmapping == SM_TETRA && smtetraclip) glDisable(GL_CLIP_PLANE0);
            shadowmapping = SM_SPOT;
            border = 0;
            sidemask = 1;
        }
        else if(tetra) 
        {
            if(shadowmapping != SM_TETRA && smtetraclip) glEnable(GL_CLIP_PLANE0);
            shadowmapping = SM_TETRA;
            border = smborder;
            sidemask = smsidecull ? cullfrustumtetra(l.o, l.radius, sm.size, smborder) : 0xF;
        }
        else 
        {
            shadowmapping = SM_CUBEMAP;
            border = smborder;
            sidemask = smsidecull ? cullfrustumsides(l.o, l.radius, sm.size, smborder) : 0x3F;
        }

        sm.sidemask = sidemask;

        shadoworigin = l.o;
        shadowradius = l.radius;
        shadowbias = border / float(sm.size - border);
        shadowdir = l.dir;
        shadowspot = l.spot;

        findshadowvas();
        findshadowmms();

        shadowmaskbatchedmodels(!(l.flags&L_NODYNSHADOW));
        batchshadowmapmodels();

        shadowcacheval *cached = NULL;
        int cachemask = 0;
        if(smcache)
        {
            int dynmask = smcache <= 1 ? batcheddynamicmodels() : 0;
            if(shadowmapping == SM_TETRA && !smtetraclear)
            {
                if(dynmask&0x3) dynmask |= 0x3;
                if(dynmask&0xC) dynmask |= 0xC;
            }
            cached = sm.cached;
            if(cached)
            {
                if(!debugshadowatlas) cachemask = cached->sidemask & ~dynmask;
                sm.sidemask |= cachemask;
            }
            sm.sidemask &= ~dynmask;

            sidemask &= ~cachemask;
            if(!sidemask) { clearbatchedmapmodels(); continue; }
        }

        float smnearclip = SQRT3 / l.radius, smfarclip = SQRT3;
        GLfloat smprojmatrix[16] =
        {
            float(sm.size - border) / sm.size, 0, 0, 0,
            0, float(sm.size - border) / sm.size, 0, 0,
            0, 0, -(smfarclip + smnearclip) / (smfarclip - smnearclip), -1,
            0, 0, -2*smnearclip*smfarclip / (smfarclip - smnearclip), 0
        };
        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf(smprojmatrix);
        glMatrixMode(GL_MODELVIEW);

        GLfloat lightmatrix[16] =
        {
            1.0f/l.radius, 0, 0, 0,
            0, 1.0f/l.radius, 0, 0,
            0, 0, 1.0f/l.radius, 0,
            -l.o.x/l.radius, -l.o.y/l.radius, -l.o.z/l.radius, 1
        };

        glmatrixf smviewmatrix;

        if(shadowmapping == SM_SPOT)
        {
            glViewport(sm.x, sm.y, sm.size, sm.size);
            glScissor(sm.x, sm.y, sm.size, sm.size);
            glClear(GL_DEPTH_BUFFER_BIT);

            GLfloat spotmatrix[16] = 
            {
                l.spotx.x, l.spoty.x, -l.dir.x, 0, 
                l.spotx.y, l.spoty.y, -l.dir.y, 0,
                l.spotx.z, l.spoty.z, -l.dir.z, 0,
                0, 0, 0, 1
            };
            smviewmatrix.mul(spotmatrix, lightmatrix);
            glLoadMatrixf(smviewmatrix.v);

            glCullFace((l.dir.scalartriple(l.spoty, l.spotx) < 0) == (smcullside != 0) ? GL_BACK : GL_FRONT);

            shadowside = 0;

            rendershadowmapworld();
            rendermodelbatches();
        }
        else if(shadowmapping == SM_TETRA)
        {
            if(!cachemask)
            {
                glScissor(sm.x + (sidemask & 0x3 ? 0 : sm.size), sm.y, sidemask & 0x3 && sidemask & 0xC ? 2*sm.size : sm.size, sm.size);
                glClear(GL_DEPTH_BUFFER_BIT);
            }
            loop(side, 4) if(sidemask&(1<<side))
            {
                int sidex = (side>>1)*sm.size;
                if(!(side&1) || !(sidemask&(1<<(side-1))))
                {
                    glViewport(sm.x + sidex, sm.y, sm.size, sm.size);
                    glScissor(sm.x + sidex, sm.y, sm.size, sm.size);
                    if(cachemask && (!smtetraclear || (sidemask&(3<<(side&~1))) == (3<<(side&~1)))) glClear(GL_DEPTH_BUFFER_BIT);
                }

                if(cachemask && smtetraclear && (sidemask&(3<<(side&~1))) != (3<<(side&~1)))
                {
                    if(smtetraclip) glDisable(GL_CLIP_PLANE0);
                    if(smpolyfactor || smpolyoffset) glDisable(GL_POLYGON_OFFSET_FILL);
                    glDisable(GL_CULL_FACE);
                    glDepthFunc(GL_ALWAYS);

                    SETSHADER(tetraclear);
                    glBegin(GL_TRIANGLES);
                    switch(side)
                    {
                    case 0: glVertex3f(1, 1, 1); glVertex3f(-1, 1, 1); glVertex3f(1, -1, 1); break;
                    case 1: glVertex3f(-1, -1, 1); glVertex3f(-1, 1, 1); glVertex3f(1, -1, 1); break;
                    case 2: glVertex3f(1, -1, 1); glVertex3f(-1, -1, 1); glVertex3f(1, 1, 1); break;
                    case 3: glVertex3f(-1, 1, 1); glVertex3f(-1, -1, 1); glVertex3f(1, 1, 1); break;
                    }
                    glEnd();

                    glDepthFunc(GL_LESS);
                    glEnable(GL_CULL_FACE);
                    if(smpolyfactor || smpolyoffset) glEnable(GL_POLYGON_OFFSET_FILL);
                    if(smtetraclip) glEnable(GL_CLIP_PLANE0);
                }

                smviewmatrix.mul(tetrashadowviewmatrix[side], lightmatrix);
                glLoadMatrixf(smviewmatrix.v);

                glCullFace((side>>1) ^ smcullside ? GL_BACK : GL_FRONT);

                if(smtetraclip)
                {
                    smtetraclipplane.toplane(vec(-smviewmatrix.v[2], -smviewmatrix.v[6], 0), l.o);
                    smtetraclipplane.offset += smtetraborder/(0.5f*sm.size);
                    GLOBALPARAM(tetraclip, (smtetraclipplane));
                }

                shadowside = side;

                rendershadowmapworld();
                rendermodelbatches();
            }
        }
        else
        {
            if(!cachemask)
            {
                int cx1 = sidemask & 0x03 ? 0 : (sidemask & 0xC ? sm.size : 2 * sm.size),
                    cx2 = sidemask & 0x30 ? 3 * sm.size : (sidemask & 0xC ? 2 * sm.size : sm.size),
                    cy1 = sidemask & 0x15 ? 0 : sm.size,
                    cy2 = sidemask & 0x2A ? 2 * sm.size : sm.size;
                glScissor(sm.x + cx1, sm.y + cy1, cx2 - cx1, cy2 - cy1);
                glClear(GL_DEPTH_BUFFER_BIT);
            }
            loop(side, 6) if(sidemask&(1<<side))
            {
                int sidex = (side>>1)*sm.size, sidey = (side&1)*sm.size;
                glViewport(sm.x + sidex, sm.y + sidey, sm.size, sm.size);
                glScissor(sm.x + sidex, sm.y + sidey, sm.size, sm.size);
                if(cachemask) glClear(GL_DEPTH_BUFFER_BIT);

                smviewmatrix.mul(cubeshadowviewmatrix[side], lightmatrix);
                glLoadMatrixf(smviewmatrix.v);

                glCullFace((side & 1) ^ (side >> 2) ^ smcullside ? GL_FRONT : GL_BACK);

                shadowside = side;

                rendershadowmapworld();
                rendermodelbatches();
            }
        }

        clearbatchedmapmodels();
    }

    if(shadowmapping == SM_TETRA && smtetraclip) glDisable(GL_CLIP_PLANE0);
    if(smpolyfactor || smpolyoffset) glDisable(GL_POLYGON_OFFSET_FILL);

    shadowmapping = 0;
}

void rendershadowatlas()
{
    timer_begin(TIMER_CPU_SM);
    timer_begin(TIMER_SM);

    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, shadowatlasfbo);
    //glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

    if(debugshadowatlas)
    {
        glClearDepth(0);
        glClear(GL_DEPTH_BUFFER_BIT);
        glClearDepth(1);
    }

    glEnable(GL_SCISSOR_TEST);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    // sun light
    if(sunlight && csmshadowmap) 
    {
        csm.setup();
        rendercsmshadowmaps();
    }

    packlights();

    // point lights
    rendershadowmaps();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);
    glDisable(GL_SCISSOR_TEST);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    timer_end(TIMER_SM);
    timer_end(TIMER_CPU_SM);
}

void setbilateralshader(int radius, int pass, float sigma, float depth, bool linear, bool packed)
{
    float stepx = linear ? 1 : float(vieww)/aow, stepy = linear ? 1 : float(viewh)/aoh;
    bilateralshader[pass]->set();
    sigma *= 2*radius;
    LOCALPARAM(bilateralparams, (1.0f/(2*sigma*sigma), 1.0f/(depth*depth), pass==0 ? stepx : 0, pass==1 ? stepy : 0));
}

void renderao()
{
    timer_begin(TIMER_AO);

    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, gdepthtex);

    bool linear = hasTRG && hasTF && aoreducedepth && (aoreduce || aoreducedepth > 1);
    float xscale = eyematrix.v[0], yscale = eyematrix.v[5];
    if(linear)
    {
        glBindFramebuffer_(GL_FRAMEBUFFER_EXT, aofbo[2]);
        glViewport(0, 0, aow, aoh);
        SETSHADER(linearizedepth);
        screenquad(vieww, viewh);

        xscale *= float(vieww)/aow;
        yscale *= float(viewh)/aoh;

        glBindTexture(GL_TEXTURE_RECTANGLE_ARB, aotex[2]);
    }

    ambientobscuranceshader->set();

    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, aofbo[0]);
    glViewport(0, 0, aow, aoh);
    glActiveTexture_(GL_TEXTURE1_ARB);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, gnormaltex);
    glActiveTexture_(GL_TEXTURE2_ARB);
    glBindTexture(GL_TEXTURE_2D, aonoisetex);
    glActiveTexture_(GL_TEXTURE0_ARB);

    LOCALPARAM(tapparams, (aoradius*eyematrix.v[14]/xscale, aoradius*eyematrix.v[14]/yscale, aoradius*aoradius*aocutoff*aocutoff));
    LOCALPARAM(contrastparams, ((2.0f*aodark)/aotaps, aosharp));
    LOCALPARAM(offsetscale, (xscale/eyematrix.v[14], yscale/eyematrix.v[14], eyematrix.v[12]/eyematrix.v[14], eyematrix.v[13]/eyematrix.v[14]));
    screenquad(vieww, viewh, aow/float(1<<aonoise), aoh/float(1<<aonoise));

    if(aobilateral)
    {
        if(!linear && aopackdepth && hasTRG && hasTF) linear = true;
        loopi(2 + 2*aoiter)
        {
            setbilateralshader(aobilateral, i%2, aobilateralsigma, aobilateraldepth, linear, linear && aopackdepth);
            glBindFramebuffer_(GL_FRAMEBUFFER_EXT, aofbo[(i+1)%2]);
            glViewport(0, 0, aow, aoh);
            glBindTexture(GL_TEXTURE_RECTANGLE_ARB, aotex[i%2]);
            if(!linear || !aopackdepth)
            {
                glActiveTexture_(GL_TEXTURE1_ARB);
                glBindTexture(GL_TEXTURE_RECTANGLE_ARB, linear ? aotex[2] : gdepthtex);
                glActiveTexture_(GL_TEXTURE0_ARB);
            }
            screenquad(vieww, viewh);
        }
    }
    else if(aoblur)
    {
        float blurweights[MAXBLURRADIUS+1], bluroffsets[MAXBLURRADIUS+1];
        setupblurkernel(aoblur, aosigma, blurweights, bluroffsets);
        loopi(2 + 2*aoiter)
        {
            glBindFramebuffer_(GL_FRAMEBUFFER_EXT, aofbo[(i+1)%2]);
            glViewport(0, 0, aow, aoh);
            setblurshader(i%2, 1, aoblur, blurweights, bluroffsets, GL_TEXTURE_RECTANGLE_ARB);
            glBindTexture(GL_TEXTURE_RECTANGLE_ARB, aotex[i%2]);
            screenquad(aow, aoh);
        }
    }

    timer_end(TIMER_AO);
}

void processhdr(GLuint outfbo = 0)
{
    if(!hdr)
    {
        glBindFramebuffer_(GL_FRAMEBUFFER_EXT, outfbo);
        glViewport(0, 0, vieww, viewh);
        SETSHADER(hdrnop);
        glBindTexture(GL_TEXTURE_RECTANGLE_ARB, hdrtex);
        screenquad(vieww, viewh);
        return;
    }

    timer_begin(TIMER_HDR);

    GLOBALPARAM(hdrparams, (hdrbright, hdrsaturate, bloomthreshold, bloomscale));
    GLOBALPARAM(hdrgamma, (hdrgamma, 1.0f/hdrgamma));

    GLuint b0fbo = bloomfbo[1], b0tex = bloomtex[1], b1fbo =  bloomfbo[0], b1tex = bloomtex[0], ptex = hdrtex;
    int b0w = max(vieww/4, bloomw), b0h = max(viewh/4, bloomh), b1w = max(vieww/2, bloomw), b1h = max(viewh/2, bloomh),
        pw = vieww, ph = viewh;
    if(hdrreduce) while(pw > bloomw || ph > bloomh)
    {
        GLuint cfbo = b1fbo, ctex = b1tex;
        int cw = max(pw/2, bloomw), ch = max(ph/2, bloomh);

        if(hdrreduce > 1 && cw/2 >= bloomw && ch/2 >= bloomh)
        {
            cw /= 2;
            ch /= 2;
            SETSHADER(hdrreduce2);
        }
        else SETSHADER(hdrreduce);
        if(cw == bloomw && ch == bloomh) { if(bloomfbo[5]) { cfbo = bloomfbo[5]; ctex = bloomtex[5]; } else { cfbo = bloomfbo[2]; ctex = bloomtex[2]; } }
        glBindFramebuffer_(GL_FRAMEBUFFER_EXT, cfbo);
        glViewport(0, 0, cw, ch);
        glBindTexture(GL_TEXTURE_RECTANGLE_ARB, ptex);
        screenquad(pw, ph);

        ptex = ctex;
        pw = cw;
        ph = ch;
        swap(b0fbo, b1fbo);
        swap(b0tex, b1tex);
        swap(b0w, b1w);
        swap(b0h, b1h);
    }

    if(!lasthdraccum || lastmillis - lasthdraccum >= hdraccummillis)
    {
        GLuint ltex = ptex;
        int lw = pw, lh = ph;
        for(int i = 0; lw > 2 || lh > 2; i++)
        {
            int cw = max(lw/2, 2), ch = max(lh/2, 2);

            if(hdrreduce > 1 && cw/2 >= 2 && ch/2 >= 2)
            {
                cw /= 2;
                ch /= 2;
                if(i) SETSHADER(hdrreduce2); else SETSHADER(hdrluminance2);
            }
            else if(i) SETSHADER(hdrreduce); else SETSHADER(hdrluminance);
            glBindFramebuffer_(GL_FRAMEBUFFER_EXT, b1fbo);
            glViewport(0, 0, cw, ch);
            glBindTexture(GL_TEXTURE_RECTANGLE_ARB, ltex);
            screenquad(lw, lh);

            ltex = b1tex;
            lw = cw;
            lh = ch;
            swap(b0fbo, b1fbo);
            swap(b0tex, b1tex);
            swap(b0w, b1w);
            swap(b0h, b1h);
        }
   
        glBindFramebuffer_(GL_FRAMEBUFFER_EXT, bloomfbo[4]);
        glViewport(0, 0, 1, 1);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);
        SETSHADER(hdraccum);
        glBindTexture(GL_TEXTURE_RECTANGLE_ARB, b0tex);
        LOCALPARAM(accumscale, (lasthdraccum ? pow(hdraccumscale, float(lastmillis - lasthdraccum)/hdraccummillis) : 0));
        screenquad(2, 2);
        glDisable(GL_BLEND);

        lasthdraccum = lastmillis;
    }

    b0fbo = bloomfbo[3];
    b0tex = bloomtex[3];
    b1fbo = bloomfbo[2];
    b1tex = bloomtex[2];
    b0w = b1w = bloomw;
    b0h = b1h = bloomh;

    glActiveTexture_(GL_TEXTURE2_ARB);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, bloomtex[4]);
    glActiveTexture_(GL_TEXTURE0_ARB);

    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, b0fbo);
    glViewport(0, 0, b0w, b0h);
    SETSHADER(hdrbloom);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, ptex);
    screenquad(pw, ph);

    if(bloomblur)
    {
        float blurweights[MAXBLURRADIUS+1], bluroffsets[MAXBLURRADIUS+1];
        setupblurkernel(bloomblur, bloomsigma, blurweights, bluroffsets);
        loopi(2 + 2*bloomiter)
        {
            glBindFramebuffer_(GL_FRAMEBUFFER_EXT, b1fbo);
            glViewport(0, 0, b1w, b1h);
            setblurshader(i%2, 1, bloomblur, blurweights, bluroffsets, GL_TEXTURE_RECTANGLE_ARB);
            glBindTexture(GL_TEXTURE_RECTANGLE_ARB, b0tex);
            screenquad(b0w, b0h);
            swap(b0w, b1w);
            swap(b0h, b1h);
            swap(b0tex, b1tex);
            swap(b0fbo, b1fbo);
        }
    }

    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, outfbo);
    glViewport(0, 0, vieww, viewh);
    SETSHADER(hdrtonemap);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, hdrtex);
    glActiveTexture_(GL_TEXTURE1_ARB);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, b0tex);
    glActiveTexture_(GL_TEXTURE0_ARB);
    screenquad(vieww, viewh, b0w, b0h);
    timer_end(TIMER_HDR);
}

void dosmaa()
{
    timer_begin(TIMER_SMAA);

    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, smaafbo[1]);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    if(smaadepthmask) 
    {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_ALWAYS);
        glDepthRange(1, 1);
    }
    else if(smaastencil && ((gdepthstencil && hasDS) || gstencil)) 
    {
        glEnable(GL_STENCIL_TEST);
        glStencilFunc(GL_ALWAYS, 8, 8);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    }
    if(smaacoloredge) smaacoloredgeshader->set();
    else smaalumaedgeshader->set();
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, gcolortex);
    screenquad(vieww, viewh);

    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, smaafbo[2]);
    if(smaadepthmask)
    {
        glDepthFunc(GL_EQUAL);
        glDepthMask(GL_FALSE);
    }
    else if(smaastencil && ((gdepthstencil && hasDS) || gstencil))
    {
        glStencilFunc(GL_EQUAL, 8, 8);
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    }
    glClear(GL_COLOR_BUFFER_BIT);
    smaablendweightshader->set();
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, gnormaltex);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glActiveTexture_(GL_TEXTURE1_ARB);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, smaaareatex);
    glActiveTexture_(GL_TEXTURE2_ARB);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, smaasearchtex);
    glActiveTexture_(GL_TEXTURE0_ARB);
    screenquad(vieww, viewh);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    if(smaadepthmask) 
    {
        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS);
        glDepthRange(0, 1);
    }
    else if(smaastencil && ((gdepthstencil && hasDS) || gstencil)) glDisable(GL_STENCIL_TEST);

    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, 0);
    smaaneighborhoodshader->set();
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, gcolortex);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glActiveTexture_(GL_TEXTURE1_ARB);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, gglowtex);
    glActiveTexture_(GL_TEXTURE0_ARB);
    screenquad(vieww, viewh);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    timer_end(TIMER_SMAA);
}

FVAR(refractmargin, 0, 0.1f, 1);
FVAR(refractdepth, 1e-3f, 16, 1e3f);

void rendertransparent()
{
    int hasalphavas = findalphavas();
    int hasmats = findmaterials();
    if(!hasalphavas && !hasmats) return;
        
    timer_begin(TIMER_TRANSPARENT);

    if(hasalphavas&4 || hasmats&4)
    {
        glBindFramebuffer_(GL_FRAMEBUFFER_EXT, refractfbo);
        glDepthMask(GL_FALSE);
        glBindTexture(GL_TEXTURE_RECTANGLE_ARB, gdepthtex);
        float sx1 = min(alpharefractsx1, matrefractsx1), sy1 = min(alpharefractsy1, matrefractsy1),
              sx2 = max(alpharefractsx2, matrefractsx2), sy2 = max(alpharefractsy2, matrefractsy2);
        bool scissor = sx1 > -1 || sy1 > -1 || sx2 < 1 || sy2 < 1;
        if(scissor)
        {
            int x1 = int(floor(max(sx1*0.5f+0.5f-refractmargin*viewh/vieww, 0.0f)*vieww)),
                y1 = int(floor(max(sy1*0.5f+0.5f-refractmargin, 0.0f)*viewh)),
                x2 = int(ceil(min(sx2*0.5f+0.5f+refractmargin*viewh/vieww, 1.0f)*vieww)),
                y2 = int(ceil(min(sy2*0.5f+0.5f+refractmargin, 1.0f)*viewh));
            glEnable(GL_SCISSOR_TEST);
            glScissor(x1, y1, x2 - x1, y2 - y1);
        }
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        if(scissor) glDisable(GL_SCISSOR_TEST);
        SETSHADER(refractmask);
		LOCALPARAM(refractdepth, (1.0f/refractdepth));
        if(hasalphavas&4) renderrefractmask();
        if(hasmats&4) rendermaterialmask();
        glDepthMask(GL_TRUE);
    }

    glActiveTexture_(GL_TEXTURE7_ARB);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, refracttex);
    glActiveTexture_(GL_TEXTURE8_ARB);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, hdrtex);
    glActiveTexture_(GL_TEXTURE9_ARB);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, gdepthtex);
    glActiveTexture_(GL_TEXTURE0_ARB);

    if((gdepthstencil && hasDS) || gstencil) glEnable(GL_STENCIL_TEST);

    glmatrixf raymatrix = mvmatrix.v;
    loopk(4)
    {
        raymatrix.v[0 + k*4] = 0.5f*vieww*(raymatrix.v[2 + k*4] - raymatrix.v[0 + k*4]*projmatrix.v[0]);
        raymatrix.v[1 + k*4] = 0.5f*viewh*(raymatrix.v[2 + k*4] - raymatrix.v[1 + k*4]*projmatrix.v[5]);
    }

    loop(layer, 3)
    {
        uint tiles[LIGHTTILE_H];
        float sx1, sy1, sx2, sy2;
        switch(layer)
        {
        case 0:
            if(!(hasmats&1)) continue;
            sx1 = matliquidsx1; sy1 = matliquidsy1; sx2 = matliquidsx2; sy2 = matliquidsy2;
            memcpy(tiles, matliquidtiles, sizeof(tiles));
            break;
        case 1:
            if(!(hasalphavas&1)) continue;
            sx1 = alphabacksx1; sy1 = alphabacksy1; sx2 = alphabacksx2; sy2 = alphabacksy2;
            memcpy(tiles, alphatiles, sizeof(tiles));
            break;
        case 2:
            if(!(hasalphavas&2) && !(hasmats&2)) continue;
            sx1 = alphafrontsx1; sy1 = alphafrontsy1; sx2 = alphafrontsx2; sy2 = alphafrontsy2;
            memcpy(tiles, alphatiles, sizeof(tiles));
            if(hasmats&2)
            {
                sx1 = min(sx1, matsolidsx1);
                sy1 = min(sy1, matsolidsy1);
                sx2 = max(sx2, matsolidsx2);
                sy2 = max(sy2, matsolidsy2);
                loopj(LIGHTTILE_H) tiles[j] |= matsolidtiles[j];
            }
            break;
        default:
            continue;
        }

        glBindFramebuffer_(GL_FRAMEBUFFER_EXT, gfbo);
        if((gdepthstencil && hasDS) || gstencil)
        {
            glStencilFunc(GL_ALWAYS, 1<<layer, 1<<layer);
            glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        }
        else
        {
            maskgbuffer("cng");
            bool scissor = sx1 > -1 || sy1 > -1 || sx2 < 1 || sy2 < 1;
            if(scissor)
            {
                int x1 = int(floor((sx1*0.5f+0.5f)*vieww)), y1 = int(floor((sy1*0.5f+0.5f)*viewh)),
                    x2 = int(ceil((sx2*0.5f+0.5f)*vieww)), y2 = int(ceil((sy2*0.5f+0.5f)*viewh));
                glEnable(GL_SCISSOR_TEST);
                glScissor(x1, y1, x2 - x1, y2 - y1);
            }
            glClearColor(0, 0, 0, 0);
            glClear(GL_COLOR_BUFFER_BIT);
            if(scissor) glDisable(GL_SCISSOR_TEST);
            maskgbuffer("cngd");
        }

        if(wireframe && editmode) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        glMatrixMode(GL_TEXTURE);
        glLoadMatrixf(linearworldmatrix.v);
        glActiveTexture_(GL_TEXTURE1_ARB);
        glLoadMatrixf(raymatrix.v);
        glActiveTexture_(GL_TEXTURE0_ARB);

        switch(layer)
        {
        case 0:
            if(hasmats&1) renderliquidmaterials();
            break;
        case 1:
            if(hasalphavas&1) renderalphageom(1);
            break;
        case 2:
            if(hasalphavas&2) renderalphageom(2);
            if(hasmats&2) rendersolidmaterials();
            break;
        }

        if(wireframe && editmode) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        glBindFramebuffer_(GL_FRAMEBUFFER_EXT, hdrfbo);
        if((gdepthstencil && hasDS) || gstencil)
        {
            glStencilFunc(GL_EQUAL, 1<<layer, 1<<layer);
            glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
        }

        renderlights(sx1, sy1, sx2, sy2, tiles);
    }

    if((gdepthstencil && hasDS) || gstencil) glDisable(GL_STENCIL_TEST);

    timer_end(TIMER_TRANSPARENT);
}

VAR(gdepthclear, 0, 1, 1);
VAR(gcolorclear, 0, 1, 1);

void rendergbuffer()
{
    timer_begin(TIMER_CPU_GBUFFER);
    timer_begin(TIMER_GBUFFER);

    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, gfbo);
    glViewport(0, 0, vieww, viewh);

    if(gdepthformat && gdepthclear)
    {
        maskgbuffer("d");
        if(gdepthformat == 1) glClearColor(1, 1, 1, 1);
        else glClearColor(-farplane, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0, 0, 0, 0);
        maskgbuffer("cng");
    }
    glClear((minimapping < 2 ? GL_DEPTH_BUFFER_BIT : 0)|(gcolorclear ? GL_COLOR_BUFFER_BIT : 0)|(minimapping < 2 && ((gdepthstencil && hasDS) || gstencil) ? GL_STENCIL_BUFFER_BIT : 0));
    if(gdepthformat && gdepthclear) maskgbuffer("cngd");

    glmatrixf screenmatrix;
    screenmatrix.identity();
    screenmatrix.scale(2.0f/vieww, 2.0f/viewh, 2.0f);
    screenmatrix.translate(-1.0f, -1.0f, -1.0f);
    eyematrix.mul(invprojmatrix, screenmatrix);
    if(minimapping)
    {
        linearworldmatrix.mul(invmvpmatrix, screenmatrix);
        worldmatrix = linearworldmatrix;
        glScissor(1, 1, vieww-2, viewh-2);
        glEnable(GL_SCISSOR_TEST);
    }
    else
    {
        linearworldmatrix.mul(invprojmatrix, screenmatrix);
        float xscale = linearworldmatrix.v[0], yscale = linearworldmatrix.v[5], xoffset = linearworldmatrix.v[12], yoffset = linearworldmatrix.v[13], zscale = linearworldmatrix.v[14];
        GLfloat depthmatrix[16] =
        {
            xscale/zscale,  0,              0, 0,
            0,              yscale/zscale,  0, 0,
            xoffset/zscale, yoffset/zscale, 1, 0,
            0,              0,              0, 1
        };
        linearworldmatrix.mul(invmvmatrix.v, depthmatrix);
        if(gdepthformat) worldmatrix = linearworldmatrix;
        else worldmatrix.mul(invmvpmatrix, screenmatrix);
    }

    GLOBALPARAM(viewsize, (vieww, viewh, 1.0f/vieww, 1.0f/viewh));
    GLOBALPARAM(gdepthscale, (eyematrix.v[14], eyematrix.v[11], eyematrix.v[15]));
    GLOBALPARAM(gdepthpackparams, (-1.0f/farplane, -255.0f/farplane, -(255.0f*255.0f)/farplane));
    GLOBALPARAM(gdepthunpackparams, (-farplane, -farplane/255.0f, -farplane/(255.0f*255.0f)));

    GLERROR;
    if(limitsky())
    {
        renderexplicitsky();
        GLERROR;
    }
    rendergeom();
    GLERROR;
    resetmodelbatches();
    rendermapmodels();
    GLERROR;
    if(minimapping)
    {
        renderminimapmaterials();
        glDisable(GL_SCISSOR_TEST);
    }
    else if(!envmapping)
    {
        game::rendergame();
        rendermodelbatches();
        GLERROR;
        maskgbuffer("c");
        renderdecals(true);
        maskgbuffer("cngd");
        GLERROR;
        if(!isthirdperson())
        {
            project(curavatarfov, aspect, farplane, avatardepth);
            game::renderavatar();
            project(fovy, aspect, farplane);
        }
        GLERROR;
    }

    timer_end(TIMER_GBUFFER);
    timer_end(TIMER_CPU_GBUFFER);
}

void shadegbuffer()
{
    GLERROR;
    timer_begin(TIMER_CPU_SHADING);
    timer_begin(TIMER_SHADING);

    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, hdrfbo);
    glViewport(0, 0, vieww, viewh);

    if(!minimapping) 
    {
        if(hdrclear > 0)
        {
            glClearColor(0, 0, 0, 0);
            glClear(GL_COLOR_BUFFER_BIT);
            hdrclear--;
        }
        drawskybox(farplane);
    }

    renderlights();
    GLERROR;

    timer_end(TIMER_SHADING);
    timer_end(TIMER_CPU_SHADING);
}

void drawminimap()
{
    if(!game::needminimap()) { clearminimap(); return; }

    renderprogress(0, "generating mini-map...", 0, !renderedframe);

    gl_setupframe(screen->w, screen->h);

    if(!deferredminimapshader) deferredminimapshader = loaddeferredlightshader(true);

    int size = 1<<minimapsize, sizelimit = min(hwtexsize, min(vieww, viewh));
    while(size > sizelimit) size /= 2;
    if(!minimaptex) glGenTextures(1, &minimaptex);

    extern vector<vtxarray *> valist;
    ivec bbmin(worldsize, worldsize, worldsize), bbmax(0, 0, 0);
    loopv(valist)
    {
        vtxarray *va = valist[i];
        loopk(3)
        {
            if(va->geommin[k]>va->geommax[k]) continue;
            bbmin[k] = min(bbmin[k], va->geommin[k]);
            bbmax[k] = max(bbmax[k], va->geommax[k]);
        }
    }
    if(minimapclip)
    {
        ivec clipmin(worldsize, worldsize, worldsize), clipmax(0, 0, 0);
        clipminimap(clipmin, clipmax);
        loopk(2) bbmin[k] = max(bbmin[k], clipmin[k]);
        loopk(2) bbmax[k] = min(bbmax[k], clipmax[k]);
    }

    minimapradius = bbmax.tovec().sub(bbmin.tovec()).mul(0.5f);
    minimapcenter = bbmin.tovec().add(minimapradius);
    minimapradius.x = minimapradius.y = max(minimapradius.x, minimapradius.y);
    minimapscale = vec((0.5f - 1.0f/size)/minimapradius.x, (0.5f - 1.0f/size)/minimapradius.y, 1.0f);

    envmapping = true;
    minimapping = 1;

    physent *oldcamera = camera1;
    static physent cmcamera;
    cmcamera = *player;
    cmcamera.reset();
    cmcamera.type = ENT_CAMERA;
    cmcamera.o = vec(minimapcenter.x, minimapcenter.y, minimapheight > 0 ? minimapheight : minimapcenter.z + minimapradius.z + 1);
    cmcamera.yaw = 0;
    cmcamera.pitch = -90;
    cmcamera.roll = 0;
    camera1 = &cmcamera;
    setviewcell(vec(-1, -1, -1));

    float oldldrscale = ldrscale, oldldrscaleb = ldrscaleb;
    int oldvieww = vieww, oldviewh = viewh;
    vieww = viewh = size;

    float zscale = max(float(minimapheight), minimapcenter.z + minimapradius.z + 1) + 1;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-minimapradius.x, minimapradius.x, -minimapradius.y, minimapradius.y, 0, 2*zscale);
    glMatrixMode(GL_MODELVIEW);

    transplayer();
    readmatrices();

    resetlights();

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    xtravertsva = xtraverts = glde = gbatches = vtris = vverts = 0;
    flipqueries();

    ldrscale = 1;
    ldrscaleb = ldrscale/255;
    GLOBALPARAM(ldrscale, (ldrscale));
    GLOBALPARAM(camera, (camera1->o.x, camera1->o.y, camera1->o.z, 1));
    GLOBALPARAM(millis, (lastmillis/1000.0f, lastmillis/1000.0f, lastmillis/1000.0f));

    visiblecubes(false);
    collectlights();
    rendershadowatlas();
    findmaterials();

    rendergbuffer();

    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, hdrfbo);
    glViewport(0, 0, vieww, viewh);
    glClearColor(minimapcolor.x*ldrscaleb, minimapcolor.y*ldrscaleb, minimapcolor.z*ldrscaleb, 0); 
    glClear(GL_COLOR_BUFFER_BIT);

    shadegbuffer();

    if(minimapheight > 0 && minimapheight < minimapcenter.z + minimapradius.z)
    {
        minimapping = 2;
        camera1->o.z = minimapcenter.z + minimapradius.z + 1;
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glTranslatef(0, 0, 1);
        glOrtho(-minimapradius.x, minimapradius.x, -minimapradius.y, minimapradius.y, 0, 2*zscale);
        glMatrixMode(GL_MODELVIEW);
        transplayer();
        readmatrices();
        rendergbuffer();
        shadegbuffer();
    }
        
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    vieww = oldvieww;
    viewh = oldviewh;
    ldrscale = oldldrscale;
    ldrscaleb = oldldrscaleb;

    camera1 = oldcamera;
    envmapping = false;
    minimapping = 0;

    if(hasCBF || !hdrfloat) glBindFramebuffer_(GL_FRAMEBUFFER_EXT, hdrfbo);
    else
    {
        glBindFramebuffer_(GL_FRAMEBUFFER_EXT, 0);
        SETSHADER(hdrnop);
        glBindTexture(GL_TEXTURE_RECTANGLE_ARB, hdrtex);
        screenquad(size, size);
    }

    glViewport(0, 0, vieww, viewh);

    glBindTexture(GL_TEXTURE_2D, minimaptex);
    if(hasCBF && hdrfloat) glClampColor_(GL_CLAMP_READ_COLOR_ARB, GL_TRUE);
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB5, 0, 0, size, size, 0);
    if(hasCBF && hdrfloat) glClampColor_(GL_CLAMP_READ_COLOR_ARB, GL_FIXED_ONLY_ARB);
    setuptexparameters(minimaptex, NULL, 3, 1, GL_RGB5, GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    GLfloat border[4] = { minimapcolor.x/255.0f, minimapcolor.y/255.0f, minimapcolor.z/255.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);
    glBindTexture(GL_TEXTURE_2D, 0);

    hdrclear = 3;

    glBindFramebuffer_(GL_FRAMEBUFFER_EXT, 0);
}

void drawcubemap(int size, const vec &o, float yaw, float pitch, const cubemapside &side)
{
    envmapping = true;

    physent *oldcamera = camera1;
    static physent cmcamera;
    cmcamera = *player;
    cmcamera.reset();
    cmcamera.type = ENT_CAMERA;
    cmcamera.o = o;
    cmcamera.yaw = yaw;
    cmcamera.pitch = pitch;
    cmcamera.roll = 0;
    camera1 = &cmcamera;
    setviewcell(camera1->o);

    float fogmargin = 1 + WATER_AMPLITUDE + nearplane;
    int fogmat = lookupmaterial(vec(camera1->o.x, camera1->o.y, camera1->o.z - fogmargin))&MATF_VOLUME, abovemat = MAT_AIR;
    float fogbelow = 0;
    if(fogmat==MAT_WATER || fogmat==MAT_LAVA)
    {
        float z = findsurface(fogmat, vec(camera1->o.x, camera1->o.y, camera1->o.z - fogmargin), abovemat) - WATER_OFFSET;
        if(camera1->o.z < z + fogmargin)
        {
            fogbelow = z - camera1->o.z;
        }
        else fogmat = abovemat;
    }
    else fogmat = MAT_AIR;
    setfog(abovemat);
    
    float oldaspect = aspect, oldfovy = fovy, oldfov = curfov, oldldrscale = ldrscale, oldldrscaleb = ldrscaleb;
    int oldfarplane = farplane, oldvieww = vieww, oldviewh = viewh;
    curfov = fovy = 90;
    aspect = 1;
    farplane = worldsize*2;
    vieww = viewh = size;
    project(90.0f, 1.0f, farplane);

    transplayer();
    readmatrices();
    findorientation();

    resetlights();

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    xtravertsva = xtraverts = glde = gbatches = vtris = vverts = 0;
    flipqueries();

    ldrscale = 1;
    ldrscaleb = ldrscale/255;
    GLOBALPARAM(ldrscale, (ldrscale));
    GLOBALPARAM(camera, (camera1->o.x, camera1->o.y, camera1->o.z, 1));
    GLOBALPARAM(millis, (lastmillis/1000.0f, lastmillis/1000.0f, lastmillis/1000.0f));

    visiblecubes();

    GLERROR;

    rendergbuffer();
    GLERROR;

    rendershadowatlas();
    GLERROR;

    shadegbuffer();
    GLERROR;

    if(fogmat)
    {
        setfog(fogmat, fogbelow, 1, abovemat);

        glActiveTexture_(GL_TEXTURE9_ARB);
        glBindTexture(GL_TEXTURE_RECTANGLE_ARB, gdepthtex);
        glActiveTexture_(GL_TEXTURE0_ARB);

        glBindFramebuffer_(GL_FRAMEBUFFER_EXT, hdrfbo);
        renderwaterfog(fogmat, fogbelow);
    
        setfog(fogmat, fogbelow, clamp(fogbelow, 0.0f, 1.0f), abovemat);
    }

    rendertransparent();
    GLERROR;

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    aspect = oldaspect;
    fovy = oldfovy;
    curfov = oldfov;
    farplane = oldfarplane;
    vieww = oldvieww;
    viewh = oldviewh;
    ldrscale = oldldrscale;
    ldrscaleb = oldldrscaleb;

    camera1 = oldcamera;
    envmapping = false;

    if(hasCBF) glBindFramebuffer_(GL_FRAMEBUFFER_EXT, hdrfbo);
    else
    {
        glBindFramebuffer_(GL_FRAMEBUFFER_EXT, 0);
        SETSHADER(hdrnop);
        glBindTexture(GL_TEXTURE_RECTANGLE_ARB, hdrtex);
        screenquad(size, size);
    }

    hdrclear = 3;
}

void gl_setupframe(int w, int h)
{
    setupgbuffer(w, h);
    if(hdr && (bloomw < 0 || bloomh < 0)) setupbloom(w, h);
    if(ao && (aow < 0 || aoh < 0)) setupao(w, h);
    if(smaa && !smaafbo[0]) setupsmaa();
    if(!shadowatlasfbo) setupshadowatlas();
    if(sunlight && csmshadowmap && gi && giscale && gidist && !rhfbo) setupradiancehints();
    if(!deferredlightshader) loaddeferredlightshaders();
}

void gl_drawframe(int w, int h)
{
    timer_sync();

    defaultshader->set();

    vieww = w;
    viewh = h;
    aspect = w/float(h);
    fovy = 2*atan2(tan(curfov/2*RAD), aspect)/RAD;
    
    float fogmargin = 1 + WATER_AMPLITUDE + nearplane;
    int fogmat = lookupmaterial(vec(camera1->o.x, camera1->o.y, camera1->o.z - fogmargin))&MATF_VOLUME, abovemat = MAT_AIR;
    float fogbelow = 0;
    if(fogmat==MAT_WATER || fogmat==MAT_LAVA)
    {
        float z = findsurface(fogmat, vec(camera1->o.x, camera1->o.y, camera1->o.z - fogmargin), abovemat) - WATER_OFFSET;
        if(camera1->o.z < z + fogmargin)
        {
            fogbelow = z - camera1->o.z;
        }
        else fogmat = abovemat;
    }
    else fogmat = MAT_AIR;    
    setfog(abovemat);
    //setfog(fogmat, fogbelow, 1, abovemat);

    farplane = worldsize*2;

    resetlights();

    if(!debugcsm)
    {
        project(fovy, aspect, farplane);
        transplayer();
    }
    else
    {
        glMatrixMode(GL_PROJECTION);
        const int splitid = min(debugcsm-1, csmsplitn-1);
        glLoadMatrixf(csm.splits[splitid].proj.v);
        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixf(csm.model.v);
    }

    readmatrices();
    findorientation();

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    xtravertsva = xtraverts = glde = gbatches = vtris = vverts = 0;
    flipqueries();

    ldrscale = hdr ? 0.5f : 1;
    ldrscaleb = ldrscale/255;
    GLOBALPARAM(ldrscale, (ldrscale));
    GLOBALPARAM(camera, (camera1->o.x, camera1->o.y, camera1->o.z, 1));
    GLOBALPARAM(millis, (lastmillis/1000.0f, lastmillis/1000.0f, lastmillis/1000.0f));

    visiblecubes();
  
    if(wireframe && editmode) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
 
    rendergbuffer();

    if(wireframe && editmode) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    else if(limitsky() && editmode) renderexplicitsky(true);

    if(ao) 
    {
        renderao();
        glBindFramebuffer_(GL_FRAMEBUFFER_EXT, gfbo);
        glViewport(0, 0, vieww, viewh);
        GLERROR;
    }
 
    // render grass after AO to avoid disturbing shimmering patterns
    generategrass();
    rendergrass();
    GLERROR;

    if(!rhinoq) 
    {
        renderradiancehints();
        GLERROR;
    }

    rendershadowatlas();
    GLERROR;

    shadegbuffer();
    GLERROR;

    if(fogmat)
    {
        setfog(fogmat, fogbelow, 1, abovemat);

        glActiveTexture_(GL_TEXTURE9_ARB);
        glBindTexture(GL_TEXTURE_RECTANGLE_ARB, gdepthtex);
        glActiveTexture_(GL_TEXTURE0_ARB);

        glBindFramebuffer_(GL_FRAMEBUFFER_EXT, hdrfbo);
        renderwaterfog(fogmat, fogbelow);

        setfog(fogmat, fogbelow, clamp(fogbelow, 0.0f, 1.0f), abovemat);
    }

    rendertransparent();
    GLERROR;

    if(fogmat) setfog(fogmat, fogbelow, 1, abovemat);

    defaultshader->set();
    GLERROR;

    extern int outline;
    if(!wireframe && editmode && outline) renderoutline();
    GLERROR;

    rendereditmaterials();
    GLERROR;

    glActiveTexture_(GL_TEXTURE2_ARB);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, gdepthtex);
    glActiveTexture_(GL_TEXTURE0_ARB);

    renderparticles(true);
    GLERROR;

    extern int hidehud;
    if(editmode && !hidehud)
    {
        glDepthMask(GL_FALSE);
        renderblendbrush();
        rendereditcursor();
        glDepthMask(GL_TRUE);
    }
        
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);

    processhdr(smaa ? smaafbo[0] : 0);
    if(smaa) dosmaa();

    addmotionblur();
    if(fogoverlay && fogmat != MAT_AIR) drawfogoverlay(fogmat, fogbelow, clamp(fogbelow, 0.0f, 1.0f), abovemat);
    renderpostfx();

    defaultshader->set();
    gui::render();

    notextureshader->set();

    gl_drawhud(w, h);

    timer_nextframe();
}

void gl_drawmainmenu(int w, int h)
{
    xtravertsva = xtraverts = glde = gbatches = vtris = vverts = 0;

    renderbackground(NULL, NULL, NULL, NULL, true, true);
    renderpostfx();
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    defaultshader->set();
    gui::render();

    notextureshader->set();

    gl_drawhud(w, h);
}

VARNP(damagecompass, usedamagecompass, 0, 1, 1);
VARP(damagecompassfade, 1, 1000, 10000);
VARP(damagecompasssize, 1, 30, 100);
VARP(damagecompassalpha, 1, 25, 100);
VARP(damagecompassmin, 1, 25, 1000);
VARP(damagecompassmax, 1, 200, 1000);

float dcompass[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
void damagecompass(int n, const vec &loc)
{
    if(!usedamagecompass) return;
    vec delta(loc);
    delta.sub(camera1->o); 
    float yaw, pitch;
    if(delta.magnitude()<4) yaw = camera1->yaw;
    else vectoyawpitch(delta, yaw, pitch);
    yaw -= camera1->yaw;
    if(yaw >= 360) yaw = fmod(yaw, 360);
    else if(yaw < 0) yaw = 360 - fmod(-yaw, 360);
    int dir = (int(yaw+22.5f)%360)/45;
    dcompass[dir] += max(n, damagecompassmin)/float(damagecompassmax);
    if(dcompass[dir]>1) dcompass[dir] = 1;

}
void drawdamagecompass(int w, int h)
{
    int dirs = 0;
    float size = damagecompasssize/100.0f*min(h, w)/2.0f;
    loopi(8) if(dcompass[i]>0)
    {
        if(!dirs)
        {
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glColor4f(1, 0, 0, damagecompassalpha/100.0f);
        }
        dirs++;

        glPushMatrix();
        glTranslatef(w/2, h/2, 0);
        glRotatef(i*45, 0, 0, 1);
        glTranslatef(0, -size/2.0f-min(h, w)/4.0f, 0);
        float logscale = 32,
              scale = log(1 + (logscale - 1)*dcompass[i]) / log(logscale);
        glScalef(size*scale, size*scale, 0);

        glBegin(GL_TRIANGLES);
        glVertex3f(1, 1, 0);
        glVertex3f(-1, 1, 0);
        glVertex3f(0, 0, 0);
        glEnd();
        glPopMatrix();

        // fade in log space so short blips don't disappear too quickly
        scale -= float(curtime)/damagecompassfade;
        dcompass[i] = scale > 0 ? (pow(logscale, scale) - 1) / (logscale - 1) : 0;
    }
}

int damageblendmillis = 0;

VARFP(damagescreen, 0, 1, 1, { if(!damagescreen) damageblendmillis = 0; });
VARP(damagescreenfactor, 1, 7, 100);
VARP(damagescreenalpha, 1, 45, 100);
VARP(damagescreenfade, 0, 125, 1000);
VARP(damagescreenmin, 1, 10, 1000);
VARP(damagescreenmax, 1, 100, 1000);

void damageblend(int n)
{
    if(!damagescreen) return;
    if(lastmillis > damageblendmillis) damageblendmillis = lastmillis;
    damageblendmillis += clamp(n, damagescreenmin, damagescreenmax)*damagescreenfactor;
}

void drawdamagescreen(int w, int h)
{
    if(lastmillis >= damageblendmillis) return;

    defaultshader->set();
    glEnable(GL_TEXTURE_2D);

    static Texture *damagetex = NULL;
    if(!damagetex) damagetex = textureload("data/textures/hud/damage.png", 3);

    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glBindTexture(GL_TEXTURE_2D, damagetex->id);
    float fade = damagescreenalpha/100.0f;
    if(damageblendmillis - lastmillis < damagescreenfade)
        fade *= float(damageblendmillis - lastmillis)/damagescreenfade;
    glColor4f(fade, fade, fade, fade);

    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(0, 0); glVertex2f(0, 0);
    glTexCoord2f(1, 0); glVertex2f(w, 0);
    glTexCoord2f(0, 1); glVertex2f(0, h);
    glTexCoord2f(1, 1); glVertex2f(w, h);
    glEnd();

    glDisable(GL_TEXTURE_2D);
    notextureshader->set();
}

VAR(hidestats, 0, 0, 1);
VAR(hidehud, 0, 0, 1);

VARP(crosshairsize, 0, 15, 50);
VARP(cursorsize, 0, 10, 50);
VARP(crosshairfx, 0, 1, 1);

void drawcrosshair(int w, int h)
{
    bool windowhit = gui::hascursor(false) || !GuiControl::isMouselooking(); // INTENSITY: Mouselooking
    if(!windowhit && (hidehud || gui::mainmenu)) return;

    float r = 1, g = 1, b = 1, cx = 0.5f, cy = 0.5f, chsize;
    Texture *crosshair;
    if(windowhit)
    {
        static Texture *cursor = NULL;
        if(!cursor) cursor = textureload("data/textures/ui/guicursor.png", 3, true);
        crosshair = cursor;
        chsize = cursorsize*w/900.0f;
        gui::getcursorpos(cx, cy);
    }
    else
    { 
        const char *crname = ""; // OF: Start script-controlled crosshairs
        varsys::Variable *ev = varsys::get("crosshair");
        if (!ev || (ev->type() != varsys::TYPE_S))
            crname = "data/textures/hud/crosshair.png";
        else
            crname = ((varsys::String_Alias*)ev)->get();

        crosshair = textureload(crname, 3, true, false);
        if (crosshair == notexture) return;

        chsize = crosshairsize*w/900.0f;
    }
    if(crosshair->type&Texture::ALPHA) glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    else glBlendFunc(GL_ONE, GL_ONE);
    glColor3f(r, g, b);
    float x = cx*w - (windowhit ? 0 : chsize/2.0f);
    float y = cy*h - (windowhit ? 0 : chsize/2.0f);
    glBindTexture(GL_TEXTURE_2D, crosshair->id);
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(0, 0); glVertex2f(x,          y);
    glTexCoord2f(1, 0); glVertex2f(x + chsize, y);
    glTexCoord2f(0, 1); glVertex2f(x,          y + chsize);
    glTexCoord2f(1, 1); glVertex2f(x + chsize, y + chsize);
    glEnd();
}

VARP(wallclock, 0, 0, 1);
VARP(wallclock24, 0, 0, 1);
VARP(wallclocksecs, 0, 0, 1);

static time_t walltime = 0;

types::String getwallclock()
{
    if(wallclock)
    {
        if(!walltime) { walltime = time(NULL); walltime -= totalmillis/1000; if(!walltime) walltime++; }
        time_t walloffset = walltime + totalmillis/1000;
        struct tm *localvals = localtime(&walloffset);
        static string buf;
        if(localvals && strftime(buf, sizeof(buf), wallclocksecs ? (wallclock24 ? "%H:%M:%S" : "%I:%M:%S%p") : (wallclock24 ? "%H:%M" : "%I:%M%p"), localvals))
        {
            // hack because not all platforms (windows) support %P lowercase option
            // also strip leading 0 from 12 hour time
            char *dst = buf;
            const char *src = &buf[!wallclock24 && buf[0]=='0' ? 1 : 0];
            while(*src) *dst++ = tolower(*src++);
            *dst++ = '\0';

            return buf;
        }
    }
    return types::String();
}

VARP(showfps, 0, 1, 1);
VARP(showfpsrange, 0, 0, 1);
VAR(showeditstats, 0, 0, 1);
VAR(statrate, 1, 200, 1000);

FVARP(conscale, 1e-3f, 0.33f, 1e3f);

void gl_drawhud(int w, int h)
{
    gettextres(w, h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, w, h, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    glColor3f(1, 1, 1);

    if(debugshadowatlas) viewshadowatlas();
    else if(debugbuffersplit) viewbuffersplitmerge();
    else if(debugao) viewao(); 
    else if(debugdepth) viewdepth();
    else if(debugrefract) viewrefract();
    else if(debugsmaa) viewsmaa();
    else if(debuglightscissor) viewlightscissor();
    else if(debugrsm) viewrsm();
    else if(debugrh) viewrh();

    glEnable(GL_BLEND);

    if(!gui::mainmenu)
    {
        drawdamagescreen(w, h);
        drawdamagecompass(w, h);
    }

    glEnable(GL_TEXTURE_2D);
    defaultshader->set();

    int conw = int(w/conscale), conh = int(h/conscale), abovehud = conh - FONTH;
    if(!hidehud && !gui::mainmenu)
    {
        if(!hidestats)
        {
            glPushMatrix();
            glScalef(conscale, conscale, 1);

            int roffset = 0;
            if(showfps)
            {
                static int lastfps = 0, prevfps[3] = { 0, 0, 0 }, curfps[3] = { 0, 0, 0 };
                if(totalmillis - lastfps >= statrate)
                {
                    memcpy(prevfps, curfps, sizeof(prevfps));
                    lastfps = totalmillis - (totalmillis%statrate);
                }
                int nextfps[3];
                getfps(nextfps[0], nextfps[1], nextfps[2]);
                loopi(3) if(prevfps[i]==curfps[i]) curfps[i] = nextfps[i];
                if(showfpsrange) draw_textf("fps %d+%d-%d", conw-7*FONTH, conh-FONTH*3/2, curfps[0], curfps[1], curfps[2]);
                else draw_textf("fps %d", conw-5*FONTH, conh-FONTH*3/2, curfps[0]);
                roffset += FONTH;
            }

            timer_print(conw, conh);
            frametimer_print(conw, conh);

            if(wallclock)
            {
                if(!walltime) { walltime = time(NULL); walltime -= totalmillis/1000; if(!walltime) walltime++; }
                time_t walloffset = walltime + totalmillis/1000;
                struct tm *localvals = localtime(&walloffset);
                static string buf;
                if(localvals && strftime(buf, sizeof(buf), wallclocksecs ? (wallclock24 ? "%H:%M:%S" : "%I:%M:%S%p") : (wallclock24 ? "%H:%M" : "%I:%M%p"), localvals))
                {
                    // hack because not all platforms (windows) support %P lowercase option
                    // also strip leading 0 from 12 hour time
                    char *dst = buf;
                    const char *src = &buf[!wallclock24 && buf[0]=='0' ? 1 : 0];
                    while(*src) *dst++ = tolower(*src++);
                    *dst++ = '\0'; 
                    draw_text(buf, conw-5*FONTH, conh-FONTH*3/2-roffset);
                    roffset += FONTH;
                }
            }
                       
            if(editmode || showeditstats)
            {
                static int laststats = 0, prevstats[7] = { 0, 0, 0, 0, 0, 0, 0 }, curstats[7] = { 0, 0, 0, 0, 0, 0, 0 };
                if(totalmillis - laststats >= statrate)
                {
                    memcpy(prevstats, curstats, sizeof(prevstats));
                    laststats = totalmillis - (totalmillis%statrate);
                }
                int nextstats[7] =
                {
                    vtris*100/max(wtris, 1),
                    vverts*100/max(wverts, 1),
                    xtraverts/1024,
                    xtravertsva/1024,
                    glde,
                    gbatches,
                    getnumqueries()
                };
                loopi(7) if(prevstats[i]==curstats[i]) curstats[i] = nextstats[i];

                abovehud -= 2*FONTH;
                draw_textf("wtr:%dk(%d%%) wvt:%dk(%d%%) evt:%dk eva:%dk", FONTH/2, abovehud, wtris/1024, curstats[0], wverts/1024, curstats[1], curstats[2], curstats[3]);
                draw_textf("ond:%d va:%d gl:%d(%d) oq:%d pvs:%d", FONTH/2, abovehud+FONTH, allocnodes*8, allocva, curstats[4], curstats[5], curstats[6], getnumviewcells());
            }

            if(editmode)
            {
                abovehud -= FONTH;
                draw_textf("cube %s%d", FONTH/2, abovehud, selchildcount<0 ? "1/" : "", abs(selchildcount));

                const char *editinfo = lapi::state.get<lua::Function>(
                    "LAPI", "GUI", "HUD", "edit"
                ).call<const char*>();
                if(editinfo && editinfo[0])
                {
                    int tw, th;
                    text_bounds(editinfo, tw, th);
                    th += FONTH-1; th -= th%FONTH;
                    abovehud -= max(th, FONTH);
                    draw_text(editinfo, FONTH/2, abovehud);
                }
            }
            else
            {
                const char *gameinfo = lapi::state.get<lua::Function>(
                    "LAPI", "GUI", "HUD", "game"
                ).call<const char*>();
                if(gameinfo && gameinfo[0])
                {
                    int tw, th;
                    text_bounds(gameinfo, tw, th);
                    th += FONTH-1; th -= th%FONTH;
                    roffset += max(th, FONTH);    
                    draw_text(gameinfo, conw-max(5*FONTH, 2*FONTH+tw), conh-FONTH/2-roffset);
                }
            } 
            
            glPopMatrix();
        }

        if(hidestats || (!editmode && !showeditstats))
        {
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            game::gameplayhud(w, h);
            abovehud = min(abovehud, int(conh*game::abovegameplayhud()));
        }

        rendertexturepanel(w, h);
    }
    glPushMatrix();
    glScalef(conscale, conscale, 1);
    abovehud -= rendercommand(FONTH/2, abovehud - FONTH/2, conw-FONTH);
    extern int fullconsole;
    if(!hidehud || fullconsole) renderconsole(conw, conh, abovehud - FONTH/2);
    glPopMatrix();

    drawcrosshair(w, h);

    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
}


