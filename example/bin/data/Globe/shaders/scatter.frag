//#version 120

varying vec4 vPos;
varying vec2 vTexCoord;
varying vec3 vNormal;
varying vec3 vEye;

float fg = 0.59;
float fg2 = 0.;
uniform float fExposure;

// Mie phase function
float getMiePhase(float fCos, float fCos2, float g, float g2){
   return 1.5 * ((1.0 - g2) / (2.0 + g2)) * (1.0 + fCos2) / pow(1.0 + g2 - 2.0*g*fCos, 1.5);   
}

// Rayleigh phase function
float getRayleighPhase(float fCos2){
   // return 0.75 + 0.75 * fCos2;
   return 0.75 * (2.0 + 0.5 * fCos2);
}

void main (void){
	float cosTheta = abs( dot( normalize(vEye), vNormal) );
    float fCos = 1.0-cosTheta;
    float fCos2 = fCos*fCos;
    vec4 scatter = getRayleighPhase(fCos2) * gl_SecondaryColor + getMiePhase(fCos, fCos2, fg, fg2*fg2) * gl_Color;
    scatter.a = max(max(scatter.r, scatter.g), scatter.b);
    scatter = 1.0 - exp(-fExposure * scatter);

    gl_FragColor = scatter;
}