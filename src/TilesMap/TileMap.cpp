//
//  TileMap.cpp
//
//  Created by Patricio Gonzalez Vivo on 7/2/14.
//
//

#include "TileMap.h"

#include "QueueSorter.h"

TileMap::TileMap(){
    width = 0;
    height = 0;
    
    centerCoordinate.row = 0.5;
    centerCoordinate.column = 0.5;
    centerCoordinate.zoom = 0;
}

TileMap::~TileMap(){
    //  TODO:
    //          - clean images and recent images
};

void TileMap::allocate(float _width, float _height){
    width = _width;
    height = _height;
    fbo.allocate(width,height);
}

void TileMap::loadMap(MapProviderRef _provider) {
	setMapProvider( _provider );
}

void TileMap::loadMap(string _predef){
    if ( _predef == "OpenStreet" ){
        setMapProvider( OpenStreetMapProvider::create() );
    } else if ( _predef == "Stamen-toner" ){
        setMapProvider( TemplatedMapProvider::create("http://spaceclaw.stamen.com/toner/{Z}/{X}/{Y}.png") );
    } else if ( _predef == "Stamen-toner-lite" ){
        setMapProvider( TemplatedMapProvider::create("http://spaceclaw.stamen.com/toner-lite/{Z}/{X}/{Y}.png") );
    } else if ( _predef == "Stamen-toner-background" ){
        setMapProvider( TemplatedMapProvider::create("http://spaceclaw.stamen.com/toner-background/{Z}/{X}/{Y}.png") );
    } else if ( _predef == "Stamen-terrain" ){
        setMapProvider( TemplatedMapProvider::create("http://spaceclaw.stamen.com/terrain/{Z}/{X}/{Y}.png") );
    } else if ( _predef == "Stamen-watercolor" ){
        setMapProvider( TemplatedMapProvider::create("http://spaceclaw.stamen.com/watercolor/{Z}/{X}/{Y}.png") );
    } else if ( _predef == "MapBox" ){
//        setMapProvider( TemplatedMapProvider::create("http://c.tiles.mapbox.com/v3/examples.map-szwdot65/{Z}/{X}/{Y}.png") );
//        setMapProvider( TemplatedMapProvider::create("http://c.tiles.mapbox.com/v3/examples.map-i87786ca/{Z}/{X}/{Y}.png") );
//        setMapProvider( TemplatedMapProvider::create("http://c.tiles.mapbox.com/v3/examples.map-i875mjb7/{Z}/{X}/{Y}.png") );
//        setMapProvider( TemplatedMapProvider::create("http://c.tiles.mapbox.com/v3/examples.map-8ced9urs/{Z}/{X}/{Y}.png") );
//        setMapProvider( TemplatedMapProvider::create("http://c.tiles.mapbox.com/v3/examples.map-y7l23tes/{Z}/{X}/{Y}.png") );
        setMapProvider( TemplatedMapProvider::create("http://c.tiles.mapbox.com/v3/examples.map-qfyrx5r8/{Z}/{X}/{Y}.png") );
    } else if ( _predef == "Mgcdn" ){
        setMapProvider( TemplatedMapProvider::create("http://otile1.mqcdn.com/tiles/1.0.0/osm/{Z}/{X}/{Y}.png") );
    } else if ( _predef == "GeoIp" ){
        setMapProvider( TemplatedMapProvider::create("http://acetate.geoiq.com/tiles/acetate/{Z}/{X}/{Y}.png") );
    } else if ( _predef == "Modest-BlueMarble" ){
        setMapProvider( TemplatedMapProvider::create("http://s3.amazonaws.com/com.modestmaps.bluemarble/{Z}-r{Y}-c{X}.jpg") );
    } else if ( _predef == "BayArea" ){
        setMapProvider( TemplatedMapProvider::create("http://osm-bayarea.s3.amazonaws.com/{Z}-r{Y}-c{X}.jpg") );
    } else if ( _predef == "Bing" ){
        setMapProvider( BingMapsProvider::create() );
    }
}

void TileMap::setUseTexture(bool bUseTex){
    // ...
}

ofTexture& TileMap::getTextureReference(){
    return fbo.getTextureReference();
}

