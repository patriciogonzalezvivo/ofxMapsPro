#ifndef TILE_LOADER
#define TILE_LOADER

#include "ofMain.h"
#include "ofThread.h"
#include "Coordinate.h"
#include "SimpleWebScraper.h"

class ofMemoryImage : public ofImage {
public:
    
	void loadFromData(const unsigned char * datasource, int len){
        setUseTexture(false);
        ofBuffer buffer;
        buffer.set((const char *)datasource, len);
        loadImage(buffer);
    }
    
    void initTex(){
        setUseTexture(true);
        update();
    }
};

class ofxMapPro;
class AbstractMapProvider;

class TileLoader {
public:
    Coordinate coord;
	MapProviderRef provider;
	ofxMapPro *theMap;
	
	void start(Coordinate _coord, MapProviderRef _provider, ofxMapPro *_map);
	SimpleWebScraper sws;
		
	// callback events ----------------------------------------------------------
	void onTextReceived(const void* pSender, string& response);
	void onUnthreadedStreamReceived(const void* pSender, StreamEventArgs & args_);
	void onThreadedStreamReceived(const void* pSender, StreamEventArgs & args_);
		
};

	
#endif