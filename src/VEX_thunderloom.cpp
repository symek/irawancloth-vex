// #include <stdio.h>
// #include <stdlib.h>
// #include <unistd.h>
#include <iostream>
#include <map>
#include <mutex>
#include <cstring>

#include <UT/UT_DSOVersion.h>
#include <UT/UT_Thread.h>
#include <VEX/VEX_VexOp.h>

#define TL_NO_TEXTURE_CALLBACKS
#define TL_THUNDERLOOM_IMPLEMENTATION
#include "thunderloom.h"


namespace VEX_THUNDERLOOM
{


struct IrawanInstance
{
    tlWeaveParameters  *parms;
    tlIntersectionData *intersection;
    int                 index;
};

static std::mutex mtx;
static std::map<int, IrawanInstance> IrawanStore;

static void * irawan_open_init()
{
    std::lock_guard<std::mutex> guard(mtx);

    const int index = IrawanStore.size();
    IrawanInstance cloth_shader;
    cloth_shader.parms = NULL;
    cloth_shader.intersection = NULL;
    cloth_shader.index = index;   

    IrawanStore.insert(std::pair<int, IrawanInstance>(index, cloth_shader));

    std::cout << "irawan inited: " << index << "\n";
    return (void*) &cloth_shader;
}

static void irawan_open_cleanup(void *data)
{
    // tl_free_weave_parameters((tlWeaveParameters *)data);  
     std::cout << "IrawanStore.size(): " << IrawanStore.size() << "\n"; 

     // for (std::map<int, IrawanInstance>::iterator it=IrawanStore.begin(); it!=IrawanStore.end(); ++it){
        // std::cout << it->second.parms->uscale << '\n';
     // }

}


static void irawan_open(int argc, void *argv[], void *data)
{
    int            *result    = (int*)            argv[0];
    const char     *wifile    = (const char*)     argv[1];
    const VEXfloat *uscale    = (const VEXfloat*) argv[2];
    const VEXfloat *vscale    = (const VEXfloat*) argv[3];
    const VEXfloat *intensity = (const VEXfloat*) argv[4];
    const VEXint   *realuv    = (const VEXint*)   argv[5];

    IrawanInstance *shader = (IrawanInstance*)data;
    const char *errors = 0;

    if (!shader->parms) {
        shader->parms = tl_weave_pattern_from_file(wifile, &errors);
        if (!shader->parms) {
            std::cerr << "Can't read wif file: " << wifile << "\n";
            const VEXint err = -1;
            std::memcpy(result, &err, sizeof(VEXint));
        } else { 
            shader->parms->uscale = *uscale;
            shader->parms->vscale = *vscale;
            shader->parms->intensity_fineness  = *intensity;
            shader->parms->realworld_uv        = *realuv;
            tl_prepare(shader->parms);
        }
    } else {
         std::memcpy(result, &(shader->index), sizeof(int));
        }

    std::cout << shader->parms->uscale << "\n";
}


} // end of VEX_THUNDERLOOM namespace



using namespace VEX_THUNDERLOOM;


void
newVEXOp(void *)
{
    new VEX_VexOp("irawan_open@&ISFFFI", /*int handle = irawan_open(filename, uscale, vscale, intensity, realuv)*/ 
            irawan_open, 
            VEX_SURFACE_CONTEXT, 
            irawan_open_init,
            irawan_open_cleanup, 
            VEX_OPTIMIZE_2);

    // new VEX_VexOp("irawan_sample@&VVVV", /*vector color = irawan_open(wo, wi, uv)*/ 
    //         irwan_open, 
    //         VEX_SURFACE_CONTEXT, 
    //         irwan_open_init,
    //         irwan_open_cleanup, 
    //         VEX_OPTIMIZE_2);


}



    // const char *errors = 0;
    // tlWeaveParameters *params = NULL;
    // params =  tl_weave_pattern_from_file("/home/symek/work/ThunderLoom/src/wif/data/2229.wif", &errors);
    // if (!params)
    //     return NULL;
    // params->uscale = 1.f;
    // params->vscale = 1.f;
    // params->intensity_fineness  =  1.f;
    // params->realworld_uv        = 1;
    // tl_prepare(params);
   


    // const VEXvec3  *wi        = (const VEXvec3*)  argv[6];
    // const VEXvec3  *wo        = (const VEXvec3*)  argv[7];
    // const VEXfloat *u         = (const VEXfloat*) argv[8];
    // const VEXfloat *v         = (const VEXfloat*) argv[9];

    // tlWeaveParameters *params = (tlWeaveParameters *)data;
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
    // params = tl_weave_pattern_from_file(wifile, &errors);
    // if (params) {


    //     // params->uscale = *uscale;
    //     // params->vscale = *vscale;
    //     // params->intensity_fineness  = *intensity;
    //     // params->realworld_uv        = *realuv;
    //     // tl_prepare(params);

    //     tlIntersectionData intersection;
    //     intersection.wi_x = wi->x();
    //     intersection.wi_y = wi->y();
    //     intersection.wi_z = wi->z();

    //     intersection.wi_x = wo->x();
    //     intersection.wi_y = wo->y();
    //     intersection.wi_z = wo->z();

    //     intersection.context = NULL;

    //     intersection.uv_y = (float)*u;
    //     intersection.uv_x = (float)*v;

    //     tlColor col = tl_shade(intersection, params);
    //     result->x() = (float)col.r;
    //     result->y() = (float)col.g;
    //     result->z() = (float)col.b;
    // } else {
    //     result->x() = 1.f;
    //     result->y() = 0.5f;
    //     result->z() = 0.2f;
    // }
