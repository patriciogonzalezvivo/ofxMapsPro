uniform float exposure;
uniform float decay;
uniform float density;
uniform float weight;
uniform vec2 lightPositionOnScreen;

uniform vec2 resolution;

uniform sampler2DRect firstPass;

varying vec2 texCoordVar;

const int NUM_SAMPLES = 100;

void main(){
    vec2 texCoord = texCoordVar;
    vec2 deltaTextCoord = vec2( texCoord - lightPositionOnScreen.xy*resolution );
    deltaTextCoord *= 1.0 / float(NUM_SAMPLES) * density;
    float illuminationDecay = 1.0;
    
    vec4 tmpColor = vec4(0.0,0.0,0.0,0.0);
    
    for (int i = 0; i < NUM_SAMPLES; i++) {
        texCoord -= deltaTextCoord;
        vec4 sample = texture2DRect(firstPass, texCoord);
        sample *= illuminationDecay * (weight);
        
        tmpColor += sample;
        illuminationDecay *= decay;
    }
    
    tmpColor*= exposure*10.;
    gl_FragColor = clamp(tmpColor,vec4(0.0,0.0,0.0,0.0),vec4(1.0,1.0,1.0,1.0));
    // gl_FragColor.a = max(gl_FragColor.r,max(gl_FragColor.g,gl_FragColor.b));
}