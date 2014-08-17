//#version 120
uniform sampler2D cloudMap;
uniform float cloudAmount;
uniform float time;

varying vec4 vPos;
varying vec2 vTexCoord;
varying vec3 vNormal;
varying vec3 vEye;

int lightsNumber = 8;

void DirectionalLight(in int i, in vec3 normal,inout vec4 ambient, inout vec4 diffuse, inout vec4 specular){
    float nDotVP;
    float nDotHV;
    float pf;

    nDotVP = max(0.0, dot(normal, normalize(vec3(gl_LightSource[i].position))));
    nDotHV = max(0.0, dot(normal, vec3(gl_LightSource[i].halfVector)));

    if (nDotVP == 0.0)
        pf = 0.0;
    else
        pf = pow(nDotHV, gl_FrontMaterial.shininess);

    ambient  += gl_LightSource[i].ambient;
    diffuse  += gl_LightSource[i].diffuse * nDotVP;
    specular += gl_LightSource[i].specular * pf;
}

void PointLight(in int i, in vec3 eye, in vec3 ecPosition3, in vec3 normal, inout vec4 ambient, inout vec4 diffuse, inout vec4 specular){
    float nDotVP;
    float nDotHV;
    float pf;
    float attenuation;
    float d;
    vec3  VP;
    vec3  halfVector;

    VP = vec3(gl_LightSource[i].position) - ecPosition3;

    d = length(VP);

    VP = normalize(VP);

    attenuation = 1.0 / (gl_LightSource[i].constantAttenuation +
                         gl_LightSource[i].linearAttenuation * d +
                         gl_LightSource[i].quadraticAttenuation * d * d);

    halfVector = normalize(VP + eye);

    nDotVP = max(0.0, dot(normal, VP));
    nDotHV = max(0.0, dot(normal, halfVector));

    if (nDotVP == 0.0)
        pf = 0.0;
    else
        pf = pow(nDotHV, gl_FrontMaterial.shininess);

    ambient += gl_LightSource[i].ambient * attenuation;
    diffuse += gl_LightSource[i].diffuse * nDotVP * attenuation;
    specular += gl_LightSource[i].specular * pf * attenuation;
}

void SpotLight(in int i, in vec3 eye, vec3 ecPosition3, in vec3 normal, inout vec4 ambient, inout vec4 diffuse, inout vec4 specular){
    float nDotVP, nDotHV, pf, spotDot, spotAttenuation, attenuation, d;
    vec3 VP, halfVector;

    VP = vec3(gl_LightSource[i].position) - ecPosition3;

    d = length(VP);

    VP = normalize(VP);

    attenuation = 1.0 / (gl_LightSource[i].constantAttenuation +
                         gl_LightSource[i].linearAttenuation * d +
                         gl_LightSource[i].quadraticAttenuation * d * d);

    spotDot = dot(-VP, normalize(gl_LightSource[i].spotDirection));

    if (spotDot < gl_LightSource[i].spotCosCutoff)
        spotAttenuation = 0.0;
    else
        spotAttenuation = pow(spotDot, gl_LightSource[i].spotExponent);

    attenuation *= spotAttenuation;

    halfVector = normalize(VP + eye);

    nDotVP = max(0.0, dot(normal, VP));
    nDotHV = max(0.0, dot(normal, halfVector));

    if (nDotVP == 0.0)
        pf = 0.0;
    else
        pf = pow(nDotHV, gl_FrontMaterial.shininess);

    ambient  += gl_LightSource[i].ambient * attenuation;
    diffuse  += gl_LightSource[i].diffuse * nDotVP * attenuation;
    specular += gl_LightSource[i].specular * pf * attenuation;
}

vec4 calc_lighting_color(in vec3 _ecPosition, in vec3 _normal) {
    vec3 eye = vec3(0.0, 0.0, 1.0);

    vec4 amb  = vec4(0.0);
    vec4 diff = vec4(0.0);
    vec4 spec = vec4(0.0);

    for (int i = 0; i < lightsNumber; i++){
        if (gl_LightSource[i].position.w == 0.0)
            DirectionalLight(i, normalize(_normal), amb, diff, spec);
        else if (gl_LightSource[i].spotCutoff == 180.0)
            PointLight(i, eye, _ecPosition, normalize(_normal), amb, diff, spec);
        else
            SpotLight(i, eye, _ecPosition, normalize(_normal), amb, diff, spec);
    }

    vec4 diffuseColor = gl_FrontMaterial.diffuse;

    return  gl_FrontLightModelProduct.sceneColor +
            amb * gl_FrontMaterial.ambient +
            diff * diffuseColor +
            spec * gl_FrontMaterial.specular;
}

float fg = 0.5555555;
float fg2 = 0.0;
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
    vec4 color = vec4(0.);
    vec4 clouds = vec4(0.);
    vec4 scatter = vec4(0.);

    {
        //  Atmospheric Scatter
        //
        float cosTheta = abs( dot( normalize(vEye), vNormal) );
        float fCos = 1.0-cosTheta;
        float fCos2 = fCos*fCos;
        scatter = getRayleighPhase(fCos2) * gl_SecondaryColor + getMiePhase(fCos, fCos2, fg, fg2*fg2) * gl_Color;
        scatter.a = max(max(scatter.r, scatter.g), scatter.b);
        scatter = 1.0 - exp(-fExposure * scatter);
    }

    {
        vec2 newCloudsCoordinate = vec2(vTexCoord.x + time/1000.0 , vTexCoord.y);
        vec2 position = (newCloudsCoordinate-0.5)*2.0;
        float len = length(position);
        vec2 dir = position/len;

        newCloudsCoordinate += dir*cos(len*10.0-time*0.05)*0.01;

        clouds = vec4(texture2D(cloudMap, newCloudsCoordinate).r);
        clouds.rgb = calc_lighting_color(vEye,vNormal).rgb*(3.*(1.-scatter.a));
        clouds.a *= cloudAmount;
    }

    color = mix(clouds,scatter,scatter.a);

    gl_FragColor = color;
}