-- Copyright (C) 2011 Jorge Jimenez (jorge@iryoku.com)
-- Copyright (C) 2011 Belen Masia (bmasia@unizar.es) 
-- Copyright (C) 2011 Jose I. Echevarria (joseignacioechevarria@gmail.com) 
-- Copyright (C) 2011 Fernando Navarro (fernandn@microsoft.com) 
-- Copyright (C) 2011 Diego Gutierrez (diegog@unizar.es)
-- All rights reserved.
-- 
-- Redistribution and use in source and binary forms, with or without
-- modification, are permitted provided that the following conditions are met:
-- 
--    1. Redistributions of source code must retain the above copyright notice,
--       this list of conditions and the following disclaimer.
-- 
--    2. Redistributions in binary form must reproduce the following disclaimer
--       in the documentation and/or other materials provided with the 
--       distribution:
-- 
--      "Uses SMAA. Copyright (C) 2011 by Jorge Jimenez, Jose I. Echevarria,
--       Belen Masia, Fernando Navarro and Diego Gutierrez."
-- 
-- THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS 
-- IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
-- THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
-- PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS OR CONTRIBUTORS 
-- BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
-- CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
-- SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
-- INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
-- CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
-- ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
-- POSSIBILITY OF SUCH DAMAGE.
-- 
-- The views and conclusions contained in the software and documentation are 
-- those of the authors and should not be interpreted as representing official
-- policies, either expressed or implied, of the copyright holders.
--
--                  _______  ___  ___       ___           ___
--                 /       ||   \/   |     /   \         /   \
--                |   (---- |  \  /  |    /  ^  \       /  ^  \
--                 \   \    |  |\/|  |   /  /_\  \     /  /_\  \
--              ----)   |   |  |  |  |  /  _____  \   /  _____  \
--             |_______/    |__|  |__| /__/     \__\ /__/     \__\
-- 
--                               E N H A N C E D
--       S U B P I X E L   M O R P H O L O G I C A L   A N T I A L I A S I N G
--
--                         http://www.iryoku.com/smaa/


smaadefs = ([[
    #define SMAA_PRESET @(smaapreset)

    #if SMAA_PRESET == 1
    #define SMAA_THRESHOLD 0.1
    #define SMAA_MAX_SEARCH_STEPS 8
    #define SMAA_MAX_SEARCH_STEPS_DIAG 0
    #define SMAA_CORNER_ROUNDING 100
    #elif SMAA_PRESET == 2
    #define SMAA_THRESHOLD 0.1
    #define SMAA_MAX_SEARCH_STEPS 16
    #define SMAA_MAX_SEARCH_STEPS_DIAG 8
    #define SMAA_CORNER_ROUNDING 25
    #elif SMAA_PRESET == 3
    #define SMAA_THRESHOLD 0.05
    #define SMAA_MAX_SEARCH_STEPS 32
    #define SMAA_MAX_SEARCH_STEPS_DIAG 16
    #define SMAA_CORNER_ROUNDING 25
    #else
    #define SMAA_THRESHOLD 0.15
    #define SMAA_MAX_SEARCH_STEPS 4
    #define SMAA_MAX_SEARCH_STEPS_DIAG 0
    #define SMAA_CORNER_ROUNDING 100
    #endif

    #define SMAA_AREATEX_MAX_DISTANCE 16
    #define SMAA_AREATEX_MAX_DISTANCE_DIAG 20
    #define SMAA_AREATEX_WIDTH 160
    #define SMAA_AREATEX_HEIGHT 560
    #define SMAA_SEARCHTEX_WIDTH 66
    #define SMAA_SEARCHTEX_HEIGHT 33
]]):eval_embedded()