void TileMap::update(){
    
        // if we're in between zoom levels, we need to choose the nearest:
        int baseZoom = ofClamp((int)round(centerCoordinate.zoom), provider->getMinZoom(), provider->getMaxZoom());
        
        // these are the top left and bottom right tile coordinates
        // we'll be loading everything in between:
        Coordinate tl = pointCoordinate(ofPoint(0,0)).zoomTo(baseZoom);
        Coordinate tr = pointCoordinate(ofPoint(width,0)).zoomTo(baseZoom);
        Coordinate bl = pointCoordinate(ofPoint(0,height)).zoomTo(baseZoom);
        Coordinate br = pointCoordinate(ofPoint(width,height)).zoomTo(baseZoom);
        
        // find start and end columns
        int minCol = floor(std::min(std::min(tl.column,tr.column),std::min(bl.column,br.column)));
        int maxCol = floor(std::max(std::max(tl.column,tr.column),std::max(bl.column,br.column)));
        int minRow = floor(std::min(std::min(tl.row,tr.row),std::min(bl.row,br.row)));
        int maxRow = floor(std::max(std::max(tl.row,tr.row),std::max(bl.row,br.row)));
        
        // pad a bit, for luck (well, because we might be zooming out between zoom levels)
        minCol -= GRID_PADDING;
        minRow -= GRID_PADDING;
        maxCol += GRID_PADDING;
        maxRow += GRID_PADDING;
        
        visibleKeys.clear();
        // grab coords for visible tiles
        //
        for (int col = minCol; col <= maxCol; col++) {
            for (int row = minRow; row <= maxRow; row++) {
                
                //  source coordinate wraps around the world:
                Coordinate coord = Coordinate(row,col,baseZoom);
                
                // keep this for later:
                visibleKeys.insert(coord);
                
                if (images.count(coord) == 0) {
                    
                    // fetch it if we don't have it
                    grabTile(coord);
                    
                    // see if we have  a parent coord for this tile?
                    bool gotParent = false;
                    for (int i = (int)coord.zoom; i > 0; i--) {
                        Coordinate zoomed = coord.zoomTo(i).container();
                        if (images.count(zoomed) > 0) {
                            visibleKeys.insert(zoomed);
                            gotParent = true;
                            break;
                        }
                    }
                    
                    // or if we have any of the children
                    if (!gotParent) {
                        Coordinate zoomed = coord.zoomBy(1).container();
                        vector<Coordinate> kids;
                        kids.push_back(zoomed);
                        kids.push_back(zoomed.right());
                        kids.push_back(zoomed.down());
                        kids.push_back(zoomed.right().down());
                        for (int i = 0; i < kids.size(); i++) {
                            if (images.count(kids[i]) > 0) {
                                visibleKeys.insert(kids[i]);
                            }
                        }
                    }
                    
                }
                
            } // rows
        } // columns
        
        //  Render
        //
        if (fbo.getWidth() != width || fbo.getHeight() != height ){
            fbo.allocate(width, height);
        }
        fbo.begin();
        ofPushMatrix();
        ofRotate(ofRadToDeg(rotation));
        
        ofClear(0,0);
        
        int numDrawnImages = 0;
        std::set<Coordinate>::iterator citer;
        for (citer = visibleKeys.begin(); citer != visibleKeys.end(); citer++) {
            Coordinate coord = *citer;
            
            float scale = pow(2.0, centerCoordinate.zoom - coord.zoom);
            ofPoint tileSize = provider->getTileSize() * scale;
            ofPoint center = ofPoint(width*0.5,height*0.5);
            Coordinate theCoord = centerCoordinate.zoomTo(coord.zoom);
            
            double tx = center.x + (coord.column - theCoord.column) * tileSize.x;
            double ty = center.y + (coord.row - theCoord.row) * tileSize.y;
            
            if (images.count(coord) > 0) {
                ofImage *tile = images[coord];
                // we want this image to be at the end of recentImages, if it's already there we'll remove it and then add it again
                //			recentImages.erase(remove(recentImages.begin(), recentImages.end(), tile), recentImages.end());
                std::vector<ofImage*>::iterator result = find(recentImages.begin(), recentImages.end(), tile);
                if (result != recentImages.end()) {
                    recentImages.erase(result);
                } else {
                    // if it's not in recent images it must be brand new?
                    tile->setUseTexture(true);
                    //					tile->initTex();
                    tile->update();
                }
                
                tile->draw( tx, ty, tileSize.x, tileSize.y );
                numDrawnImages++;
                recentImages.push_back(tile);
            }
        }
        
        ofPopMatrix();
        fbo.end();
        
        // stop fetching things we can't see:
        // (visibleKeys also has the parents and children, if needed, but that shouldn't matter)
        vector<Coordinate>::iterator iter = queue.begin();
        while (iter != queue.end()) {
            Coordinate key = *iter;
            if (visibleKeys.count(key) == 0){
                iter = queue.erase(iter);
            }
            else {
                ++iter;
            }
        }
        
        //  Process Queue ( load up to 4 more things )
        //
        if (queue.size() > MAX_PENDING-pending.size()) {
            sort(queue.begin(), queue.end(), QueueSorter(centerCoordinate));
        }
        
        while (pending.size() < MAX_PENDING && queue.size() > 0) {
            Coordinate coord = *(queue.begin());
            Coordinate key = Coordinate(coord);
            
            pending[key] = TileLoader();
            pending[key].start(key, provider, this);
            
            queue.erase(queue.begin());
        }
        
        // clear some images away if we have too many...
        int numToKeep = max(numDrawnImages,MAX_IMAGES_TO_KEEP);
        if (recentImages.size() > numToKeep) {
            
            // first clear the pointers from recentImages
            recentImages.erase(recentImages.begin(), recentImages.end()-numToKeep);
            //images.values().retainAll(recentImages);
            
            // TODO: re-think the stl collections used so that a simpler retainAll equivalent is available
            // now look in the images map and if the value is no longer in recent images then get rid of it
            //
            map<Coordinate,ofImage*>::iterator iter = images.begin();
            map<Coordinate,ofImage*>::iterator endIter = images.end();
            for (; iter != endIter;) {
                ofImage* tile = iter->second;
                vector<ofImage*>::iterator result = find(recentImages.begin(), recentImages.end(), tile);
                if (result == recentImages.end()) {
                    images.erase(iter++);
                    delete tile;
                }
                else {
                    ++iter;
                }
            }
        }
}

