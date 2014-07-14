//
//  ofxStarsPro.h
//
//  Created by Patricio Gonzalez Vivo on 8/13/13.
//
//

#pragma once
#include "UIClass.h"
#include "Star.h"
#include "VboMapLoader.h"

class ofxStarsPro : public UIClass , public VboMapLoader {
public:
    ofxStarsPro();
    
    void    setRadius(float _size);
    
    void    setupUI();
    void    load( string _file );
    void    clear();
    
    string  getClassName(){return "STARS";}
    float   getRadius(){return radius;}
    
    void    draw();
    
protected:
    void    guiEvent(ofxUIEventArgs &e);
    
    void    loadLine();
    vector< Star* > stars;
    vector< string > constelations;
    float   constelationAlpha;
    
    string  lastConstelation;
    float   radius;
    float   pointSize;
    float   alpha;
};