// #include <stdio.h>
// #include <stdlib.h>
// #include <unistd.h>
#include <iostream>
#include <UT/UT_DSOVersion.h>
#include <UT/UT_Thread.h>
#include <VEX/VEX_VexOp.h>

#define TL_NO_TEXTURE_CALLBACKS
#define TL_THUNDERLOOM_IMPLEMENTATION
#include "thunderloom.h"

namespace VEX_THUNDERLOOM
{
const char *errors = 0;


static void * irwan_init()
{
    tlWeaveParameters *params = NULL;
    return (void*)params;
}

static void irwan_cleanup(void *data)
{
    tl_free_weave_parameters((tlWeaveParameters *)data);   
}


static void irwan_cloth(int argc, void *argv[], void *data)
{
    VEXvec3        *result    = (VEXvec3*)        argv[0];
    const char     *wifile    = (const char*)     argv[1];
    const VEXfloat *uscale    = (const VEXfloat*) argv[2];
    const VEXfloat *vscale    = (const VEXfloat*) argv[3];
    const VEXfloat *intensity = (const VEXfloat*) argv[4];
    const VEXint   *realuv    = (const VEXint*)   argv[5];
    const VEXvec3  *wi        = (const VEXvec3*)  argv[6];
    const VEXvec3  *wo        = (const VEXvec3*)  argv[7];
    const VEXfloat *u         = (const VEXfloat*) argv[8];
    const VEXfloat *v         = (const VEXfloat*) argv[9];

    tlWeaveParameters *params = (tlWeaveParameters *)data;
    // if (!params) {
    //     params = tl_weave_pattern_from_file(wifile, &errors);

    //     if (!params) {
    //         std::cerr << "Can't read wif file" << wifile << "\n";
    //         result->x() = 1.f;
    //         result->y() = 0.f;
    //         result->z() = 0.f;
    //     } else { 
    //         params->uscale = *uscale;
    //         params->vscale = *vscale;
    //         params->intensity_fineness  = *intensity;
    //         params->realworld_uv        = *realuv;
    //         tl_prepare(params);
    //     }
    // }
    params = tl_weave_pattern_from_file(wifile, &errors);
    if (params) {


        params->uscale = *uscale;
        params->vscale = *vscale;
        params->intensity_fineness  = *intensity;
        params->realworld_uv        = *realuv;
        tl_prepare(params);

        tlIntersectionData intersection;
        intersection.wi_x = wi->x();
        intersection.wi_y = wi->y();
        intersection.wi_z = wi->z();

        intersection.wi_x = wo->x();
        intersection.wi_y = wo->y();
        intersection.wi_z = wo->z();

        intersection.context = NULL;

        intersection.uv_y = (float)*u;
        intersection.uv_x = (float)*v;

        tlColor col = tl_shade(intersection, params);
        result->x() = (float)col.r;
        result->y() = (float)col.g;
        result->z() = (float)col.b;
    } else {
        result->x() = 1.f;
        result->y() = 0.f;
        result->z() = 0.f;
    }

}


} // end of VEX_THUNDERLOOM namespace



using namespace VEX_THUNDERLOOM;


void
newVEXOp(void *)
{
    new VEX_VexOp("irwancloth@&VSFFFIVVFF", 
            irwan_cloth, /*vector col = irwancloth(filename, uscale, vsclale, intensity, realuv, wi, wo, u, v)*/
            VEX_SURFACE_CONTEXT, 
            irwan_init,
            irwan_cleanup, 
            VEX_OPTIMIZE_2);
}