void TileMap::draw(float _x, float _y,float _width, float _height) {
    if(_width==-1||_height==-1){
        _width = width;
        _height = height;
    }
//    if (bEnable){
        ofPushStyle();
        fbo.draw(_x,_y,_width,_height);
        ofPopStyle();
//    }
}

void TileMap::setExtent( const MapExtent &extent, bool forceIntZoom ){
    Coordinate TL = provider->locationCoordinate( extent.getNorthWest() ).zoomTo( getZoom() );
    Coordinate BR = provider->locationCoordinate( extent.getSouthEast() ).zoomTo( getZoom() );
    
    const ofPoint tileSize = provider->getTileSize();
    
    // multiplication factor between horizontal span and map width
    const double hFactor = (BR.column - TL.column) / (width / tileSize.x);
    
    // multiplication factor expressed as base-2 logarithm, for zoom difference
    const double hZoomDiff = log2(hFactor);
    
    // possible horizontal zoom to fit geographical extent in map width
    const double hPossibleZoom = TL.zoom - (forceIntZoom ? ceil(hZoomDiff) : hZoomDiff);
    
    // multiplication factor between vertical span and map height
    const double vFactor = (BR.row - TL.row) / (height / tileSize.y);
    
    // multiplication factor expressed as base-2 logarithm, for zoom difference
    const double vZoomDiff = log2(vFactor);
    
    // possible vertical zoom to fit geographical extent in map height
    const double vPossibleZoom = TL.zoom - (forceIntZoom ? ceil(vZoomDiff) : vZoomDiff);
    
    // initial zoom to fit extent vertically and horizontally
    double initZoom = std::min(hPossibleZoom, vPossibleZoom);
    
    // additionally, make sure it's not outside the boundaries set by provider limits
    initZoom = std::min(initZoom, (double)provider->getMaxZoom());
    initZoom = std::max(initZoom, (double)provider->getMinZoom());
    
    // coordinate of extent center
    const double centerRow = (TL.row + BR.row) / 2.0;
    const double centerColumn = (TL.column + BR.column) / 2.0;
    const double centerZoom = (TL.zoom + BR.zoom) / 2.0;
    setCenter( Coordinate(centerRow, centerColumn, centerZoom).zoomTo(initZoom) );
}

void TileMap::setMapProvider( MapProviderRef _mapProvider ){
    images.clear();
    queue.clear();
    recentImages.clear();
    visibleKeys.clear();
    provider = _mapProvider;
}

void TileMap::setCenter(const Coordinate &center) {
    centerCoordinate = center;
}

