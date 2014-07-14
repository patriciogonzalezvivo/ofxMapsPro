uniform sampler2DRect tex0;

varying vec2 texCoord;
uniform vec2 resolution;

int nSamples = 5;
uniform float flareDispersal;
uniform float flareHaloWidth;
uniform vec3 flareChromaticDistortion;

vec3 textureDistorted(sampler2DRect tex, vec2 sample_center, vec2 sample_vector, vec3 distortion) {
	return vec3(
		texture2DRect(tex, sample_center + sample_vector * distortion.r).r,
		texture2DRect(tex, sample_center + sample_vector * distortion.g).g,
		texture2DRect(tex, sample_center + sample_vector * distortion.b).b
	);
}

void main(){
    vec2 imageCenter = vec2(0.5);
    vec2 sampleVector = (imageCenter - (texCoord / resolution)) * flareDispersal;
    vec2 haloVector = normalize(sampleVector) * flareHaloWidth*1000.0;
    
    vec3 color = textureDistorted(tex0, texCoord + haloVector, haloVector, flareChromaticDistortion).rgb * 3.0;
    for(int i = 0; i < nSamples; i++){
        vec2 offset = sampleVector * float(i);
        color += textureDistorted(tex0, texCoord + (offset * resolution), offset * resolution, flareChromaticDistortion).rgb;
    }
        
    gl_FragColor = vec4(color, 1.);//max(color.r,max(color.g,color.b)));
}