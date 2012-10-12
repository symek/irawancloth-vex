#define WARP 0
#define WEFT 1
#define INV_PI 0.31830988618379067154f

/*
////////////////////////////////////////////////////
            Weave and yarns (SOA)
///////////////////////////////////////////////////
*/

struct Weave 
{
    string name;

    /* Tile size of the weave pattern */
    int tileWidth;
    int tileHeight;

    /* Uniform and forward scattering parameters */
    float alpha;
    float beta;

    /* Filament smoothing */
    float ss;

    /* Highlight width */
    float hWidth;

    /* Combined warp/weft size */
    float warpArea;
    float weftArea;

    /* Noise-related parameters */
    float dWarpUmaxOverDWarp;
    float dWarpUmaxOverDWeft;
    float dWeftUmaxOverDWarp;
    float dWeftUmaxOverDWeft;
    float fineness;
    float period;

    /* Weave pattern description */
    int pattern[];
}

struct Yarns 
{
    int type[];
    /* Fiber twist angle */
    float psi[];
    /* Maximum inclination angle */
    float umax[];
    /* Spine curvature */
    float kappa[];
    /* Width and length of the segment rectangle */
    float width[];
    float length[];
    /* Yarn segment center in tile space */
    float centerU[];
    float centerV[];
	/* Diffuse and specular color */
	vector kd[];
	vector ks[];
}

/*
///////////////////////////////////////////////////////////////////////////
 Returns yarn ID given parameteric uv (might be st) and pattern
 /////////////////////////////////////////////////////////////////////////
 */

void getYarnID(int yarnID; vector _uv; Weave pattern; float repeatU; float repeatV)
{
    vector myuv;
    myuv.x = _uv.x * repeatU;
    myuv.y = (1-_uv.y) * repeatV;

    vector xy;
    xy.x = myuv.x * pattern.tileWidth;
    xy.y = myuv.y * pattern.tileHeight;

    vector lookup;
    lookup.x = xy.x % pattern.tileWidth;
    lookup.y = xy.y % pattern.tileHeight;
    
    yarnID = pattern.pattern[(int)lookup.x + (int)lookup.y * pattern.tileWidth] -1;
}

vector getDiffuseReflectance(vector _uv; Weave pattern; yarns _yarns; 
                            float repeatU; float repeatV)
{
    vector myuv;
    myuv.x = _uv.x       * repeatU;
    myuv.y = (1 - _uv.y) * repeatV;

    vector xy;
    xy.x = myuv.x * pattern.tileWidth;
    xy.y = myuv.y * pattern.tileHeight;

    vector lookup;
    lookup.x = xy.x % pattern.tileWidth;
    lookup.y = xy.y % pattern.tileHeight;
    
    int yarnID = pattern.pattern[(int)lookup.x + (int)lookup.y * pattern.tileWidth] -1;
    return _yarns.kd[yarnID];

}

/*
///////////////////////////////////////////////////////////
    Technical staff used by BRDF
///////////////////////////////////////////////////////////
*/

float atanh(float arg) 
{
	return log((1.0f + arg) / (1.0f - arg)) / 2.0f;
}


// von Mises Distribution
float vonMises(float cos_x; float b)  
{
	// assumes a = 0, b > 0 is a concentration parameter.
	float I0;
    float absB = abs(b);
	if (absB <= 3.75f) 
    {
		float t = absB / 3.75f;
		t = t * t;
		I0 = 1.0f + t*(3.5156229f + t*(3.0899424f + t*(1.2067492f
				+ t*(0.2659732f + t*(0.0360768f + t*0.0045813f)))));
	} 
    else 
    {
		float t = 3.75f / absB;
		I0 = exp(absB) / sqrt(absB) * (0.39894228f + t*(0.01328592f
			+ t*(0.00225319f + t*(-0.00157565f + t*(0.00916281f + t*(-0.02057706f
			+ t*(0.02635537f + t*(-0.01647633f + t*0.00392377f))))))));
	}

	return exp(b * cos_x) / (2 * M_PI * I0);
}

/// Attenuation term
float seeliger(float cos_th1; float cos_th2; float sg_a; float sg_s) 
{
	float al = sg_s / (sg_a + sg_s); // albedo
	float c1 = max(0.0f, cos_th1);
	float c2 = max(0.0f, cos_th2);
	if (c1 == 0.0f || c2 == 0.0f)
		return 0.0f;
	return al / (4.0f * M_PI) * c1 * c2 / (c1 + c2);
}


