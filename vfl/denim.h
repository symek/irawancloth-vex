#define WEFT_KD {0.13774, 0.13133, 0.16336}
#define WEFT_KS {0.35359, 0.35359, 0.35359}
#define WARP_KD {0.0076877, 0.0096096, 0.014735}
#define WARP_KS {0.020799, 0.026346, 0.031893}


#define DENIM    Weave weave = { "Cotton denim", \
                 /* Tile size of the weave pattern */  \
                 3, 6, \
                 /* Uniform and forward scattering parameters */ \
                 0.01, 4.0, \
                 /* Filament smoothing */ \
                 0.0, \
                 /* Highlight width */ \
                 0.5, \
                 /* Combined warp/weft size */ \
                 5, 1, \
                 /* Noise-related parameters */ \
                 90, 90, 90, 90, 3, 0, \
                 /* Weave pattern description */ \
                 { 1, 3, 8, \
	             1, 3, 5 , \
	             1, 7, 5, \
	             1, 4, 5, \
	             6, 4, 5, \
	             2, 4, 5, }}; \
                Yarns yarns =  {                    \
                {WARP, WARP, WARP, WARP, WARP, WEFT, WEFT, WEFT},            \
                /* Fiber twist angle */                    \
                {-30, -30, -30, -30, -30, -30, -30, -30},  \
                /* Maximum inclination angle */            \
                {12, 12, 12, 12, 12, 38, 38, 38},          \
                /* Spine curvature */                      \
                {0, 0, 0, 0, 0, 0, 0, 0},                  \
                /* Width and length of the segment rectangle */ \
                {1, 1, 1, 1, 1, 1, 1, 1},                  \
                {5, 5, 5, 5, 5, 1, 1, 1},                  \
                /* Yarn segment center in tile space */    \
                {0.1667, 0.1667, 0.5, 0.5, 0.8333, 0.1667, 0.5, 0.8333}, \
                {0.75,  -0.25,  1.083,   0.0833,  0.4167, 0.25,   0.5833, 0.9167},   \
            	/* Diffuse and specular color */           \
            	{WARP_KD, WARP_KD, WARP_KD, WARP_KD, WARP_KD, WEFT_KD, WEFT_KD, WEFT_KD}, \
                {WARP_KS, WARP_KS, WARP_KS, WARP_KS, WARP_KS, WEFT_KS, WEFT_KS, WEFT_KS}};