CAPI.shader(0, "SMAALumaEdgeDetection" .. smaapreset, [[
    void main(void)
    {
        gl_Position = gl_Vertex;
        gl_TexCoord[0].xy = gl_MultiTexCoord0.xy;
        gl_TexCoord[1] = gl_MultiTexCoord0.xyxy + vec4(-1.0, 0.0, 0.0, -1.0);
        gl_TexCoord[2] = gl_MultiTexCoord0.xyxy + vec4( 1.0, 0.0, 0.0,  1.0);
        gl_TexCoord[3] = gl_MultiTexCoord0.xyxy + vec4(-2.0, 0.0, 0.0, -2.0);
    }
]], ([[
    #extension GL_ARB_texture_rectangle : enable
    @(smaadefs)
    uniform sampler2DRect tex0;
    void main(void)
    {
        // Calculate lumas:
        vec3 weights = vec3(0.2126, 0.7152, 0.0722);
        float L = dot(texture2DRect(tex0, gl_TexCoord[0].xy).rgb, weights);
        float Lleft = dot(texture2DRect(tex0, gl_TexCoord[1].xy).rgb, weights);
        float Ltop  = dot(texture2DRect(tex0, gl_TexCoord[1].zw).rgb, weights);

        // We do the usual threshold:
        vec4 delta;
        delta.xy = abs(L - vec2(Lleft, Ltop));
        vec2 edges = step(SMAA_THRESHOLD, delta.xy);

        // Then discard if there is no edge:
        if(dot(edges, vec2(1.0)) <= 0.0)
            discard;

        // Calculate right and bottom deltas:
        float Lright = dot(texture2DRect(tex0, gl_TexCoord[2].xy).rgb, weights);
        float Lbottom  = dot(texture2DRect(tex0, gl_TexCoord[2].zw).rgb, weights);
        delta.zw = abs(L - vec2(Lright, Lbottom));

        // Calculate the maximum delta in the direct neighborhood:
        vec2 maxDelta = max(delta.xy, delta.zw);
        maxDelta = max(maxDelta.xx, maxDelta.yy);

        // Calculate left-left and top-top deltas:
        float Lleftleft = dot(texture2DRect(tex0, gl_TexCoord[3].xy).rgb, weights);
        float Ltoptop = dot(texture2DRect(tex0, gl_TexCoord[3].zw).rgb, weights);
        delta.zw = abs(vec2(Lleft, Ltop) - vec2(Lleftleft, Ltoptop));

        // Calculate the final maximum delta:
        maxDelta = max(maxDelta.xy, delta.zw);

        /**
         * Each edge with a delta in luma of less than 50% of the maximum luma
         * surrounding this pixel is discarded. This allows to eliminate spurious
         * crossing edges, and is based on the fact that, if there is too much
         * contrast in a direction, that will hide contrast in the other
         * neighbors.
         * This is done after the discard intentionally as this situation doesn't
         * happen too frequently (but it's important to do as it prevents some 
         * edges from going undetected).
         */
        edges.xy *= step(0.5 * maxDelta, delta.xy);

        gl_FragColor = vec4(edges, 0.0, 0.0);
    }
]]):eval_embedded())

CAPI.shader(0, "SMAAColorEdgeDetection" .. smaapreset, [[
    void main(void)
    {
        gl_Position = gl_Vertex;
        gl_TexCoord[0].xy = gl_MultiTexCoord0.xy;
        gl_TexCoord[1] = gl_MultiTexCoord0.xyxy + vec4(-1.0, 0.0, 0.0, -1.0);
        gl_TexCoord[2] = gl_MultiTexCoord0.xyxy + vec4( 1.0, 0.0, 0.0,  1.0);
        gl_TexCoord[3] = gl_MultiTexCoord0.xyxy + vec4(-2.0, 0.0, 0.0, -2.0);
    }
]], ([[
    #extension GL_ARB_texture_rectangle : enable
    @(smaadefs)
    uniform sampler2DRect tex0;
    void main(void)
    {
        // Calculate color deltas:
        vec2 delta;
        vec3 C = texture2DRect(tex0, gl_TexCoord[0].xy).rgb;

        vec3 Cleft = abs(C - texture2DRect(tex0, gl_TexCoord[1].xy).rgb);
        delta.x = max(max(Cleft.r, Cleft.g), Cleft.b);
        vec3 Ctop = abs(C - texture2DRect(tex0, gl_TexCoord[1].zw).rgb);
        delta.y = max(max(Ctop.r, Ctop.g), Ctop.b);

        // We do the usual threshold:
        vec2 edges = step(SMAA_THRESHOLD, delta);

        // Then discard if there is no edge:
        if (dot(edges, vec2(1.0)) == 0.0)
            discard;

        // Calculate right and bottom deltas:
        vec3 Cright = abs(C - texture2DRect(tex0, gl_TexCoord[2].xy).rgb);
        vec3 Cbottom = abs(C - texture2DRect(tex0, gl_TexCoord[2].zw).rgb);
        // Calculate left-left and top-top deltas:
        vec3 Cleftleft = abs(C - texture2DRect(tex0, gl_TexCoord[3].xy).rgb);
        vec3 Ctoptop = abs(C - texture2DRect(tex0, gl_TexCoord[3].zw).rgb);
        // Calculate the maximum delta in the direct neighborhood:
        vec3 t = max(max(Cright, Cbottom), max(Cleftleft, Ctoptop));
        // Calculate the final maximum delta:
        float maxDelta = max(max(delta.x, delta.y), max(max(t.r, t.g), t.b));

        // Local contrast adaptation in action:
        edges.xy *= step(0.5 * maxDelta, delta.xy);

        gl_FragColor = vec4(edges, 0.0, 0.0);
    }
]]):eval_embedded())

