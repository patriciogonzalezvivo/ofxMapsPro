//
//  ofxGlobePro.cpp
//  Monster
//
//  Created by Patricio Gonzalez Vivo on 7/11/14.
//
//

#include "ofxGlobePro.h"

#define STRINGIFY(A) #A

ofxGlobePro::ofxGlobePro(){

    string vShader = STRINGIFY(varying vec4 vPos;
                                    varying vec2 vTexCoord;
                                    varying vec3 vNormal;
                                    varying vec3 vEye;
                                    
                                    const float PI = 3.1415926535897932384626433832795;
                                    const float Km = 0.0015;
                                    const float Kr = 0.0025;
                                    
                                    float fInnerRadius = 0.14;
                                    float fOuterRadius = 1.1;
                                    float fScaleDepth = 1.0;
                                    float fSamples = 5.0;
                                    int nSamples = int(fSamples);
                                    float fScale = 1.0/(fOuterRadius-fInnerRadius);
                                    float fScaleOverScaleDepth = fScale/fScaleDepth;
                                    float fInvScaleDepth = (1.0 / fScaleDepth);
                                    float fOuterRadius2 = fOuterRadius*fOuterRadius;
                                    float fKm4PI = Km*4.0*PI;
                                    float fKr4PI = Kr*4.0*PI;
                                    
                                    float getNearIntersection(vec3 v3Pos, vec3 v3Ray, float fDistance2, float fRadius2){
                                        float B = 2.0 * dot(v3Pos, v3Ray);
                                        float C = fDistance2 - fRadius2;
                                        float fDet = max(0.0, B*B - 4.0 * C);
                                        return 0.5 * (-B - sqrt(fDet));
                                    }
                                    
                                    float scale(float fCos){
                                        float x = 1.0 - fCos;
                                        return fScaleDepth * exp(-0.00287 + x*(0.459 + x*(3.83 + x*(-6.80 + x*5.25))));
                                    }
                                    
                                    void main() {
                                        
                                        vPos =  vec4(gl_Vertex.xyz, 1.0 );
                                        vTexCoord = gl_MultiTexCoord0.xy;
                                        vNormal = normalize(gl_NormalMatrix * gl_Normal);
                                        vec4 eyeSpaceVertexPos = gl_ModelViewMatrix * gl_Vertex;
                                        vEye = (vec3(eyeSpaceVertexPos)) / eyeSpaceVertexPos.w;
                                        gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * vec4( gl_Vertex.xyz, 1.0 );
                                        
                                        //  LUT
                                        //
                                        vec3 v3InvWavelength;
                                        v3InvWavelength.x = 1.0/pow(0.650, 4.0);
                                        v3InvWavelength.y = 1.0/pow(0.570, 4.0);
                                        v3InvWavelength.z = 1.0/pow(0.475, 4.0);
                                        
                                        //  Camera
                                        //
                                        vec3 v3CameraPos = vec4(gl_ModelViewMatrixInverse * normalize(vec4(1.))).xyz;
                                        float fCameraHeight = length(v3CameraPos);
                                        float fCameraHeight2 = fCameraHeight*fCameraHeight;
                                        
                                        //  LIGHT (Sun)
                                        //
                                        vec3 v3LightPos = vec4(gl_ModelViewMatrixInverse * gl_LightSource[0].position).xyz;
                                        float ESun = length(v3LightPos)*10.;//gl_LightSource[0].diffuse.rgb)*20.;
                                        float fKrESun = Kr*ESun;
                                        float fKmESun = Km*ESun;
                                        
                                        vec3 v3Pos = gl_Vertex.xyz;
                                        vec3 v3Ray = v3Pos - v3CameraPos;
                                        float fFar = length(v3Ray);
                                        v3Ray /= fFar;
                                        
                                        float fNear = getNearIntersection(v3CameraPos, v3Ray, fCameraHeight2, fOuterRadius2);
                                        
                                        vec3 v3Start = v3CameraPos + v3Ray * fNear;
                                        fFar -= fNear;
                                        
                                        float fStartAngle = dot(v3Ray, v3Start) / fOuterRadius;
                                        float fStartDepth = exp(-fInvScaleDepth);
                                        float fStartOffset = fStartDepth*scale(fStartAngle);
                                        
                                        float fDepth = exp((fInnerRadius - fOuterRadius) / fScaleDepth);
                                        float fCameraAngle = dot(-v3Ray, v3Pos) / length(v3Pos);
                                        float fLightAngle = dot(normalize(v3LightPos), v3Pos) / length(v3Pos);
                                        float fCameraScale = scale(fCameraAngle);
                                        float fLightScale = scale(fLightAngle);
                                        float fCameraOffset = fDepth*fCameraScale;
                                        float fTemp = (fLightScale + fCameraScale);
                                        
                                        float fSampleLength = fFar / fSamples;
                                        float fScaledLength = fSampleLength * fScale;
                                        vec3 v3SampleRay = v3Ray * fSampleLength;
                                        vec3 v3SamplePoint = v3Start + v3SampleRay * 0.5;
                                        
                                        vec3 v3FrontColor = vec3(0.0);
                                        for(int i=0; i<nSamples; i++){
                                            float fHeight = length(v3SamplePoint);
                                            float fDepth = exp(fScaleOverScaleDepth * (fInnerRadius - fHeight));
                                            float fScatter = fDepth*fTemp - fCameraOffset;
                                            vec3 v3Attenuate = exp(-fScatter * (v3InvWavelength * fKr4PI + fKm4PI));
                                            v3FrontColor += v3Attenuate * (fDepth * fScaledLength);
                                            v3SamplePoint += v3SampleRay;
                                        }
                                        
                                        gl_FrontColor = vec4(v3FrontColor * (v3InvWavelength * fKrESun), 1.0);
                                        gl_FrontSecondaryColor = vec4(v3FrontColor*fKmESun,1.0);
                                    }
                                    );
    
    string fShader = STRINGIFY(varying vec4 vPos;
                                  varying vec2 vTexCoord;
                                  varying vec3 vNormal;
                                  varying vec3 vEye;
                                  
                                  float fg = 0.59;
                                  float fg2 = 0.;
                                  uniform float fExposure;
                                  
                                  float getMiePhase(float fCos, float fCos2, float g, float g2){
                                      return 1.5 * ((1.0 - g2) / (2.0 + g2)) * (1.0 + fCos2) / pow(1.0 + g2 - 2.0*g*fCos, 1.5);
                                  }
                                  
                                  float getRayleighPhase(float fCos2){
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
                                  );
    
    scatterShader.setupShaderFromSource(GL_VERTEX_SHADER, vShader);
    scatterShader.setupShaderFromSource(GL_FRAGMENT_SHADER, fShader);
    scatterShader.linkProgram();
    
    loadResources("maps");
    
    vertexShader = "#version 120\n\n\
varying vec4 vPos;\n\
uniform sampler2D bumpMap;\n\
\n\
varying vec2 vTexCoord;\n\
varying vec3 vNormal;\n\
varying vec3 vEye;\n\
\n\
void main(){\n\
\n\
    vTexCoord = gl_MultiTexCoord0.xy;\n\
    vNormal = normalize(gl_NormalMatrix * gl_Normal);\n\
\n\
    float height = texture2D(bumpMap,vTexCoord.xy).r;\n\
\n\
    vec4 eyeSpaceVertexPos = gl_ModelViewMatrix * gl_Vertex;\n\
    vEye = (vec3(eyeSpaceVertexPos)) / eyeSpaceVertexPos.w;\n\
    vPos =  vec4( gl_Vertex.xyz, 1.0 );\n\
\n\
    gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * vPos;\n\
    gl_FrontColor = gl_Color;\n\
}";
    
    fragmentShader = "#version 120\n\
\n\
uniform sampler2D earth_day;\n\
uniform sampler2D earth_night;\n\
uniform sampler2D normalMap;\n\
uniform sampler2D bumpMap;\n\
uniform sampler2D specularMap;\n\
\n\
uniform float useNormal;\n\
uniform float useRim;\n\
uniform float useTex;\n\
uniform float useTransparency;\n\
uniform float useSpecular;\n\
\n\
varying vec4 vPos;\n\
varying vec2 vTexCoord;\n\
varying vec3 vNormal;\n\
varying vec3 vEye;\n\
\n\
int lightsNumber = 8;\n\
\n\
void DirectionalLight(in int i, in vec3 normal,inout vec4 ambient, inout vec4 diffuse, inout vec4 specular){\n\
    float nDotVP;\n\
    float nDotHV;\n\
    float pf;\n\
    \n\
    nDotVP = max(0.0, dot(normal, normalize(vec3(gl_LightSource[i].position))));\n\
    nDotHV = max(0.0, dot(normal, vec3(gl_LightSource[i].halfVector)));\n\
    \n\
    if (nDotVP == 0.0)\n\
        pf = 0.0;\n\
    else\n\
        pf = pow(nDotHV, gl_FrontMaterial.shininess);\n\
    \n\
    ambient  += gl_LightSource[i].ambient;\n\
    diffuse  += gl_LightSource[i].diffuse * nDotVP;\n\
    specular += gl_LightSource[i].specular * pf;\n\
}\n\
\n\
void PointLight(in int i, in vec3 eye, in vec3 ecPosition3, in vec3 normal, inout vec4 ambient, inout vec4 diffuse, inout vec4 specular){\n\
    float nDotVP;\n\
    float nDotHV;\n\
    float pf;\n\
    float attenuation;\n\
    float d;\n\
    vec3  VP;\n\
    vec3  halfVector;\n\
    \n\
    VP = vec3(gl_LightSource[i].position) - ecPosition3;\n\
        \n\
    d = length(VP);\n\
    \n\
    VP = normalize(VP);\n\
    \n\
    attenuation = 1.0 / (gl_LightSource[i].constantAttenuation +\n\
                         gl_LightSource[i].linearAttenuation * d +\n\
                         gl_LightSource[i].quadraticAttenuation * d * d);\n\
    \n\
    halfVector = normalize(VP + eye);\n\
    \n\
    nDotVP = max(0.0, dot(normal, VP));\n\
    nDotHV = max(0.0, dot(normal, halfVector));\n\
    \n\
    if (nDotVP == 0.0)\n\
        pf = 0.0;\n\
    else\n\
        pf = pow(nDotHV, gl_FrontMaterial.shininess);\n\
    \n\
    ambient += gl_LightSource[i].ambient * attenuation;\n\
    diffuse += gl_LightSource[i].diffuse * nDotVP * attenuation;\n\
    specular += gl_LightSource[i].specular * pf * attenuation;\n\
}\n\
\n\
void SpotLight(in int i, in vec3 eye, vec3 ecPosition3, in vec3 normal, inout vec4 ambient, inout vec4 diffuse, inout vec4 specular){\n\
    float nDotVP, nDotHV, pf, spotDot, spotAttenuation, attenuation, d;\n\
    vec3 VP, halfVector;\n\
    \n\
    VP = vec3(gl_LightSource[i].position) - ecPosition3;\n\
    \n\
    d = length(VP);\n\
    \n\
    VP = normalize(VP);\n\
    \n\
    attenuation = 1.0 / (gl_LightSource[i].constantAttenuation +\n\
                         gl_LightSource[i].linearAttenuation * d +\n\
                         gl_LightSource[i].quadraticAttenuation * d * d);\n\
    \n\
    spotDot = dot(-VP, normalize(gl_LightSource[i].spotDirection));\n\
    \n\
    if (spotDot < gl_LightSource[i].spotCosCutoff)\n\
        spotAttenuation = 0.0;\n\
    else\n\
        spotAttenuation = pow(spotDot, gl_LightSource[i].spotExponent);\n\
    \n\
    attenuation *= spotAttenuation;\n\
    \n\
    halfVector = normalize(VP + eye);\n\
    \n\
    nDotVP = max(0.0, dot(normal, VP));\n\
    nDotHV = max(0.0, dot(normal, halfVector));\n\
    \n\
    if (nDotVP == 0.0)\n\
        pf = 0.0;\n\
    else\n\
        pf = pow(nDotHV, gl_FrontMaterial.shininess);\n\
    \n\
    ambient  += gl_LightSource[i].ambient * attenuation;\n\
    diffuse  += gl_LightSource[i].diffuse * nDotVP * attenuation;\n\
    specular += gl_LightSource[i].specular * pf * attenuation;\n\
}\n\
\n\
vec4 calc_lighting_color(in vec3 _ecPosition, in vec3 _normal) {\n\
    vec3 eye = vec3(0.0, 0.0, 1.0);\n\
    \n\
    vec4 amb  = vec4(0.0);\n\
    vec4 diff = vec4(0.0);\n\
    vec4 spec = vec4(0.0);\n\
    \n\
    for (int i = 0; i < lightsNumber; i++){\n\
        if (gl_LightSource[i].position.w == 0.0)\n\
            DirectionalLight(i, normalize(_normal), amb, diff, spec);\n\
        else if (gl_LightSource[i].spotCutoff == 180.0)\n\
            PointLight(i, eye, _ecPosition, normalize(_normal), amb, diff, spec);\n\
        else\n\
            SpotLight(i, eye, _ecPosition, normalize(_normal), amb, diff, spec);\n\
    }\n\
\n\
    vec4 diffuseColor = gl_FrontMaterial.diffuse;\n\
    \n\
    vec4 specular = gl_FrontMaterial.specular;\n\
    if(useSpecular>0.){\n\
        vec2 st = vTexCoord;\n\
        st.y = 1.-st.y;\n\
        specular += texture2D(specularMap,st)*useSpecular;\n\
    }\n\
    \n\
    return  gl_FrontLightModelProduct.sceneColor +\n\
            amb * gl_FrontMaterial.ambient +\n\
            diff * diffuseColor +\n\
            spec * specular;\n\
}\n\
\n\
vec3 random3(vec3 c) {\n\
    float j = 4096.0*sin(dot(c,vec3(17.0, 59.4, 15.0)));\n\
    vec3 r;\n\
    r.z = fract(512.0*j);\n\
    j *= .125;\n\
    r.x = fract(512.0*j);\n\
    j *= .125;\n\
    r.y = fract(512.0*j);\n\
    return r-0.5;\n\
}\n\
\n\
const float F3 =  0.3333333;\n\
const float G3 =  0.1666667;\n\
float simplexNoise(vec3 p) {\n\
    \n\
    vec3 s = floor(p + dot(p, vec3(F3)));\n\
    vec3 x = p - s + dot(s, vec3(G3));\n\
    \n\
    vec3 e = step(vec3(0.0), x - x.yzx);\n\
    vec3 i1 = e*(1.0 - e.zxy);\n\
    vec3 i2 = 1.0 - e.zxy*(1.0 - e);\n\
    \n\
    vec3 x1 = x - i1 + G3;\n\
    vec3 x2 = x - i2 + 2.0*G3;\n\
    vec3 x3 = x - 1.0 + 3.0*G3;\n\
    \n\
    vec4 w, d;\n\
    \n\
    w.x = dot(x, x);\n\
    w.y = dot(x1, x1);\n\
    w.z = dot(x2, x2);\n\
    w.w = dot(x3, x3);\n\
    \n\
    w = max(0.6 - w, 0.0);\n\
    \n\
    d.x = dot(random3(s), x);\n\
    d.y = dot(random3(s + i1), x1);\n\
    d.z = dot(random3(s + i2), x2);\n\
    d.w = dot(random3(s + 1.0), x3);\n\
    \n\
    w *= w;\n\
    w *= w;\n\
    d *= w;\n\
    \n\
    return dot(d, vec4(52.0));\n\
}\n\
\n\
uniform float oceanNoisePct;\n\
uniform float oceanNoiseZoom;\n\
uniform float oceanNoiseSpeed;\n\
uniform float time;\n\
\n\
vec3 normalNoise(vec2 _st, float _zoom, float _speed){\n\
    vec2 v1 = _st;\n\
    vec2 v2 = _st;\n\
    vec2 v3 = _st;\n\
    float expon = pow(10., _zoom);\n\
    v1 /= 1.0*expon;\n\
    v2 /= 0.62*expon;\n\
    v3 /= 0.83*expon;\n\
    float n = time*_speed;\n\
    float nr = (simplexNoise(vec3(v1, n)) + simplexNoise(vec3(v2, n)) + simplexNoise(vec3(v3, n))) / 6.0 + 0.5;\n\
    n = time * _speed + 1000.0;\n\
    float ng = (simplexNoise(vec3(v1, n)) + simplexNoise(vec3(v2, n)) + simplexNoise(vec3(v3, n))) / 6.0 + 0.5;\n\
    return vec3(nr,ng,1.0);\n\
}\n\
\n\
void main (void){\n\
    vec3 N = normalize(vNormal);\n\
    vec2 st = vTexCoord;\n\
    st.y = 1.-st.y;\n\
    \n\
    vec4 day = texture2D(earth_day, st);\n\
    vec4 night = texture2D(earth_night, st);\n\
    \n\
    vec3 normalTex = vec3(0.5);\n\
    if (useNormal>0.){\n\
        \n\
        float spec = texture2D(specularMap,st).r;\n\
        normalTex = texture2D(normalMap, st).rgb;\n\
        \n\
        if(oceanNoisePct>0.&&spec>0.){\n\
            vec3 noiseTexture = normalNoise(st*vec2(1000.0,2000.0),oceanNoiseZoom,oceanNoiseSpeed);\n\
            normalTex = mix(normalTex,noiseTexture,oceanNoisePct*spec);\n\
        }\n\
        normalTex = normalTex*2.0-1.;\n\
        normalTex.xy *= useNormal*2.0;\n\
        \n\
        vec3 T = vec3(0.,1.,0.);\n\
        vec3 BT = normalize( cross( vNormal, T ) * -1.0 );\n\
        mat3 tsb = mat3( normalize( T ), normalize( BT ), normalize( vNormal ) );\n\
        N = tsb * normalTex;\n\
    }\n\
    \n\
    vec4 light = calc_lighting_color(vEye,N);\n\
    \n\
    vec4 color = light;\n\
    if(useTex>0.){\n\
        color = mix(light,mix(night,day,length(light.rgb) ),useTex);\n\
    }\n\
    \n\
    if(useRim>0.){\n\
        float cosTheta = abs( dot( normalize(vEye), N) );\n\
        float f = useRim * ( 1. - smoothstep( 0.0, 1., pow(cosTheta,0.2) ) );\n\
        color.rgb += (1.0-gl_SecondaryColor.rgb)*f;\n\
    }\n\
    \n\
    gl_FragColor.rgb = color.rgb;\n\
    \n\
    if(useTransparency>0.){\n\
        float cosTheta = abs( dot( normalize(vEye), N) );\n\
        float fresnel = pow(1.0 - cosTheta, 7.0);\n\
        \n\
        gl_FragColor.a = mix(gl_Color.a,fresnel,useTransparency);\n\
    } else {\n\
        gl_FragColor.a = 1.;\n\
    }\n\
}";
    
    setupShaderFromSource(GL_VERTEX_SHADER, vertexShader);
    setupShaderFromSource(GL_FRAGMENT_SHADER, fragmentShader);
    
    linkProgram();
    
    extractUniforms(fragmentShader);
}

void ofxGlobePro::setupUI(){
    
    gui->addSlider("Radius", 10, 100, &radius);
    gui->addSlider("Scatter_Exp", 0, 1., &scatterExpo);
    gui->addSpacer();
    
    gui->addLabel("Vector");
    gui->addToggle("Coast", &bCoast);
    addUIColor("Coast_Color", &cCoast);
    gui->addSlider("Coast_Alpha",0,1,&cCoast.alpha);
    
    gui->addToggle("Rivers", &bRivers);
    addUIColor("Rivers_Color", &cRivers);
    gui->addSlider("Rivers_Alpha",0,1, &cRivers.alpha);
    
    gui->addToggle("Borders", &bBorders);
    addUIColor("Borders_Color", &cBorders);
    gui->addSlider("Borders_Alpha",0,1,&cBorders.alpha);
    
    gui->addSpacer();
    gui->addLabel("Textures");
    
    UIShader::setupUI();
}

void ofxGlobePro::guiEvent(ofxUIEventArgs &e){
    string name = e.widget->getName();
    
    bool loadCoast = false;
    bool loadBorders = false;
    bool loadRivers = false;
    
    if(name == "Coast"){
        if(bCoast){
            loadCoast = true;
        } else {
            coast.clear();
        }
    } else if (name == "Borders"){
        if(bBorders){
            loadBorders = true;
        } else {
            borders.clear();
        }
    } else if (name == "Rivers"){
        if(bRivers){
            loadRivers = true;
        } else {
            rivers.clear();
        }
    } else if (name == "Radius"){
        loadCoast = bCoast;
        loadBorders = bBorders;
        loadRivers = bRivers;
    }
    
    if(loadCoast){
        coast.clear();
        coast.load(MapFileData(path+"/CIA/namer-cil.txt",radius));
        coast.load(MapFileData(path+"/CIA/samer-cil.txt",radius));
        coast.load(MapFileData(path+"/CIA/europe-cil.txt",radius));
        coast.load(MapFileData(path+"/CIA/africa-cil.txt",radius));
        coast.load(MapFileData(path+"/CIA/asia-cil.txt",radius));
    }
    if(loadBorders){
        borders.clear();
        borders.load(MapFileData(path+"/CIA/namer-bdy.txt",radius));
        borders.load(MapFileData(path+"/CIA/samer-bdy.txt",radius));
        borders.load(MapFileData(path+"/CIA/europe-bdy.txt",radius));
        borders.load(MapFileData(path+"/CIA/africa-bdy.txt",radius));
        borders.load(MapFileData(path+"/CIA/asia-bdy.txt",radius));
    }
    if(loadRivers){
        rivers.clear();
        rivers.load(MapFileData(path+"/CIA/namer-riv.txt",radius));
        rivers.load(MapFileData(path+"/CIA/samer-riv.txt",radius));
        rivers.load(MapFileData(path+"/CIA/europe-riv.txt",radius));
        rivers.load(MapFileData(path+"/CIA/africa-riv.txt",radius));
        rivers.load(MapFileData(path+"/CIA/asia-riv.txt",radius));
    }
    
    UIShader::guiEvent(e);
    
}

void ofxGlobePro::loadResources(string _path){
    path = _path;
    
    ofDisableArbTex();
	ofLoadImage(texDay,path+"/textures/texture.png");
	ofLoadImage(texNight,path+"/textures/earth_night4k.jpg");
    ofLoadImage(texNormals,path+"/textures/normal.png");
    ofLoadImage(texHeight,path+"/textures/bump.jpg");
    ofLoadImage(texSpecular,path+"/textures/specular.png");
    ofEnableArbTex();
}

string ofxGlobePro::getClassName(){
    if(fragFile.isFile()){
        return fragFile.getBaseName();
    } else {
        return "GLOBE";
    }
}

void ofxGlobePro::draw(){
    ofPushStyle();
    ofSetColor(255);
    if(bEnable ){
        begin();
        if(texDay.isAllocated())
            setUniformTexture("earth_day",texDay, 0);
        
        if(texNight.isAllocated())
            setUniformTexture("earth_night",texNight, 1);
        
        if(texNormals.isAllocated())
            setUniformTexture("normalMap",texNormals,2);
        
        if(texHeight.isAllocated())
            setUniformTexture("bumpMap",texHeight,3);
        
        if(texSpecular.isAllocated())
            setUniformTexture("specularMap",texSpecular,4);
        
        ofDrawSphere(0, 0, radius);
        end();
    }
    
    if(scatterExpo>0.){
        scatterShader.begin();
        scatterShader.setUniform1f("fExposure", scatterExpo);
        ofDrawSphere(0, 0, radius+radius*0.01);
        scatterShader.end();
    }
    ofPopStyle();
    
    ofDisableLighting();
    {
        if(bRivers){
            cRivers.update();
            ofSetColor(cRivers,cRivers.a*255.);
            rivers.draw();
        }
        
        if(bCoast){
            cCoast.update();
            ofSetColor(cCoast,cCoast.a*255.);
            coast.draw();
        }
        
        if(bBorders){
            cBorders.update();
            ofSetColor(cBorders,cBorders.a*255.);
            borders.draw();
        }
    }
    ofEnableLighting();
}

