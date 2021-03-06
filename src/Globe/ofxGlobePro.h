//
//  ofxGlobePro.h
//
//  Created by Patricio Gonzalez Vivo on 7/11/14.
//
//

#pragma once

#include "UIShader.h"

#include "VboMapLoader.h"

class ofxGlobePro : public UIShader{
public:
    
    ofxGlobePro();
    
    void    setupUI();
    void    loadResources(string _path);
    
    string  getClassName();
    float   getRadius(){return radius;}
    
    void    draw();
    
    UIShader    cloudsShader;
    
protected:
    void    guiEvent(ofxUIEventArgs &e);
    
    string      path;
    
    //  Earth
    //
    ofTexture   texDay, texNight;
    ofTexture   texNormals, texHeight, texSpecular;
    
    VboMapLoader coast, borders, rivers;
    aColor      cCoast, cBorders, cRivers;
    bool        bCoast, bBorders, bRivers;
    float       radius;
    
    //  Clouds
    //
    ofTexture   texClouds;
    float       cloudsRadius;
    
    // Atmosphere Scatter
    //
    ofShader    scatterShader;
    float       scatterExpo, atmosphereRadius;
};