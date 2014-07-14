uniform sampler2DRect firstPass;
uniform sampler2DRect lightRays;
uniform sampler2DRect lensFlare;

uniform sampler2DRect dirtImage;
uniform vec2 dirtImageRes;

uniform vec2 resolution;

uniform float amount;

varying vec2 texCoords;

void main(){

	vec4 original = texture2DRect(firstPass, texCoords);
	vec4 lights = texture2DRect(lightRays, texCoords);

    vec4 originalColor = texture2DRect(lensFlare, texCoords);
    vec4 secondaryColor = texture2DRect(dirtImage, texCoords / resolution * dirtImageRes);
    vec4 lensDirt = vec4(originalColor.rgb * secondaryColor.rgb * vec3(amount*20.), originalColor.a);

    gl_FragColor = original+lights+originalColor+lensDirt;//vec4(1.,0.,0.,1.);
}