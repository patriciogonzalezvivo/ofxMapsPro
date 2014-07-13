varying vec4 vPos;
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