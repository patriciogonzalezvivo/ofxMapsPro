//
//  Globe.cpp
//
//  Created by Patricio Gonzalez Vivo on 9/6/13.
//
//

#include "Globe.h"

void Globe::selfSetup(){
    ofEnableSmoothing();
    ofSetCircleResolution(36);
    
    //  Light Flair
    //
    scattering.load(getDataPath()+"shaders/lightScattering");
    thresholdShader.load(getDataPath()+"shaders/threshold");
    flareShader.load(getDataPath()+"shaders/flare");
    modulateTextureShader.load(getDataPath()+"shaders/modulateTexture");
    sunImage.loadImage(getDataPath()+"sun.png");
    dirtImage.loadImage(getDataPath()+"lensdirt_highc.tga");
    setupNumViewports(2);
    ofFbo::Settings settings;
    settings.width = ofGetWidth();
    settings.height = ofGetHeight();
    settings.internalformat = GL_RGBA;
    lightRays.allocate(settings);
    thresholdFbo.allocate(settings);
    lensFlareFbo.allocate(ofGetWidth(),ofGetHeight());
    blur.allocate(ofGetWidth(),ofGetHeight(),GL_RGBA);
    blur.fade = 1.0;
    blur.passes = 5;
}

void Globe::selfSetupGuis(){
    guiAdd(globe);
    guiAdd(stars);
    
    guiAdd(scattering);
    guiAdd(thresholdShader);
    guiAdd(flareShader);
    guiAdd(modulateTextureShader);
}

void Globe::selfSetupSystemGui(){
    sysGui->addSlider("Sun_scale", 0, 1, &sunScale);
}

void Globe::guiSystemEvent(ofxUIEventArgs &e){
    
}

void Globe::selfBegin(){
    cout << "Linking FBO" << endl;
    
    logGui.linkRenderTarget(&lightRays);//lensFlareFbo.dst);
}

void Globe::selfUpdate(){

}

void Globe::billBoard(){
	ofVec3f objectLookAt = ofVec3f(0,0,1);
	ofVec3f objToCam = camera->getCameraPtn()->getGlobalPosition();
	objToCam.normalize();
	float theta = objectLookAt.angle(objToCam);
	
	ofVec3f axisOfRotation = objToCam.crossed(objectLookAt);
	axisOfRotation.normalize();
    
	glRotatef(-theta, axisOfRotation.x, axisOfRotation.y, axisOfRotation.z);
}

//	All in one function
//
void billBoard(ofVec3f globalCamPosition, ofVec3f globelObjectPosition){
    ofVec3f objectLookAt = ofVec3f(0,0,1);
    ofVec3f objToCam = globalCamPosition - globelObjectPosition;
    objToCam.normalize();
    float theta = objectLookAt.angle(objToCam);
    ofVec3f axisOfRotation = objToCam.crossed(objectLookAt);
    axisOfRotation.normalize();
    glRotatef(-theta, axisOfRotation.x, axisOfRotation.y, axisOfRotation.z);
}

void Globe::selfDraw(){
    ofEnableAlphaBlending();
    
    ofPushStyle();
    ofClear(0,0);
    
    ofDisableLighting();
    ofPushMatrix();
    ofPoint toLight = lights["POINT LIGHT 1"]->getPosition();
    ofTranslate(toLight*10.0);
    billBoard();
    ofScale(sunScale,sunScale,sunScale);
    ofSetColor(255);
    sunImage.draw(-1024, -1024, 2048, 2048);
    ofPopMatrix();
    ofEnableLighting();
    
    if(currentViewPort == 0){
        ofSetColor(255);
        materials["MATERIAL 1"]->begin();
        
        globe.draw();
        
        materials["MATERIAL 1"]->end();
        
        stars.draw();
    } else {
        ofSetColor(0);
        ofDrawSphere(0, 0, globe.getRadius());
        
        globe.scatterShader.begin();
        globe.scatterShader.setUniform1f("fExposure", 0.01);
        ofDrawSphere(0, 0, globe.getRadius()+globe.getRadius()*0.01);
        globe.scatterShader.end();
    }
    
    ofPopStyle();
}

