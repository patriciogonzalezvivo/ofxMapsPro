//
//  VboMapLoader.h
//
//  Created by Patricio Gonzalez Vivo on 7/11/14.
//
//


#pragma once

#include "ofMain.h"
#include "ofThread.h"

#include "Location.h"

//  http://en.wikipedia.org/wiki/List_of_map_projections
//
enum MapProjectionType{
    MAP_PROJECTION_NONE = 0,
    MAP_PROJECTION_EQUIRECTANGULAR
};

struct MapFileData{
    MapFileData(){};
    
    MapFileData(string _file, float _scale, MapProjectionType _proj = MAP_PROJECTION_NONE){
        file = _file;
        projection = _proj;
        scale = _scale;
    };
    
    string file;
    MapProjectionType projection;
    float scale;
};

class VboMapLoader : public ofThread, public ofVboMesh {
public:
    VboMapLoader();
    
    virtual void load(MapFileData _File);
    
    virtual void start();
    virtual void stop();
    virtual void clear();
    
    virtual void threadedFunction();
    
    virtual void draw();
    
protected:
    virtual void loadLine();
    
    vector<MapFileData> filesQueue;
    MapFileData     current;
    
    ofBuffer    buffer;
    
    ofPoint     prevPoint;
    bool        bFirst;
    bool        bLoading;
};