float radiusOfCurvature(float u; float umax; float kappa; float w; float l)
{
    // rhat determines whether the spine is a segment
    // of an ellipse, a parabole, or a hyperbola.
    // See Section 5.3.
    float rhat = 1.0f + kappa * (1.0f + 1.0f / tan(umax));

	float a = 0.5f * w;
    float R = 0;
    if (rhat == 1.0f) 
    { // circle; see Subsection 5.3.1.
        R = (0.5f * l - a * sin(umax)) / sin(umax);
    } 
    else if (rhat > 0.0f) 
    {
        float tmax = atan(rhat * tan(umax));
        float bhat = (0.5f * l - a * sin(umax)) / sin(tmax);
        float ahat = bhat / rhat;
        float t = atan(rhat * tan(u));
        R = pow(bhat * bhat * cos(t) * cos(t) + ahat * ahat * sin(t) * sin(t),(float) 1.5f) / (ahat * bhat);
	} 
    else if (rhat < 0.0f) 
    { // hyperbola; see Subsection 5.3.3.
        float tmax = -atanh(rhat * tan(umax));
		float bhat = (0.5f * l - a * sin(umax)) / sinh(tmax);
        float ahat = bhat / rhat;
        float t = -atanh(rhat * tan(u));
        R = -pow(bhat * bhat * cosh(t) * cosh(t) + ahat * ahat * sinh(t) * sinh(t), (float) 1.5f) / (ahat * bhat);
	} 
    else 
    { // rhat == 0  // parabola; see Subsection 5.3.2.
        float tmax = tan(umax);
        float ahat = (0.5f * l - a * sin(umax)) / (2 * tmax);
        float t = tan(u);
        R = 2 * ahat * pow(1 + t * t, (float) 1.5f);
    }
    return R;
}


/*
///////////////////////////////////////////////////////////////////////////////
Two most important functions evalFilamentIntegrand() and evalStapleIntegrand()
computing two types of integrants based on whether yarns are twisted or not?
//////////////////////////////////////////////////////////////////////////////
*/

float evalFilamentIntegrand(float u; float v; vector om_i; 
			 vector om_r; float alpha; float beta; float ss;
			 float umax;  float kappa; float w;    float l;
             float hWidth) 
{
		// 0 <= ss < 1.0
		if (ss < 0.0f || ss >= 1.0f)
        {
			return 0.0f;
        }
		// w * sin(umax) < l
		if (w * sin(umax) >= l)
        {
            
			return 0.0f;
        }
		// -1 < kappa < inf
		if (kappa < -1.0f)
        {
			return 0.0f;

         }
		// h is the half vector
		vector h = normalize(om_r + om_i);
 
		// u_of_v is location of specular reflection.
		float u_of_v = atan(h.y / h.z);
			
		// Check if u_of_v within the range of valid u values
		if (abs(u_of_v) < umax) {
			// n is normal to the yarn surface
			// t is tangent of the fibers.
			vector n = normalize(set(sin(v), sin(u_of_v) * cos(v), cos(u_of_v) * cos(v)));
			vector t = normalize(set(0.0f, cos(u_of_v), -sin(u_of_v)));
            
			// R is radius of curvature.
			float R = radiusOfCurvature(min(abs(u_of_v),
				(1-ss)*umax), (1-ss)*umax, kappa, w, l);
            
			// G is geometry factor.
			float a = 0.5f * w;
			vector om_i_plus_om_r = om_i + om_r;
            vector t_cross_h      = cross(t, h);
			float Gu = a * (R + a * cos(v)) / (length(om_i_plus_om_r) * abs(t_cross_h.x));
            
			// fc is phase function
			float fc = alpha + vonMises(-dot(om_i, om_r), beta);

			// A is attenuation function without smoothing.
			// As is attenuation function with smoothing.
			float A = seeliger(dot(n, om_i), dot(n, om_r), 0, 1);
			float As;
			if (ss == 0.0f)
				As = A;
			else
				As = A * (1.0f - smooth(0, 1, (abs(u_of_v) - (1.0f - ss) * umax) / (ss * umax)));
            
			// fs is scattering function.
			float fs = Gu * fc * As;

			// Domain transform.
			fs = fs * M_PI * l;


			// Highlight has constant width delta_y on screen.
			float delta_y = l * hWidth;
            //if (1==1) return delta_y;

			// Clamp y_of_v between -(l - delta_y)/2 and (l - delta_y)/2.
			float y_of_v = u_of_v * 0.5f * l / umax;
			if (y_of_v > 0.5f * (l - delta_y))
				y_of_v = 0.5f * (l - delta_y);
			else if (y_of_v < 0.5f * (delta_y - l)) 
				y_of_v = 0.5f * (delta_y - l);
			
			// Check if |y(u(v)) - y(u)| < delta_y/2.
			//if (abs(y_of_v - u * 0.5f * l / umax) < 0.5f * delta_y)
				return fs / delta_y;
		}
		return 0.0f;
	}
 


