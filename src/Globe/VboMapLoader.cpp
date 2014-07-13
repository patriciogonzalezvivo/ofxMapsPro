//
//  VboMapLoader.cpp
//  Monster
//
//  Created by Patricio Gonzalez Vivo on 7/11/14.
//
//

#include "VboMapLoader.h"

VboMapLoader::VboMapLoader(){
    bFirst = true;
    bLoading = false;
    setMode(OF_PRIMITIVE_LINES);
    
    
}

void VboMapLoader::load(MapFileData _filePath){
    
    filesQueue.push_back(_filePath);
    
    if(!bLoading){
        start();
    }
}

void VboMapLoader::start(){
    if(!isThreadRunning()){
        startThread();
    }
}

void VboMapLoader::stop(){
    stopThread();
    bLoading = false;
}

void VboMapLoader::clear(){
    if(bLoading){
        stop();
    }
    filesQueue.clear();
    ofVboMesh::clear();
}

void VboMapLoader::threadedFunction(){
    while(isThreadRunning()){
        if(lock()){
            
            if(!buffer.isLastLine()){
                loadLine();
            } else {
                if (filesQueue.size() > 0) {
                    
                    current = filesQueue[0];
                    
                    cout << "Load " << current.file << endl;
                    buffer = ofBufferFromFile(current.file);
                    
                    filesQueue.erase(filesQueue.begin()+0);
                    bFirst = true;
                    
                } else {
                    cout << "Loading Complete" << endl;
                    stop();
                }
            }
            
            unlock();
        } else {
            ofLogWarning("threadedFunction()") << "Unable to lock mutex.";
        }
    }
}

void VboMapLoader::loadLine(){
    
    string temp = buffer.getNextLine();
    if(temp.length() != 0) {
        vector<string> values = ofSplitString(temp, " ");
        
        if ( values[0] == "segment"){
            bFirst = true;
        } else {
            ofPoint newPoint;
            if(current.projection == MAP_PROJECTION_NONE){
                
                //  3D GLOBE
                //
                ofVec3f displacementFromCenter = ofVec3f(0,0,-current.scale);
                
                ofQuaternion latRot;
                latRot.makeRotate( ofToFloat(values[0]), 1, 0, 0);
                
                ofQuaternion longRot;
                longRot.makeRotate( ofToFloat(values[1]), 0, 1, 0);
                
                newPoint = latRot * longRot * displacementFromCenter;
            } else {
                Location loc = Location(ofToDouble(values[0]),ofToDouble(values[1]));
                newPoint = loc.getMercator()/10000.0;
            }
            
            if (!bFirst){
                
                addVertex(prevPoint);
//                addNormal(prevPoint.getNormalized());
                
                addVertex(newPoint);
//                addNormal(newPoint.getNormalized());
            } else {
                bFirst = false;
            }
            prevPoint = newPoint;
        }
    }
}

void VboMapLoader::draw(){
    if ( lock() ) {
        ofVboMesh::draw();
        unlock();
    } else {
        ofLogWarning("threadedFunction()") << "Unable to lock mutex.";
    }
}