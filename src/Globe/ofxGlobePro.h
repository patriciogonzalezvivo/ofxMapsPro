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
    
    ofShader    scatterShader;
    float       scatterExpo;
    
protected:
    void    guiEvent(ofxUIEventArgs &e);
    
    string      path;
    
    ofTexture   texDay, texNight;
    ofTexture   texNormals, texHeight, texSpecular;
    
    VboMapLoader coast, borders, rivers;
    aColor      cCoast, cBorders, cRivers;
    bool        bCoast, bBorders, bRivers;
    
    float       radius;
};