float evalStapleIntegrand(float u, v; vector om_i, om_r; 
                          float alpha, beta, psi, umax, kappa, w, l, hWidth)
{

    // w * sin(umax) < l
    if (w * sin(umax) >= l)
    {
        printf("w * sin(umax) < l"); 
	    return 0.0f;
    }

    // -1 < kappa < inf
    if (kappa < -1.0f)
    {
       printf(" // -1 < kappa < inf");
	    return 0.0f;
    }

    // h is the half vector
    vector h = normalize(om_i + om_r);

    // v_of_u is location of specular reflection.
    float D = (h.y*cos(u) - h.z*sin(u)) / (sqrt(h.x * h.x + pow(h.y * sin(u) + h.z * cos(u), (float) 2.0f)) * tan(psi));
    float v_of_u = atan2(-h.y * sin(u) - h.z * cos(u), h.x) + acos(D);

    // Check if v_of_u within the range of valid v values
    if (abs(D) < 1.0f && abs(v_of_u) < M_PI / 2.0f) 
    {
         //printf("Check if v_of_u within the range of valid v values");
        // n is normal to the yarn surface.
        // t is tangent of the fibers.
	
	    vector n = normalize(set(sin(v_of_u), sin(u) * cos(v_of_u), cos(u) * cos(v_of_u)));

	    /*vector t = normalize(vector(-std::cos(v_of_u) * sin(psi),
			    std::cos(u) * std::cos(psi) + sin(u) * sin(v_of_u) * sin(psi), 
			    -sin(u) * std::cos(psi) + std::cos(u) * sin(v_of_u) * sin(psi))); */

	    // R is radius of curvature.
	    float R = radiusOfCurvature(abs(u), umax, kappa, w, l);

	    // G is geometry factor.
	    float a = 0.5f * w;
	    vector om_i_plus_om_r = om_i + om_r;
	    float Gv = a * (R + a * cos(v_of_u)) / (length(om_i_plus_om_r) * dot(n, h) * abs(sin(psi+1)));
        //return Gv;

	    // fc is phase function.
	    float fc = alpha + vonMises(-dot(om_i, om_r), beta);

	    // A is attenuation function without smoothing.
	    float A = seeliger(dot(n, om_i), dot(n, om_r), 0, 1);

	    // fs is scattering function.
	    float fs = Gv * fc * A;

	    // Domain transform.
	    fs = fs * 2.0f * w * umax;
        //if (1==1) return fs;

	    // Highlight has constant width delta_x on screen.
	    float delta_x = w * hWidth;
	
	    // Clamp x_of_u between (w - delta_x)/2 and -(w - delta_x)/2.
	    float x_of_u = v_of_u * w / M_PI;
	    if (x_of_u > 0.5f * (w - delta_x))
		    x_of_u = 0.5f * (w - delta_x);
	    else if (x_of_u < 0.5f * (delta_x - w))
		    x_of_u = 0.5f * (delta_x - w);

	    // Check if |x(v(u)) - x(v)| < delta_x/2.
	    //if (abs(x_of_u - v * w / M_PI) < 0.5f * delta_x)
        //{
            //return 0.75;
		    return fs / delta_x;
         //}
    }
  
    //return fs / delta_x;
    return  0.0f;
    
}

/*
////////////////////////////////////////////////////////////////////////////////////////////////////////////
Main BSDF facility. Works in tanget space: wi is incoming light direction (Vex's L), wo outgoing (Vex's -I)/
////////////////////////////////////////////////////////////////////////////////////////////////////////////
*/

