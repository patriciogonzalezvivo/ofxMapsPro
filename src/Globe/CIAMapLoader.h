//
//  CIAMapLoader.h//
//  Created by Patricio Gonzalez Vivo on 7/10/14.
//
//

#pragma once

#include "ofMain.h"
#include "ofThread.h"

#include "Location.h"

enum MapProjections{
    GLOBE,
    CYLINDRICAL
};

struct CIAData{
    CIAData(){};
    
    CIAData(string _file, MapProjections _proj, float _sphereRadius = -1){
        file = _file;
        projection = _proj;
        radius = _sphereRadius;
    };
    
    string file;
    MapProjections projection;
    float radius;
};

class CIAMapLoader : public ofThread {
public:
    CIAMapLoader();
    
    void load(CIAData _File);
    void start();
    void stop();
    
    void threadedFunction();
    
    void draw();
    
    ofVboMesh getMesh();
    
private:
    vector<CIAData> filesQueue;
    CIAData     current;
    
    ofBuffer    buffer;
    ofVboMesh   mesh;
    
    ofPoint     prevPoint;
    bool        bFirst;
    bool        bLoading;
};