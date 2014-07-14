//
//  Star.h
//  PrintMaking
//
//  Created by Patricio Gonzalez Vivo on 9/24/13.
//
//

#pragma once
#include "ofMain.h"

class Star : public ofPoint {
public:
    
    Star();
    void    place(float _ra, float _dec, float _radius );

    ofQuaternion getQuaternion(){ return quaternion; };
    
	void	draw();
    
    string  constelationName;
    float   constelationAlpha;
    
    Star    *connect;
    
protected:
    ofQuaternion quaternion;
};