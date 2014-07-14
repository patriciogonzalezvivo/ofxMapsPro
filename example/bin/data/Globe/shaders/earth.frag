//#version 120
uniform sampler2D earth_day;
uniform sampler2D earth_night;
uniform sampler2D normalMap;
uniform sampler2D bumpMap;
uniform sampler2D specularMap;

uniform float useNormal;
uniform float useRim;
uniform float useTex;
uniform float useTransparency;
uniform float useSpecular;

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

    vec4 specular = gl_FrontMaterial.specular;
    // if(useSpecular>0.){
    //     vec2 st = vTexCoord;
    //     st.y = 1.-st.y;
    //     specular *= texture2D(specularMap,st)*useSpecular;
    // }

    return  gl_FrontLightModelProduct.sceneColor +
            amb * gl_FrontMaterial.ambient +
            diff * diffuseColor +
            spec * specular;
}

// Simplex 3D noise
//
vec3 random3(vec3 c) {
    float j = 4096.0*sin(dot(c,vec3(17.0, 59.4, 15.0)));
    vec3 r;
    r.z = fract(512.0*j);
    j *= .125;
    r.x = fract(512.0*j);
    j *= .125;
    r.y = fract(512.0*j);
    return r-0.5;
}

const float F3 =  0.3333333;
const float G3 =  0.1666667;
float simplexNoise(vec3 p) {

    vec3 s = floor(p + dot(p, vec3(F3)));
    vec3 x = p - s + dot(s, vec3(G3));
     
    vec3 e = step(vec3(0.0), x - x.yzx);
    vec3 i1 = e*(1.0 - e.zxy);
    vec3 i2 = 1.0 - e.zxy*(1.0 - e);
        
    vec3 x1 = x - i1 + G3;
    vec3 x2 = x - i2 + 2.0*G3;
    vec3 x3 = x - 1.0 + 3.0*G3;
     
    vec4 w, d;
     
    w.x = dot(x, x);
    w.y = dot(x1, x1);
    w.z = dot(x2, x2);
    w.w = dot(x3, x3);
     
    w = max(0.6 - w, 0.0);
     
    d.x = dot(random3(s), x);
    d.y = dot(random3(s + i1), x1);
    d.z = dot(random3(s + i2), x2);
    d.w = dot(random3(s + 1.0), x3);
     
    w *= w;
    w *= w;
    d *= w;
     
    return dot(d, vec4(52.0));
}

uniform float oceanNoisePct;
uniform float oceanNoiseZoom;
uniform float oceanNoiseSpeed;
uniform float time;

vec3 normalNoise(vec2 _st, float _zoom, float _speed){
    vec2 v1 = _st;
    vec2 v2 = _st;
    vec2 v3 = _st;
    float expon = pow(10., _zoom);
    v1 /= 1.0*expon;
    v2 /= 0.62*expon;
    v3 /= 0.83*expon;
    float n = time*_speed;
    float nr = (simplexNoise(vec3(v1, n)) + simplexNoise(vec3(v2, n)) + simplexNoise(vec3(v3, n))) / 6.0 + 0.5;
    n = time * _speed + 1000.0;
    float ng = (simplexNoise(vec3(v1, n)) + simplexNoise(vec3(v2, n)) + simplexNoise(vec3(v3, n))) / 6.0 + 0.5;
    return vec3(nr,ng,1.0);
}

void main (void){
    vec3 N = normalize(vNormal);
    vec2 st = vTexCoord;
    st.y = 1.-st.y;

    vec4 day = texture2D(earth_day, st);
    vec4 night = texture2D(earth_night, st);
    float spec = texture2D(specularMap,st).r;
    
    vec3 normalTex = vec3(0.5);
    if (useNormal>0.){
        
        normalTex = texture2D(normalMap, st).rgb;
        
        if(oceanNoisePct>0.&&spec>0.){
            vec3 noiseTexture = normalNoise(st*vec2(1000.0,2000.0),oceanNoiseZoom,oceanNoiseSpeed);//vec3(1.0);
            normalTex = mix(normalTex,noiseTexture,oceanNoisePct*spec);
        }
        normalTex = normalTex*2.0-1.;
        normalTex.xy *= useNormal*2.0;

        vec3 T = vec3(0.,1.,0.);
        vec3 BT = normalize( cross( vNormal, T ) * -1.0 );
        mat3 tsb = mat3( normalize( T ), normalize( BT ), normalize( vNormal ) );
        N = tsb * normalTex;
    }
    
    vec4 light = calc_lighting_color(vEye,N);

    vec4 color = light;
    if(useTex>0.){
        color = mix(light,mix(night,day,length(light.rgb) ),useTex);
    }

    if(useRim>0.){
        float cosTheta = abs( dot( normalize(vEye), N) );
        float f = useRim * ( 1. - smoothstep( 0.0, 1., pow(cosTheta,0.2) ) );
        color.rgb += (1.0-gl_SecondaryColor.rgb)*f;
    }

    if(useSpecular>0.){
        vec2 st = vTexCoord;
        st.y = 1.-st.y;
        color = mix(vec4(vec3(0.),1.),color, texture2D(specularMap,st).r * useSpecular );
    }

    gl_FragColor.rgb = color.rgb;

    if(useTransparency>0.){
        float cosTheta = abs( dot( normalize(vEye), N) );
        float fresnel = pow(1.0 - cosTheta, 4.0);

        gl_FragColor.a = mix(gl_Color.a,fresnel,useTransparency);
    } else {
        gl_FragColor.a = 1.;
    }
}