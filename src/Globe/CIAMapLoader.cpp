//
//  CIAMapLoader.cpp
//  Monster
//
//  Created by Patricio Gonzalez Vivo on 7/10/14.
//
//

#include "CIAMapLoader.h"

CIAMapLoader::CIAMapLoader(){
    bFirst = true;
    bLoading = false;
    mesh.setMode(OF_PRIMITIVE_LINES);
}

void CIAMapLoader::load(CIAData _filePath){
    
    filesQueue.push_back(_filePath);
    
    if(!bLoading){
        start();
    }
}

void CIAMapLoader::start(){
    if(!isThreadRunning()){
        startThread();
    }
}

void CIAMapLoader::stop(){
    stopThread();
}

void CIAMapLoader::threadedFunction(){
    while(isThreadRunning()){
        if(lock()){
            
            if(!buffer.isLastLine()){
                string temp = buffer.getNextLine();
                if(temp.length() != 0) {
                    vector<string> values = ofSplitString(temp, " ");
                    
                    if ( values[0] == "segment"){
                        bFirst = true;
                    } else {
                        
                        ofPoint newPoint;
                        
                        if(current.projection == GLOBE){
                            ofVec3f displacementFromCenter = ofVec3f(0,0,-current.radius);
                            
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
                            
                            mesh.addVertex(prevPoint);
                            mesh.addNormal(prevPoint.getNormalized());
                            
                            mesh.addVertex(newPoint);
                            mesh.addNormal(newPoint.getNormalized());
                        } else {
                            bFirst = false;
                        }
                        
                        prevPoint = newPoint;
                    }
                }
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

void CIAMapLoader::draw(){
    if ( lock() ) {
        mesh.draw();
        unlock();
    } else {
        ofLogWarning("threadedFunction()") << "Unable to lock mutex.";
    }
}

ofVboMesh CIAMapLoader::getMesh(){
    ofScopedLock lock(mutex);
    return mesh;
}