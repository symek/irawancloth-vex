#include <iostream>
#include <map>
#include <mutex>
#include <cstring>
#include <functional>
#include <memory>

#include <UT/UT_DSOVersion.h>
#include <UT/UT_Thread.h>
#include <VEX/VEX_VexOp.h>

#define TL_NO_TEXTURE_CALLBACKS
#define TL_THUNDERLOOM_IMPLEMENTATION
#include "thunderloom.h"


namespace VEX_THUNDERLOOM
{

// template<typename T, typename... Args>
// std::unique_ptr<T> make_unique(Args&&... args)
// {
//     return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
// }

struct IrawanInstance
{
    tlWeaveParameters  *parms;
   // std::unique_ptr<tlIntersectionData> \

    tlIntersectionData *intersection;
    int handle;
};

static std::mutex irawan_mutex;
static std::map<int, IrawanInstance> IrawanStore;

static void * irawan_open_init()
{
   
}

static void irawan_open_cleanup(void *data)
{
  

}


const int irawan_hash(const char* filename, const VEXfloat uscale, 
                             const VEXfloat vscale, const VEXfloat intensity, 
                             const VEXint realuv)
{
    std::string hashbase(filename);
    hashbase += std::to_string(uscale);
    hashbase += std::to_string(vscale);
    hashbase += std::to_string(intensity);
    hashbase += std::to_string(realuv);
    std::hash<std::string> hasher;
    int hash = hasher(hashbase);
    return hash;
}

const int create_shader(const char* filename, const VEXfloat uscale, 
                        const VEXfloat vscale, const VEXfloat intensity, 
                        const VEXint realuv)
{
    std::lock_guard<std::mutex> guard(irawan_mutex);

    const int hash = irawan_hash(filename, uscale, vscale, intensity, realuv);

    std::map<int, IrawanInstance>::const_iterator it;

    it = IrawanStore.find(hash);
    if (it == IrawanStore.end()) {
        const char *errors = 0;
        IrawanInstance shader;
        shader.handle = hash;
        shader.parms  = NULL;   
        shader.parms  = tl_weave_pattern_from_file(filename, &errors);
        if (shader.parms == NULL) {
            std::cerr << errors << '\n';
            return 0;
        }

        shader.parms->uscale = uscale; 
        shader.parms->vscale = vscale;
        shader.parms->realworld_uv = realuv;
        shader.parms->intensity_fineness = intensity;

        tl_prepare(shader.parms);
        //tlIntersectionData = intersection;
        // shader.intersection = &tlIntersectionData(); //make_unique<tlIntersectionData>();
        IrawanStore.insert(std::pair<int, IrawanInstance>(hash, shader));

        return hash;
    } else {
        return it->first;
    }
}


static void fill_intersect(tlIntersectionData &data, const VEXvec3 *wi, 
    const VEXvec3 *wo, const VEXvec3 *uvw)
{
    data.wi_x = wi->x();
    data.wi_y = wi->y();
    data.wi_z = wi->z();
    data.wi_x = wo->x();
    data.wi_y = wo->y();
    data.wi_z = wo->z();
    data.uv_x = uvw->x();
    data.uv_y = uvw->y();
    data.context = NULL;
}

static void irawan_open(int argc, void *argv[], void *data)
{
    int            *result    = (int*)            argv[0];
    const char     *wifile    = (const char*)     argv[1];
    const VEXfloat *uscale    = (const VEXfloat*) argv[2];
    const VEXfloat *vscale    = (const VEXfloat*) argv[3];
    const VEXfloat *intensity = (const VEXfloat*) argv[4];
    const VEXint   *realuv    = (const VEXint*)   argv[5];

    const int handle = create_shader(wifile, *uscale, *vscale, *intensity, *realuv);
    result[0] = handle; 

}

static void irawan_sample(int argc, void *argv[], void *data)
{
          VEXvec3 *result =       (VEXvec3*) argv[0];
    const VEXint  *handle = (const VEXint* ) argv[1];
    const VEXvec3 *wi     = (const VEXvec3*) argv[2];
    const VEXvec3 *wo     = (const VEXvec3*) argv[3];
    const VEXvec3 *uvw    = (const VEXvec3*) argv[4];
    

    std::map<int, IrawanInstance>::iterator it;
    it = IrawanStore.find(*handle);
    UT_ASSERT(it != IrawanStore.end());
    IrawanInstance *shader = static_cast<IrawanInstance*>(&it->second);

    tlIntersectionData intersection;
    fill_intersect(intersection, wi, wo, uvw);

    tlColor col = tl_shade(intersection, shader->parms);
    result->x() = (float)col.r;
    result->y() = (float)col.g;
    result->z() = (float)col.b;

}

static void irawan_sample(int argc, void *argv[], void *data)
{

    const VEXint   *handle = (const VEXint* ) argv[0];
    const VEXvec3  *wi     = (const VEXvec3*) argv[1];
    const VEXvec3  *wo     = (const VEXvec3*) argv[2];
    const VEXvec3  *uvw    = (const VEXvec3*) argv[3];
          VEXvec3  *diff   = (const VEXvec3*) argv[4];
          VEXfloat *spec   = (const VEXfloat*)argv[5];
          VEXvec3  *N      = (const VEXvec3*) argv[6];
          VEXfloat *hit    = (const VEXfloat*)argv[7];

    std::map<int, IrawanInstance>::iterator it;
    it = IrawanStore.find(*handle);
    UT_ASSERT(it != IrawanStore.end());
    IrawanInstance *shader = static_cast<IrawanInstance*>(&it->second);

    tlIntersectionData intersection;
    fill_intersect(intersection, wi, wo, uvw);

    tlPatternData pattern = tl_get_pattern_data(intersection, shader->parms);

    tlColor col = tl_eval_diffuse(intersection, pattern, shader->parms);
    diff->x() = col.r; diff->y() = col.g; diff->z() = col.b;
    spec[0]   = tl_eval_specular(intersection, pattern, shader->parms);
       

    }

}
   

}


} // end of VEX_THUNDERLOOM namespace



using namespace VEX_THUNDERLOOM;


void
newVEXOp(void *)
{
    new VEX_VexOp("irawan_open@&ISFFFI", /*int handle = irawan_open(filename, uscale, vscale, intensity, realuv)*/ 
            irawan_open, 
            VEX_SURFACE_CONTEXT, 
            NULL, /*irawan_open_init,*/
            NULL, /*irawan_open_cleanup, */
            VEX_OPTIMIZE_2);

    new VEX_VexOp("irawan_sample@&VIVVV", /*vector color = irawan_sample(handle, wo, wi, uv)*/ 
            irawan_sample, 
            VEX_SURFACE_CONTEXT, 
            NULL, /*irwan_open_init,*/
            NULL, /*irwan_open_cleanup,*/ 
            VEX_OPTIMIZE_2);

      new VEX_VexOp("irawan_sample@IVVV&V&F&V&F", /*void irawan_sample(handle, wo, wi, uv, &diff, &spec, &N, &hit)*/ 
            irawan_sample, 
            VEX_SURFACE_CONTEXT, 
            NULL, /*irwan_open_init,*/
            NULL, /*irwan_open_cleanup,*/ 
            VEX_OPTIMIZE_2,
            false);


}
