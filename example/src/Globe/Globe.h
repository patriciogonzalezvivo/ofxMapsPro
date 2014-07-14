//
//  Globe.h
//
//  Created by Patricio Gonzalez Vivo on 9/6/13.
//
//

#pragma once

#include "ofxPro.h"

#include "ofxGlobePro.h"
#include "ofxStarsPro.h"

class Globe : public ofx3DPro {
public:

    string getSystemName(){ return "Globe";}

    void selfSetup();
    void selfSetupGuis();
    void selfSetupSystemGui();
    void guiSystemEvent(ofxUIEventArgs &e);
   
    void selfUpdate();
    void selfDraw();
    
    void selfDrawOverlay();
    void selfPostDraw();
    
    void selfWindowResized(ofResizeEventArgs & args);
    
protected:
    
    ofxGlobePro globe;
    ofxStarsPro stars;
    
    //  Light Flair
    //
    UIShader scattering;
    ofFbo lightRays;
    
    UIShader thresholdShader;
    ofFbo thresholdFbo;
    
    UIShader flareShader;
    UIShader modulateTextureShader;
    PingPong lensFlareFbo;
    
    ofImage sunImage;
    ofImage dirtImage;
    
    void billBoard();
    float sunScale;
};
