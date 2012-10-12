#define WEFT_KD {0.042223, 0.0044707, 0.0034772}
#define WEFT_KS {0.25977, 0.088627, 0.080477}
#define WARP_KD {0.0096037, 0.003146, 0.0029804}
#define WARP_KS {0.059085, 0.063159, 0.068253}


#define WEAVE    Weave weave = { "Silk shantung", \
                 /* Tile size of the weave pattern */  \
                 6, 8, \
                 /* Uniform and forward scattering parameters */ \
                 0.02, 1.5, \
                 /* Filament smoothing */ \
                 0.5, \
                 /* Highlight width */ \
                 0.5, \
                 /* Combined warp/weft size */ \
                 8, 16, \
                 /* Noise-related parameters */ \
                 20, 20, 10, 10, 500, 0, \
                 /* Weave pattern description */ \
                 { 3, 3, 3, 3, 2, 2, \
	             3, 3, 3, 3, 2, 2, \
	             3, 3, 3, 3, 2, 2, \
	             3, 3, 3, 3, 2, 2, \
	             4, 1, 1, 5, 5, 5, \
	             4, 1, 1, 5, 5, 5, \
	             4, 1, 1, 5, 5, 5, \
	             4, 1, 1, 5, 5, 5 }}; 


 
#define YARNS  Yarns yarns =  {                    \
        {WARP, WARP, WEFT, WEFT, WEFT},            \
        /* Fiber twist angle */                    \
        {0, 0, 0, 0, 0},                          \
        /* Maximum inclination angle */            \
        {50, 50, 23, 23, 23},                      \
        /* Spine curvature */                      \
        {-0.5, -0.5, -0.3, -0.3, -0.3},            \
        /* Width and length of the segment rectangle */ \
        {2, 2, 4, 4, 4},                           \
        {4, 4, 4, 4, 4},                           \
        /* Yarn segment center in tile space */    \
        {0.3333, 0.8333, 0.3333, -0.1667, 0.8333}, \
        {0.25,   0.75,   0.75,    0.25,   0.25},   \
    	/* Diffuse and specular color */           \
    	{WARP_KD, WARP_KD, WEFT_KD, WEFT_KD, WEFT_KD}, \
        {WARP_KS, WARP_KS, WEFT_KS, WEFT_KS, WEFT_KS}};\


