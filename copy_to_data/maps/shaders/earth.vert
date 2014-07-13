varying vec4 vPos;
uniform sampler2D bumpMap;

varying vec2 vTexCoord;
varying vec3 vNormal;
varying vec3 vEye;

void main() {
    
    vTexCoord = gl_MultiTexCoord0.xy;
    vNormal = normalize(gl_NormalMatrix * gl_Normal);

    float height = texture2D(bumpMap,vTexCoord.xy).r;

    vec4 eyeSpaceVertexPos = gl_ModelViewMatrix * gl_Vertex;
    vEye = (vec3(eyeSpaceVertexPos)) / eyeSpaceVertexPos.w;
    vPos =  vec4( gl_Vertex.xyz, 1.0 );



    gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * vPos;
    gl_FrontColor = gl_Color;
}