CAPI.shader(0, "SMAABlendingWeightCalculation" .. smaapreset, ([[
    @(smaadefs)
    void main(void)
    {
        gl_Position = gl_Vertex;
        gl_TexCoord[0].xy = gl_MultiTexCoord0.xy;

        // We will use these offsets for the searches later on (see PSEUDO_GATHER4):
        gl_TexCoord[1] = gl_MultiTexCoord0.xyxy + vec4(-0.25, -0.125,  1.25, -0.125);
        gl_TexCoord[2] = gl_MultiTexCoord0.xyxy + vec4(-0.125, -0.25, -0.125,  1.25);
    }
]]):eval_embedded(), ([[
    #extension GL_ARB_texture_rectangle : enable
    @(smaadefs)
    uniform sampler2DRect tex0, tex1, tex2;

    //-----------------------------------------------------------------------------
    // Diagonal Search Functions

    #if SMAA_MAX_SEARCH_STEPS_DIAG > 0

    /**
     * These functions allows to perform diagonal pattern searches.
     */
    float SMAASearchDiag1(vec2 texcoord, vec2 dir, float c) {
        texcoord += dir;
        vec2 e = vec2(0.0);
        float i;
        for (i = 0.0; i < float(SMAA_MAX_SEARCH_STEPS_DIAG); i++) {
            e.rg = texture2DRect(tex0, texcoord).rg;
            if (dot(e, vec2(1.0)) < 1.9) break;
            texcoord += dir;
        }
        return i + float(e.g > 0.9) * c;
    }

    float SMAASearchDiag2(vec2 texcoord, vec2 dir, float c) {
        texcoord += dir;
        vec2 e = vec2(0.0);
        float i;
        for (i = 0.0; i < float(SMAA_MAX_SEARCH_STEPS_DIAG); i++) {
            e.g = texture2DRect(tex0, texcoord).g;
            e.r = texture2DRect(tex0, texcoord + vec2(1.0, 0.0)).r;
            if (dot(e, vec2(1.0)) < 1.9) break;
            texcoord += dir;
        }
        return i + float(e.g > 0.9) * c;
    }

    /** 
     * Similar to SMAAArea, this calculates the area corresponding to a certain
     * diagonal distance and crossing edges 'e'.
     */
    vec2 SMAAAreaDiag(vec2 dist, vec2 e) {
        vec2 texcoord = float(SMAA_AREATEX_MAX_DISTANCE_DIAG) * e + dist;

        // We do a scale and bias for mapping to texel space:
        texcoord = texcoord + 0.5;

        // Diagonal areas are on the second half of the texture:
        texcoord.x += 0.5*float(SMAA_AREATEX_WIDTH);

        return texture2DRect(tex1, texcoord).ra;
    }

    /**
     * This searches for diagonal patterns and returns the corresponding weights.
     */
    vec2 SMAACalculateDiagWeights(vec2 texcoord, vec2 e) {
        vec2 weights = vec2(0.0);

        vec2 d;
        d.x = e.r > 0.0? SMAASearchDiag1(texcoord, vec2(-1.0,  1.0), 1.0) : 0.0;
        d.y = SMAASearchDiag1(texcoord, vec2(1.0, -1.0), 0.0);

        if (d.r + d.g > 2.0) { // d.r + d.g + 1 > 3
            vec4 coords = vec4(-d.r, d.r, d.g, -d.g) + texcoord.xyxy;

            vec4 c;
            c.x = texture2DRect(tex0, coords.xy + vec2(-1.0,  0.0)).g;
            c.y = texture2DRect(tex0, coords.xy + vec2( 0.0,  0.0)).r;
            c.z = texture2DRect(tex0, coords.zw + vec2( 1.0,  0.0)).g;
            c.w = texture2DRect(tex0, coords.zw + vec2( 1.0, -1.0)).r;
            vec2 e = 2.0 * c.xz + c.yw;
            e *= step(d.rg, vec2(float(SMAA_MAX_SEARCH_STEPS_DIAG) - 1.0));

            weights += SMAAAreaDiag(d, e);
        }

        d.x = SMAASearchDiag2(texcoord, vec2(-1.0, -1.0), 0.0);
        float right = texture2DRect(tex0, texcoord + vec2(1.0, 0.0)).r;
        d.y = right > 0.0? SMAASearchDiag2(texcoord, vec2(1.0, 1.0), 1.0) : 0.0;

        if (d.r + d.g > 2.0) { // d.r + d.g + 1 > 3
            vec4 coords = vec4(-d.r, -d.r, d.g, d.g) + texcoord.xyxy;

            vec4 c;
            c.x  = texture2DRect(tex0, coords.xy + vec2(-1.0,  0.0)).g;
            c.y  = texture2DRect(tex0, coords.xy + vec2( 0.0, -1.0)).r;
            c.zw = texture2DRect(tex0, coords.zw + vec2( 1.0,  0.0)).gr;
            vec2 e = 2.0 * c.xz + c.yw;
            e *= step(d.rg, vec2(float(SMAA_MAX_SEARCH_STEPS_DIAG) - 1.0));

            weights += SMAAAreaDiag(d, e).gr;
        }

        return weights;
    }
    #endif

    //-----------------------------------------------------------------------------
    // Horizontal/Vertical Search Functions

    /**
     * This allows to determine how much length should we add in the last step
     * of the searches. It takes the bilinearly interpolated edge (see 
     * PSEUDO_GATHER4), and adds 0, 1 or 2, depending on which edges and
     * crossing edges are active.
     */
    float SMAASearchLength(vec2 e, float bias, float scale) {
        e.r = bias + e.r * scale;
        return 255.0 * texture2DRect(tex2, e*vec2(float(SMAA_SEARCHTEX_WIDTH), float(SMAA_SEARCHTEX_HEIGHT))).r;
    }

    /**
    * Horizontal/vertical search functions for the 2nd pass.
    */
    float SMAASearchXLeft(vec2 texcoord) {
        /**
         * PSEUDO_GATHER4
         * This texcoord has been offset by (-0.25, -0.125) in the vertex shader to
         * sample between edge, thus fetching four edges in a row.
         * Sampling with different offsets in each direction allows to disambiguate
         * which edges are active from the four fetched ones.
         */
        vec2 e = texture2DRect(tex0, texcoord).rg;
        float i;
        for(i = -2.0; i > -2.0*float(SMAA_MAX_SEARCH_STEPS); i -= 2.0) {
            if(e.g <= 0.8281 || e.r > 0.0) break; // Is there some edge not activated or a crossing edge that breaks the line?
            e = texture2DRect(tex0, texcoord + vec2(i, 0.0)).rg;
        }
        // We correct the previous (-0.25, -0.125) offset we applied:
        // The searches are bias by 1, so adjust the coords accordingly:
        // Disambiguate the length added by the last step:
        // Undo last step
        return texcoord.x + i + (0.25 + 1.0 + 2.0) - SMAASearchLength(e, 0.0, 0.5);
    }

    float SMAASearchXRight(vec2 texcoord) {
        vec2 e = texture2DRect(tex0, texcoord).rg;
        float i;
        for(i = 2.0; i < 2.0*float(SMAA_MAX_SEARCH_STEPS); i += 2.0) {
            if(e.g <= 0.8281 || e.r > 0.0) break; // Is there some edge not activated or a crossing edge that breaks the line?
            e = texture2DRect(tex0, texcoord + vec2(i, 0.0)).rg;
        }
        return texcoord.x + i - (0.25 + 1.0 + 2.0) + SMAASearchLength(e, 0.5, 0.5);
    }

    float SMAASearchYUp(vec2 texcoord) {
        vec2 e = texture2DRect(tex0, texcoord).rg;
        float i;
        for(i = -2.0; i > -2.0*float(SMAA_MAX_SEARCH_STEPS); i -= 2.0) {
            if(e.r <= 0.8281 || e.g > 0.0) break; // Is there some edge not activated or a crossing edge that breaks the line?
            e = texture2DRect(tex0, texcoord + vec2(0.0, i)).rg;
        }
        return texcoord.y + i + (0.25 + 1.0 + 2.0) - SMAASearchLength(e.gr, 0.0, 0.5);
    }

    float SMAASearchYDown(vec2 texcoord) {
        vec2 e = texture2DRect(tex0, texcoord).rg;
        float i;
        for(i = 2.0; i < 2.0*float(SMAA_MAX_SEARCH_STEPS); i += 2.0) {
            if(e.r <= 0.8281 || e.g > 0.0) break; // Is there some edge not activated or a crossing edge that breaks the line?
            e = texture2DRect(tex0, texcoord + vec2(0.0, i)).rg;
        }
        return texcoord.y + i - (0.25 + 1.0 + 2.0) + SMAASearchLength(e.gr, 0.5, 0.5);
    }

    /** 
     * Ok, we have the distance and both crossing edges. So, what are the areas
     * at each side of current edge?
     */
    vec2 SMAAArea(vec2 dist, float e1, float e2) {
        // Rounding prevents precision errors of bilinear filtering:
        vec2 texcoord = float(SMAA_AREATEX_MAX_DISTANCE) * floor(4.0 * vec2(e1, e2) + 0.5) + dist;

        // We do a scale and bias for mapping to texel space:
        texcoord = texcoord + 0.5;

        return texture2DRect(tex1, texcoord).ra;
    }
   
    //-----------------------------------------------------------------------------
    // Corner Detection Functions

    #if SMAA_CORNER_ROUNDING < 100

    vec2 SMAADetectHorizontalCornerPattern(vec2 texcoord, vec2 d) {
        texcoord.x += abs(d.x) < abs(d.y) ? d.x : d.y + 1.0;
        vec2 e;
        e.r = texture2DRect(tex0, texcoord + vec2(0.0,  1.0)).r;
        e.g = texture2DRect(tex0, texcoord + vec2(0.0, -2.0)).r;
        return clamp(float(SMAA_CORNER_ROUNDING) / 100.0 + 1.0 - e, 0.0, 1.0);
    }

    vec2 SMAADetectVerticalCornerPattern(vec2 texcoord, vec2 d) {
        texcoord.y += abs(d.x) < abs(d.y) ? d.x : d.y + 1.0;
        vec2 e;
        e.r = texture2DRect(tex0, texcoord + vec2( 1.0, 0.0)).g;
        e.g = texture2DRect(tex0, texcoord + vec2(-2.0, 0.0)).g;
        return clamp(float(SMAA_CORNER_ROUNDING) / 100.0 + 1.0 - e, 0.0, 1.0);
    }

    #endif

    //-----------------------------------------------------------------------------
    // Blending Weight Calculation Pixel Shader (Second Pass)

    void main(void) 
    {
        vec4 weights = vec4(0.0, 0.0, 0.0, 0.0);

        vec2 e = texture2DRect(tex0, gl_TexCoord[0].xy).rg;

        if (e.g > 0.0) { // Edge at north
            #if SMAA_MAX_SEARCH_STEPS_DIAG > 0
            // Diagonals have both north and west edges, so searching for them in
            // one of the boundaries is enough.
            weights.rg = SMAACalculateDiagWeights(gl_TexCoord[0].xy, e);

            // We give priority to diagonals, so if we find a diagonal we skip 
            // horizontal/vertical processing.
            if (dot(weights.rg, vec2(1.0)) == 0.0) {
            #endif

            vec2 d;

            // Find the distance to the left:
            vec2 coords;
            coords.x = SMAASearchXLeft(gl_TexCoord[1].xy);
            coords.y = gl_TexCoord[2].y; // gl_TexCoord[2].y = gl_TexCoord[0].y - 0.25 (CROSSING_OFFSET)
            d.x = coords.x;

            // Now fetch the left crossing edges, two at a time using bilinear
            // filtering. Sampling at -0.25 (see CROSSING_OFFSET) enables to
            // discern what value each edge has:
            float e1 = texture2DRect(tex0, coords).r;

            // Find the distance to the right:
            coords.x = SMAASearchXRight(gl_TexCoord[1].zw);
            d.y = coords.x;

            // We want the distances to be in pixel units (doing this here allow to
            // better interleave arithmetic and memory accesses):
            d = d - gl_TexCoord[0].x;

            // SMAAArea below needs a sqrt, as the areas texture is compressed 
            // quadratically:
            vec2 sqrt_d = sqrt(abs(d));

            // Fetch the right crossing edges:
            float e2 = texture2DRect(tex0, coords + vec2(1.0, 0.0)).r;

            // Ok, we know how this pattern looks like, now it is time for getting
            // the actual area:
            weights.rg = SMAAArea(sqrt_d, e1, e2);

            #if SMAA_CORNER_ROUNDING < 100
            // Fix corners:
            weights.rg *= SMAADetectHorizontalCornerPattern(gl_TexCoord[0].xy, d);
            #endif

            #if SMAA_MAX_SEARCH_STEPS_DIAG > 0
            } else
                e.r = 0.0; // Skip vertical processing.
            #endif
        }

        if (e.r > 0.0) { // Edge at west
            vec2 d;

            // Find the distance to the top:
            vec2 coords;
            coords.y = SMAASearchYUp(gl_TexCoord[2].xy);
            coords.x = gl_TexCoord[1].x; // gl_TexCoord[2].x = gl_TexCoord[0].x - 0.25;
            d.x = coords.y;

            // Fetch the top crossing edges:
            float e1 = texture2DRect(tex0, coords).g;

            // Find the distance to the bottom:
            coords.y = SMAASearchYDown(gl_TexCoord[2].zw);
            d.y = coords.y;

            // We want the distances to be in pixel units:
            d = d - gl_TexCoord[0].y;

            // SMAAArea below needs a sqrt, as the areas texture is compressed 
            // quadratically:
            vec2 sqrt_d = sqrt(abs(d));

            // Fetch the bottom crossing edges:
            float e2 = texture2DRect(tex0, coords + vec2(0.0, 1.0)).g;

            // Get the area for this direction:
            weights.ba = SMAAArea(sqrt_d, e1, e2);

            #if SMAA_CORNER_ROUNDING < 100
            // Fix corners:
            weights.ba *= SMAADetectVerticalCornerPattern(gl_TexCoord[0].xy, d);
            #endif
        }

        gl_FragColor = weights;
    }
]]):eval_embedded())