void TileMap::setCenter(const Location &location) {
	setCenter( provider->locationCoordinate(location).zoomTo(getZoom()) );
}

/** sets scale according to given zoom level, should leave you with pixel perfect tiles */
void TileMap::setZoom(const double &_zoom) {
    centerCoordinate = centerCoordinate.zoomTo(_zoom);
}

MapExtent TileMap::getExtent() const{
    return MapExtent( pointLocation( ofPoint(0,0) ), pointLocation( ofPoint(width,height) ) );
}

/** @return zoom level of currently visible tile layer */
double TileMap::getZoom() const {
    return centerCoordinate.zoom;
}

Location TileMap::getCenter() const {
	return provider->coordinateLocation( centerCoordinate );
}

//----------------------------------------------------------------- CONVERSION
ofPoint TileMap::coordinatePoint(const Coordinate &target) const {
    
	/* Return an x, y point on the map image for a given coordinate. */
	Coordinate coord = target;
	
	if(coord.zoom != centerCoordinate.zoom) {
		coord = coord.zoomTo(centerCoordinate.zoom);
	}
    
	// distance from the center of the map
    const ofPoint tileSize = provider->getTileSize();
	ofPoint point = ofPoint(width,height) * 0.5;
	point.x += tileSize.x * (coord.column - centerCoordinate.column);
	point.y += tileSize.y * (coord.row - centerCoordinate.row);
    
	ofPoint rotated(point);
    rotated.rotate(rotation, ofPoint(0,0,1));
	
	return rotated;
}

Coordinate TileMap::pointCoordinate(const ofPoint &point) const {
	/* Return a coordinate on the map image for a given x, y point. */
	// new point coordinate reflecting distance from map center, in tile widths
	ofPoint rotated(point);
    const ofPoint tileSize = provider->getTileSize();
	rotated.rotate(-rotation, ofPoint(0,0,1));
	Coordinate coord(centerCoordinate);
	coord.column += (rotated.x - width * 0.5) / tileSize.x;
	coord.row += (rotated.y - height * 0.5) / tileSize.y;
	return coord;
}

ofPoint TileMap::locationPoint(const Location &location) const {
	return coordinatePoint( provider->locationCoordinate(location));
}

Location TileMap::pointLocation(const ofPoint &point) const {
	return provider->coordinateLocation(pointCoordinate(point));
}

void TileMap::panBy(const ofPoint &_pos) {
    const double sinr = sin(rotation);
    const double cosr = cos(rotation);
	const double dxr = _pos.x*cosr + _pos.y*sinr;
	const double dyr = _pos.y*cosr - _pos.x*sinr;
    const ofPoint tileSize = provider->getTileSize();
	centerCoordinate.column -= dxr / tileSize.x;
	centerCoordinate.row -= dyr / tileSize.y;
    Location centerLocation = getCenter();
    lat = centerLocation.lat;
    lon = centerLocation.lon;
}

void TileMap::scaleBy(const double &_scale, const ofPoint &_pos) {
    const double prevRotation = rotation;
	rotateBy(-prevRotation,_pos);
    ofPoint center = ofPoint(width,height) * 0.5;
	panBy(-_pos+center);
	centerCoordinate = centerCoordinate.zoomBy(log2(_scale));
	panBy(_pos-center);
	rotateBy(prevRotation,_pos);
    zoom = centerCoordinate.zoom;
}

void TileMap::rotateBy(const double &_radians, const ofPoint &_pos) {
	panBy(_pos*-1.0);
	rotation += _radians;
	panBy(_pos);
}

//------------------------------------------------------------------- MANAGING TILES

void TileMap::grabTile(Coordinate coord) {
	bool isPending = pending.count(coord) > 0;
	bool isQueued = find(queue.begin(), queue.end(), coord) != queue.end();
	bool isAlreadyLoaded = images.count(coord) > 0;
	if (!isPending && !isQueued && !isAlreadyLoaded) {
		queue.push_back(Coordinate(coord));
	}
}

// TODO: there could be issues when this is called from within a thread
// probably needs synchronizing on images / pending / queue
void TileMap::tileDone(Coordinate coord, ofImage *img) {
	// check if we're still waiting for this (new provider clears pending)
	// also check if we got something
	if (pending.count(coord) > 0 && img != NULL) {
		images[Coordinate(coord)] = img;
		pending.erase(coord);
	} else {
		pending.erase(coord);
		// try again?
	}
}