void Globe::selfDrawOverlay(){

};


void Globe::selfPostDraw(){
    int width = ofGetWidth();
    int height = ofGetHeight();
    
    //  LIGHTS
    //
    lightRays.begin();
    ofClear(0,0);
    scattering.begin();
    scattering.setUniformTexture("firstPass", getRenderTarget(1).getTextureReference(), 0);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex3f(0, 0, 0);
    glTexCoord2f(width, 0); glVertex3f(width, 0, 0);
    glTexCoord2f(width, height); glVertex3f(width, height, 0);
    glTexCoord2f(0,height);  glVertex3f(0,height, 0);
    glEnd();
    scattering.end();
    lightRays.end();
    
    //  Threshold
    //
    thresholdFbo.begin();
    ofClear(0,0);
    thresholdShader.begin();
    thresholdShader.setUniformTexture("tex0", getRenderTarget(1).getTextureReference(), 0);
    thresholdShader.setUniform1i("flipXY", 1);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex3f(0, 0, 0);
    glTexCoord2f(width, 0); glVertex3f(width, 0, 0);
    glTexCoord2f(width, height); glVertex3f(width, height, 0);
    glTexCoord2f(0,height);  glVertex3f(0,height, 0);
    glEnd();
    thresholdShader.end();
    thresholdFbo.end();
    
    blur << thresholdFbo;
    blur.update();
    
    //  FLARE
    //
    lensFlareFbo.src->begin();
    ofClear(0,0);
    flareShader.begin();
    flareShader.setUniformTexture("tex0", blur, 0);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex3f(0, 0, 0);
    glTexCoord2f(width, 0); glVertex3f(width, 0, 0);
    glTexCoord2f(width, height); glVertex3f(width, height, 0);
    glTexCoord2f(0,height);  glVertex3f(0,height, 0);
    glEnd();
    flareShader.end();
    lensFlareFbo.src->end();
    
    lensFlareFbo.dst->begin();
    ofClear(0,0);
    modulateTextureShader.begin();
    modulateTextureShader.setUniformTexture("firstPass", getRenderTarget(0).getTextureReference(), 0);
    modulateTextureShader.setUniformTexture("lightRays", lightRays.getTextureReference(), 1);
    modulateTextureShader.setUniformTexture("lensFlare", lensFlareFbo.src->getTextureReference(), 2);
    modulateTextureShader.setUniformTexture("dirtImage", dirtImage.getTextureReference(), 3);
    modulateTextureShader.setUniform2f("_dirtImageRes", dirtImage.getWidth(), dirtImage.getHeight());
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex3f(0, 0, 0);
    glTexCoord2f(width, 0); glVertex3f(width, 0, 0);
    glTexCoord2f(width, height); glVertex3f(width, height, 0);
    glTexCoord2f(0,height);  glVertex3f(0,height, 0);
    glEnd();
    modulateTextureShader.end();
    lensFlareFbo.dst->end();
    
    getRenderTarget(0).draw(0, 0);
    lensFlareFbo.dst->draw(0, 0);
    
    if(bDebug){
        ofPushMatrix();
        ofScale(0.25,0.25);
        lensFlareFbo.src->draw(0, 0);
        lensFlareFbo.dst->draw(lensFlareFbo.getWidth(), 0);
        lightRays.draw(0, lensFlareFbo.getHeight());
        
        ofPopMatrix();
    }
}


void Globe::selfWindowResized(ofResizeEventArgs & args){
    ofFbo::Settings settings;
    settings.width = ofGetWidth();
    settings.height = ofGetHeight();
    settings.internalformat = GL_RGBA;
    lightRays.allocate(settings);
    thresholdFbo.allocate(settings);
    lensFlareFbo.allocate(ofGetWidth(),ofGetHeight());
}