CAPI.shader(0, "SMAANeighborhoodBlending" .. smaapreset, [[
    void main(void)
    {
        gl_Position = gl_Vertex;
        gl_TexCoord[0].xy = gl_MultiTexCoord0.xy;

        gl_TexCoord[1] = gl_MultiTexCoord0.xyxy + vec4(-1.0, 0.0, 0.0, -1.0);
        gl_TexCoord[2] = gl_MultiTexCoord0.xyxy + vec4( 1.0, 0.0, 0.0,  1.0);
    }
]], ([[
    #extension GL_ARB_texture_rectangle : enable
    @(smaadefs)
    uniform sampler2DRect tex0, tex1;
    // Neighborhood Blending Pixel Shader (Third Pass)
    void main(void)
    {
        // Fetch the blending weights for current pixel:
        vec4 a;
        a.xz = texture2DRect(tex1, gl_TexCoord[0].xy).xz;
        a.y = texture2DRect(tex1, gl_TexCoord[2].zw).g;
        a.w = texture2DRect(tex1, gl_TexCoord[2].xy).a;

        // Is there any blending weight with a value greater than 0.0?
        if (dot(a, vec4(1.0)) < 1e-5)
            gl_FragColor.rgb = texture2DRect(tex0, gl_TexCoord[0].xy).rgb;
        else {
            // Up to 4 lines can be crossing a pixel (one through each edge). We
            // favor blending by choosing the line with the maximum weight for each
            // direction:
            vec2 offset;
            offset.x = a.a > a.b? a.a : -a.b; // left vs. right 
            offset.y = a.g > a.r? a.g : -a.r; // top vs. bottom

            // Then we go in the direction that has the maximum weight:
            if (abs(offset.x) > abs(offset.y)) // horizontal vs. vertical
                offset.y = 0.0;
            else
                offset.x = 0.0;

            // We exploit bilinear filtering to mix current pixel with the chosen
            // neighbor:
            gl_FragColor.rgb = texture2DRect(tex0, gl_TexCoord[0].xy + offset).rgb;
        }
    }
]]):eval_embedded())
