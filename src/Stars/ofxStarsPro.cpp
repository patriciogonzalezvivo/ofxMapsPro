//
//  ofxStarsPro.cpp
//  DV
//
//  Created by Patricio Gonzalez Vivo on 8/13/13.
//
//

#include "ofxStarsPro.h"

ofxStarsPro::ofxStarsPro(){
    radius = 300;
    bFirst = true;
    bLoading = false;
    setMode(OF_PRIMITIVE_POINTS);
    
    load("maps/constelations.txt");
}

void ofxStarsPro::setupUI(){
    gui->addSlider("Radius", 0.0, 500.0, &radius);
    gui->addSlider("PointSize", 0.0, 10.0, &pointSize);
    gui->addSlider("Alpha", 0.0, 1.0, &alpha);
    
    gui->addSlider("Constelations",0,1, &constelationAlpha);
}

void ofxStarsPro::guiEvent(ofxUIEventArgs &e){
    string name = e.widget->getName();
    
    if(name == "Radius"){
        clear();
        load("maps/constelations.txt");
    }
}

void ofxStarsPro::setRadius(float _size){
    radius = _size;
}

void ofxStarsPro::clear(){
    VboMapLoader::clear();
    constelations.clear();
    
    for (int i = stars.size()-1; i > 0 ; i--) {
        delete stars[i];
        stars.erase(stars.begin()+i);
    }
    stars.clear();
}

void ofxStarsPro::load( string _file ){
    lastConstelation = "";
    VboMapLoader::load(MapFileData(_file,radius));
}

void ofxStarsPro::loadLine(){
    string temp = buffer.getNextLine();
    if(temp.length() != 0) {
        vector<string> values = ofSplitString(temp, " ");
        
        while(!buffer.isLastLine()) {
            string temp = buffer.getNextLine();
            
            if(temp.length() != 0) {
                vector<string> values = ofSplitString(temp, ",");
                
                if ( values[0] != lastConstelation ){
                    constelations.push_back(values[0]);
                }
                
                lastConstelation = values[0];
                
                Star *a = new Star();
                a->place(ofToFloat(values[2]), ofToFloat(values[1]),radius );
                a->constelationName = lastConstelation;
                
                Star *b = new Star();
                b->place(ofToFloat(values[4]), ofToFloat(values[3]),radius );
                b->constelationName = lastConstelation;
                b->connect = a;
                
                stars.push_back(a);
                stars.push_back(b);
                
                addVertex( *((ofPoint*)a) );
                addVertex( *((ofPoint*)b) );
            }
        }
    }
}

void ofxStarsPro::draw(){
    
    if (bEnable){
        
        if ( lock() ) {
            ofDisableLighting();
            ofPushStyle();
            
            if(constelationAlpha>0.){
                for(int i = 0; i < stars.size(); i++){
                    stars[i]->constelationAlpha = constelationAlpha;
                    stars[i]->draw();
                }
            }
            
            ofSetColor(255,alpha*255.);
            glPointSize(pointSize);
            ofVboMesh::draw();
            
            ofPopStyle();
            ofEnableLighting();
            unlock();
        } else {
            ofLogWarning("threadedFunction()") << "Unable to lock mutex.";
        }
    }
}