vector irawanBSDF(vector _uv; vector wi; vector wo; Weave pattern; Yarns _yarns; float repeatU; float repeatV)
{
    int yarnID = 0;
    getYarnID(yarnID, _uv, pattern, repeatU, repeatV);
    vector yuv = set(_uv.x * repeatU,(1- _uv.y) * repeatV, 0.0f);
    vector uxy = set(_uv.x * pattern.tileWidth, _uv.y * pattern.tileHeight, 0.0);
    vector cen = set((uxy.x / pattern.tileWidth)  * pattern.tileWidth  + _yarns.centerU[yarnID]       * pattern.tileWidth,
			         (uxy.y / pattern.tileHeight) * pattern.tileHeight + (1 - _yarns.centerV[yarnID]) * pattern.tileHeight, 0.0);
    

    //if (1==1) return cen;
    uxy.x =	 uxy.x - cen.x;
    uxy.y = -(uxy.y - cen.y);

    int type = _yarns.type[yarnID];
    float w  = _yarns.width[yarnID];
    float l  = _yarns.length[yarnID];
 
    // Get incident and exitant directions.
    vector om_i = normalize(wi);
    vector om_r = normalize(wo);

    float psi   = _yarns.psi[yarnID]; //FIXME
    float umax  = _yarns.umax[yarnID];
    float kappa = _yarns.kappa[yarnID];

    float dUmaxOverDWarp, dUmaxOverDWeft;
    if (type == WARP) 
    {
        dUmaxOverDWarp = pattern.dWarpUmaxOverDWarp;
        dUmaxOverDWeft = pattern.dWarpUmaxOverDWeft;
    } else 
    { // type == EWeft
        dUmaxOverDWarp = pattern.dWeftUmaxOverDWarp;
        dUmaxOverDWeft = pattern.dWeftUmaxOverDWeft;
        // Rotate xy, incident, and exitant directions pi/2 radian about z-axis 
        float tmp = uxy.x;
        uxy.x = -uxy.y;
        uxy.y = tmp;
        tmp = om_i.x;
        om_i.x = -om_i.y;
        om_i.y = tmp;
        tmp = om_r.x;
        om_r.x = -om_r.y;
        om_r.y = tmp;
    }

    // Correlated (Perlin) noise.
    float random1 = 1.0f;
    float random2 = 1.0f;

    if (pattern.period > 0.0f) 
    {
        // generate 1 seed per yarn segment
        vector pos = cen;
        float r1 = cen.x * (pattern.tileHeight * repeatV + random(pos.x) * random(2*pos.y) + cen.y)  / pattern.period;
        float r2 = cen.y * (pattern.tileWidth  * repeatU + random(pos.x) * random(2*pos.y+1)+ cen.x) /  pattern.period;
        random1 = pnoise(r1, 0);
	    random2 = pnoise(r2, 0);
	    umax = umax + random1 * dUmaxOverDWarp + random2 * dUmaxOverDWeft;
    }
    
    // Compute u and v.
    // See Chapter 6.
    float uu = uxy.y / (l / 2.0f) * umax;
    float uv = uxy.x * M_PI / w;

    float integrand;
    if (psi != 0.0f)
        integrand = evalStapleIntegrand(uu, uv, om_i, om_r, pattern.alpha, pattern.beta, psi, umax, kappa, w, l, pattern.hWidth);
    else
        integrand = evalFilamentIntegrand(uu, uv, om_i, om_r, pattern.alpha, pattern.beta, pattern.ss, umax, kappa, w, l, pattern.hWidth);


    // Initialize random number generator based on texture location.
    float intensityVariation = 1.0f;
    if (pattern.fineness > 0.0f) 
    {
        // Compute random variation and scale specular component.
        // Generate fineness^2 seeds per 1 unit of texture. 
        int index1 = (int)((cen.x + uxy.x) * pattern.fineness);
        int index2 = (int)((cen.y + uxy.y) * pattern.fineness);
        float xi = random(index1) * random(index2);
        intensityVariation = min(-log(xi), 10.0f);
    }
   
    vector result = _yarns.ks[yarnID] * (intensityVariation * integrand);

    if (type == WARP)
        result *= (pattern.warpArea + pattern.weftArea) / pattern.warpArea;
    else
        result *= (pattern.warpArea + pattern.weftArea) / pattern.weftArea;

    // turn on if want to diffuse component
    // if (hasDiffuse && !m_initialization)
    result += _yarns.kd[yarnID] * INV_PI;

    return result;// * wo.z; //intensityVariation * wo.z * _yarns.ks[yarnID] ; //result;// * wo.z;

}
