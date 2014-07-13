//
//  ofxMapPro.cpp
//  PrintMaking
//
//  Created by Patricio Gonzalez Vivo on 9/25/13.
//
//

#include "ofxMapPro.h"
#include "QueueSorter.h"

ofxMapPro::ofxMapPro(){
    TileMap::TileMap();
}

void ofxMapPro::setupUI(){
    gui->addSlider("Latitud", -90.0, 90.0, &lat);
    gui->addSlider("Longitud", -180.0, 180.0, &lon);
    gui->addSlider("Zoom", 2.0, 19.0, &zoom);
    
    gui->addLabel("Provider");
    vector<string> maps;
    maps.push_back("Bing");
    maps.push_back("Stamen-toner");
    maps.push_back("Stamen-toner-lite");
    maps.push_back("Stamen-toner-background");
    maps.push_back("Stamen-terrain");
    maps.push_back("Stamen-watercolor");
    maps.push_back("MapBox");
    maps.push_back("Mgcdn");
    maps.push_back("GeoIp");
    maps.push_back("Modest-BlueMarble");
    maps.push_back("BayArea");
    providers = gui->addRadio("MAPS", maps);
}

void ofxMapPro::guiEvent(ofxUIEventArgs &e){
    string name = e.widget->getName();
    
    if(name == "Zoom"){
        setZoom(zoom);
    } else if (name == "Latitud" || name == "Longitud"){
        setCenter(Location(lat, lon));
    } else {
        if(provider != NULL){
            if(providers->getActive() != NULL){
                string selected = providers->getActive()->getName();
                if (name == selected){
                    loadMap(name);
                }
            }
        }
    }
}