/*
*  Calibration.cpp
*
*
*  Created on 2/1/09.
*  Copyright 2009 NUI Group. All rights reserved.
*
*/

#include "Calibration.h"

/******************************************************************************
 * The setup function is run once to perform initializations in the application
 *****************************************************************************/
void Calibration::setup(int _camWidth, int _camHeight, BlobTracker *trackerIn)
{
	/********************
	 * Initalize Variables
	 *********************/
	calibrationParticle.loadImage("images/particle.png");
	calibrationParticle.setUseTexture(true);

    //Fonts - Is there a way to dynamically change font size?
	verdana.loadFont("verdana.ttf", 8, true, true);	   //Font used for small images
	calibrationText.loadFont("verdana.ttf", 10, true, true);

	//Load Calibration Settings from calibration.xml file
	calibrate.setCamRes(_camWidth, _camHeight);
	calibrate.loadXMLSettings();

	tracker = trackerIn;
	tracker->passInCalibration(&calibrate);

	printf("Calibration is setup!\n\n");
}

void Calibration::passInContourFinder(int numBlobs, vector<Blob> blobs) {

    contourFinder.nBlobs = numBlobs;
	contourFinder.blobs  = blobs;
}

void Calibration::passInTracker(BlobTracker *trackerIn) {
	tracker = trackerIn;
	tracker->passInCalibration(&calibrate);
}


/******************************
 *		  CALIBRATION
 *******************************/
void Calibration::doCalibration(){

	//Change the background color to black
	ofSetColor(0x000000);
	ofFill();
	ofRect(0, 0, ofGetWidth(), ofGetHeight());

    //Draw Calibration Screen
    drawCalibrationPointsAndBox();
    //Draw blobs in calibration to see where you are touching
	if(!calibrate.bCalibrating) drawCalibrationBlobs();

	/************************************
	 * Onscreen Calibration Instructions
	 ************************************/
	ofSetColor(0xFF00FF);
	//ofSetWindowTitle("Calibration");
	char reportStr[10240];
	calibrationText.setLineHeight(20.0f);

	if(calibrate.bCalibrating){
		sprintf(reportStr,
				"CALIBRATING: \n\n-To calibrate, touch and hold current circle target until the circle turns white \n-Press [b] to recapture background (if there's false blobs) \n-Press [r] to go back to previous point(s) \n");
		calibrationText.drawString(reportStr, ofGetWidth()/2 - calibrationText.stringWidth(reportStr)/2, ofGetHeight()/2 - calibrationText.stringHeight(reportStr)/2);
	}
	else
	{
		sprintf(reportStr,
				"CALIBRATION \n\n-Press [c] to start calibrating \n-Press [x] to return main screen \n-Press [b] to recapture background \n-Press [t] to toggle blob targets \n\nCHANGING GRID SIZE (number of points): \n\n-Current Grid Size is %i x %i \n-Press [+]/[-] to add/remove points on X axis \n-Press [shift][+]/[-] to add/remove points on Y axis \n\nALINGING BOUNDING BOX TO PROJECTION SCREEN: \n\n-Use arrow keys to move bounding box\n-Press and hold [w],[a],[s],[d] (top, left, bottom, right) and arrow keys to adjust each side\n", calibrate.GRID_X + 1, calibrate.GRID_Y + 1);
		calibrationText.drawString(reportStr, ofGetWidth()/2 - calibrationText.stringWidth(reportStr)/2, ofGetHeight()/2 - calibrationText.stringHeight(reportStr)/2);
	}
}

void Calibration::drawCalibrationPointsAndBox(){

    //this all has to do with getting the angle for loading circle
    arcAngle = 0;
	std::map<int, Blob> trackedBlobs;
	std::map<int, Blob>::iterator iter;
    trackedBlobs = tracker->getTrackedBlobs(); //get blobs from tracker
	for(iter=trackedBlobs.begin(); iter!=trackedBlobs.end(); iter++)
	{
        if (iter->second.sitting > arcAngle) {arcAngle = iter->second.sitting;}
    }
    //end loading circle angle

    //Récupération des "screen points"
	calibrationPoint* screenpts = calibrate.getScreenPoints();

	int i, j;
	//Pour chaque point
	for(i=0; i<calibrate.GRID_POINTS; i++)
	{
		//Si on est en phase de calibration, on dessine un cercle rouge autour du point actif
		if(calibrate.calibrationStep == i && calibrate.bCalibrating)
		{
			glPushMatrix();
			glTranslatef(screenpts[i].coord.X * ofGetWidth(), screenpts[i].coord.Y * ofGetHeight(), 0.0f);
			ofFill();
			//Dessin d'un cercle rouge autour du point actif
			ofSetColor(targetColor);
			ofCircle(0.f, 0.f, 40);
			//Dessin d'un cercle bleu (sur le cercle rouge), représentant l'acquisition de la donnée
			ofSetColor(0x00A4FF);
            DrawCircleLoader(0.0f, 0.0f, 40, 0, arcAngle * 360);
            //Dessin d'un cercle noir, à l'intérieur des deux autres, pour ne pas les remplir
            //Cela permet juste de jouer sur l'épaisseur des cercles
			ofSetColor(0x000000);
			ofCircle(0.f, 0.f, 25);
			glPopMatrix();
        }
        // Dessin des points (croix verte permettant la calibration
		ofSetColor(0x00FF00); //Activer la couleur verte
		ofRect(screenpts[i].coord.X * ofGetWidth() - 2, screenpts[i].coord.Y * ofGetHeight() - 10, 4, 20); //Tiret horizontal
        ofRect(screenpts[i].coord.X * ofGetWidth() - 10, screenpts[i].coord.Y * ofGetHeight() - 2, 20, 4); //Tiret vertical
	}

	//Affichage des triangles (delaunay)
	int nb_triangles = calibrate.triangles->size();
	for(i=0; i<nb_triangles; i++)
	{
	    //segment [pt1,pt2]
        ofLine(calibrate.triangles->at(i)->s1->_x() * ofGetWidth(), calibrate.triangles->at(i)->s1->_y() * ofGetHeight(),
               calibrate.triangles->at(i)->s2->_x() * ofGetWidth(), calibrate.triangles->at(i)->s2->_y() * ofGetHeight());
        //segment [pt1,pt3]
        ofLine(calibrate.triangles->at(i)->s1->_x() * ofGetWidth(), calibrate.triangles->at(i)->s1->_y() * ofGetHeight(),
               calibrate.triangles->at(i)->s3->_x() * ofGetWidth(), calibrate.triangles->at(i)->s3->_y() * ofGetHeight());
        //segment [pt3,pt2]
        ofLine(calibrate.triangles->at(i)->s3->_x() * ofGetWidth(), calibrate.triangles->at(i)->s3->_y() * ofGetHeight(),
               calibrate.triangles->at(i)->s2->_x() * ofGetWidth(), calibrate.triangles->at(i)->s2->_y() * ofGetHeight());
	}
	//Dessin du rectangle blanc représentant le contour de la zone à calibrer
	ofSetColor(0xFFFFFF); //Activer la couleur blanche
	ofNoFill(); //Ne pas remplir le rectangle

    vector2df center = calibrate.screenBB.upperLeftCorner + calibrate.screenBB.lowerRightCorner/2;
    float delta_rayon = min(calibrate.screenBB.getWidth() * ofGetWidth(),calibrate.screenBB.getHeight() * ofGetHeight()) / 2 / (calibrate.GRID_Y + 1);

    for(j=0; j<=calibrate.GRID_Y; j++) //Pour chaque cercle
    {
        ofCircle(center.X * ofGetWidth(),
                 center.Y * ofGetHeight(),
                 delta_rayon*(j+1));
    }
}

void Calibration::drawCalibrationBlobs(){

	//find blobs
	std::map<int, Blob> trackedBlobs;
	std::map<int, Blob>::iterator iter;
    trackedBlobs = tracker->getTrackedBlobs(); //get blobs from tracker
	for(iter=trackedBlobs.begin(); iter!=trackedBlobs.end(); iter++)
    {
        Blob drawBlob;
        drawBlob = iter->second;

        //transform height/width to calibrated space
        drawBlob.boundingRect.width *= calibrate.screenBB.getWidth() * ofGetWidth() * 4;
        drawBlob.boundingRect.height *= calibrate.screenBB.getHeight() * ofGetHeight() * 4 ;

        //Draw Fuzzy Circles
        ofEnableAlphaBlending();
        ofImage tempCalibrationParticle;
        tempCalibrationParticle.clone(calibrationParticle);
        ofSetColor(drawBlob.color);
        tempCalibrationParticle.draw(drawBlob.centroid.x * ofGetWidth() - drawBlob.boundingRect.width * .5,
									 drawBlob.centroid.y * ofGetHeight() - drawBlob.boundingRect.height * .5,
									 drawBlob.boundingRect.width,
									 drawBlob.boundingRect.height);
        ofDisableAlphaBlending();

        //Draw Blob Targets
        if(bShowTargets)
        {
            ofSetColor(0xFFFFFF);
            glLineWidth(5);
            glPushMatrix();
			//	glLoadIdentity();
            glTranslatef(drawBlob.centroid.x * ofGetWidth(), ((drawBlob.centroid.y * ofGetHeight())), 0);
			//  ofEllipse(0, 0, drawBlob.boundingRect.width/2, drawBlob.boundingRect.height/2);
            ofLine(0, -drawBlob.boundingRect.height/2, 0, drawBlob.boundingRect.height/2);
            ofLine(-drawBlob.boundingRect.width/2, 0, drawBlob.boundingRect.width/2, 0);
            glPopMatrix();
        }
        //set line width back to normal
        glLineWidth(1);
    }
}

/*****************************************************************************
 * TOUCH EVENTS
 *****************************************************************************/
void Calibration::RAWTouchUp( Blob b)
{
	if(calibrate.bCalibrating)//If Calibrating, register the calibration point on blobOff
	{
		if(calibrate.bGoToNextStep) {
			calibrate.nextCalibrationStep();
			calibrate.bGoToNextStep = false;
            targetColor = 0xFF0000;

			if(calibrate.calibrationStep != 0){
				printf("%d (%f, %f)\n", calibrate.calibrationStep, b.centroid.x, b.centroid.y);
			}
            else{
				printf("%d (%f, %f)\n", calibrate.GRID_POINTS, b.centroid.x, b.centroid.y);
				printf("Calibration complete\n\n");
			}
		}
	}
}

void Calibration::RAWTouchHeld( Blob b) {

	if(calibrate.bCalibrating)//If Calibrating, register the calibration point on blobOff
	{
		calibrate.cameraPoints[calibrate.calibrationStep] = calibrationPoint(vector2df(b.centroid.x, b.centroid.y),0);
		calibrate.bGoToNextStep = true;
		targetColor = 0xFFFFFF;
	}
}

void Calibration::RAWTouchMoved( Blob b)
{
}

void Calibration::RAWTouchDown( Blob b)
{
}

/*****************************************************************************
* UTILS
******************************************************************************/
void Calibration::DrawCircleLoader(double xctr, double yctr, double radius, double startAngle, double endAngle)
{
	double vectorX1,vectorY1;		// vector to a point on circle from its center
	double vectorX0,vectorY0;		// previous version of vectorX1,Y1;
	double angle,ang0,ang1;			// Angle in radians from circle start point.

	ang0 = startAngle * (3.14/180.0);	// convert degrees to radians
	ang1 = endAngle * (3.14/180.0);
	glBegin(GL_TRIANGLES);		// Tell OpenGL to draw a series of triangles
								// Start at the circle's rightmost point.
	vectorX1 = xctr + radius*cos(ang0);
	vectorY1 = yctr + radius*sin(ang0);
	for(angle=ang0 + 3.14/180.0;// step through all other points on circle;
		angle < ang1 + 3.14/180.0; angle += 3.14/180.0)
	{								// (add to ang1 to ensure arcs can close)
		vectorX0 = vectorX1;		// save previous point's position,
		vectorY0 = vectorY1;
		vectorX1= xctr + radius*cos(angle);	// find a new point on the circle,
		vectorY1= yctr + radius*sin(angle);
		glVertex2d(xctr,yctr);		// plot the points of a triangle (CCW order)
		glVertex2d(vectorX0,vectorY0);	// center-->old pt-->new pt.
		glVertex2d(vectorX1,vectorY1);
	}
	glEnd();						// finished drawing triangles.
	glFlush();						// Finish any pending drawing commands

	vectorY1=yctr;					// Set starting point
	vectorX1=xctr;
}

/*****************************************************************************
 * KEY EVENTS
 *****************************************************************************/
void Calibration::_keyPressed(ofKeyEventArgs &e) {

	if(calibrating)
	{
        switch(e.key)
        {
            case 't':
                bShowTargets ? bShowTargets = false : bShowTargets = true;
                break;
                /***********************
                 * Keys for Calibration
                 ***********************/
            case 'c': //Enter/Exit Calibration
                if(calibrate.bCalibrating) {
                    calibrate.bCalibrating = false;
                    printf("Calibration Stoped\n");
				} else {
					calibrate.beginCalibration();
                    printf("Calibration Started\n");
				}
                break;
            case 'r': //Revert Calibration
                if(calibrate.bCalibrating)calibrate.revertCalibrationStep();
                break;
            case 'a': //Left
                bA = true;
                break;
            case 'd': //Right
                bD = true;
                break;
            case 'w': //Right
                bW = true;
                break;
            case 's': //Right
                bS = true;
                break;
            case OF_KEY_RIGHT: //Move bounding box right
                if(bD){
                    calibrate.screenBB.lowerRightCorner.X += .001;
                    if(calibrate.screenBB.lowerRightCorner.X > 1) calibrate.screenBB.lowerRightCorner.X = 1;
                    calibrate.setGrid(calibrate.GRID_X, calibrate.GRID_Y);
                    calibrate.calibrationStep = 0;
                }else if(bA){
                    calibrate.screenBB.upperLeftCorner.X += .001;
                    if(calibrate.screenBB.upperLeftCorner.X > 1) calibrate.screenBB.upperLeftCorner.X = 1;
                    calibrate.setGrid(calibrate.GRID_X, calibrate.GRID_Y);
                    calibrate.calibrationStep = 0;
                }else{
                    calibrate.screenBB.lowerRightCorner.X += .001;
                    if(calibrate.screenBB.lowerRightCorner.X > 1) calibrate.screenBB.lowerRightCorner.X = 1;
                    calibrate.screenBB.upperLeftCorner.X += .001;
                    if(calibrate.screenBB.upperLeftCorner.X > 1) calibrate.screenBB.upperLeftCorner.X = 1;
                    calibrate.setGrid(calibrate.GRID_X, calibrate.GRID_Y);
                    calibrate.calibrationStep = 0;
                }
                break;
            case OF_KEY_LEFT: //Move bounding box left
                if(bD){
                    calibrate.screenBB.lowerRightCorner.X -= .001;
                    if(calibrate.screenBB.lowerRightCorner.X < 0) calibrate.screenBB.lowerRightCorner.X = 0;
                    calibrate.setGrid(calibrate.GRID_X, calibrate.GRID_Y);
                    calibrate.calibrationStep = 0;
                }else if(bA){
                    calibrate.screenBB.upperLeftCorner.X -= .001;
                    if(calibrate.screenBB.upperLeftCorner.X < 0) calibrate.screenBB.upperLeftCorner.X = 0;
                    calibrate.setGrid(calibrate.GRID_X, calibrate.GRID_Y);
                    calibrate.calibrationStep = 0;
                }else{
                    calibrate.screenBB.lowerRightCorner.X -= .001;
                    if(calibrate.screenBB.lowerRightCorner.X < 0) calibrate.screenBB.lowerRightCorner.X = 0;
                    calibrate.screenBB.upperLeftCorner.X -= .001;
                    if(calibrate.screenBB.upperLeftCorner.X < 0) calibrate.screenBB.upperLeftCorner.X = 0;
                    calibrate.setGrid(calibrate.GRID_X, calibrate.GRID_Y);
                    calibrate.calibrationStep = 0;
                }
                break;
            case OF_KEY_DOWN: //Move bounding box down
                if(bS){
                    calibrate.screenBB.lowerRightCorner.Y += .001;
                    if(calibrate.screenBB.lowerRightCorner.Y > 1) calibrate.screenBB.lowerRightCorner.Y = 1;
                    calibrate.setGrid(calibrate.GRID_X, calibrate.GRID_Y);
                    calibrate.calibrationStep = 0;
                }else if(bW){
                    calibrate.screenBB.upperLeftCorner.Y += .001;
                    if(calibrate.screenBB.upperLeftCorner.Y > 1) calibrate.screenBB.upperLeftCorner.Y = 1;
                    calibrate.setGrid(calibrate.GRID_X, calibrate.GRID_Y);
                    calibrate.calibrationStep = 0;
                }else{
                    calibrate.screenBB.lowerRightCorner.Y += .001;
                    if(calibrate.screenBB.lowerRightCorner.Y > 1) calibrate.screenBB.lowerRightCorner.Y = 1;
                    calibrate.screenBB.upperLeftCorner.Y += .001;
                    if(calibrate.screenBB.upperLeftCorner.Y > 1) calibrate.screenBB.upperLeftCorner.Y = 1;
                    calibrate.setGrid(calibrate.GRID_X, calibrate.GRID_Y);
                    calibrate.calibrationStep = 0;
                }
                break;
            case OF_KEY_UP: //Move bounding box up
                if(bS){
                    calibrate.screenBB.lowerRightCorner.Y -= .001;
                    if(calibrate.screenBB.lowerRightCorner.Y < 0) calibrate.screenBB.lowerRightCorner.Y = 0;
                    calibrate.setGrid(calibrate.GRID_X, calibrate.GRID_Y);
                    calibrate.calibrationStep = 0;
                }else if(bW){
                    calibrate.screenBB.upperLeftCorner.Y -= .001;
                    if(calibrate.screenBB.upperLeftCorner.Y < 0) calibrate.screenBB.upperLeftCorner.Y = 0;
                    calibrate.setGrid(calibrate.GRID_X, calibrate.GRID_Y);
                    calibrate.calibrationStep = 0;
                }else{
                    calibrate.screenBB.lowerRightCorner.Y -= .001;
                    if(calibrate.screenBB.lowerRightCorner.Y < 0) calibrate.screenBB.lowerRightCorner.Y = 0;
                    calibrate.screenBB.upperLeftCorner.Y -= .001;
                    if(calibrate.screenBB.upperLeftCorner.Y < 0) calibrate.screenBB.upperLeftCorner.Y = 0;
                    calibrate.setGrid(calibrate.GRID_X, calibrate.GRID_Y);
                    calibrate.calibrationStep = 0;
                }
                break;
                //Start Grid Point Changing
            case '+':
                calibrate.GRID_X ++;
                if(calibrate.GRID_X > 16) calibrate.GRID_X = 16; calibrate.setGrid(calibrate.GRID_X, calibrate.GRID_Y);
                calibrate.calibrationStep = 0;
                break;
            case '-':
                calibrate.GRID_X --;
                if(calibrate.GRID_X < 1) calibrate.GRID_X = 1; calibrate.setGrid(calibrate.GRID_X, calibrate.GRID_Y);
                calibrate.calibrationStep = 0;
                break;
            case '=':
                calibrate.GRID_Y ++;
                if(calibrate.GRID_Y > 16) calibrate.GRID_Y = 16; calibrate.setGrid(calibrate.GRID_X, calibrate.GRID_Y);
                calibrate.calibrationStep = 0;
                break;
            case '_':
                calibrate.GRID_Y --;
                if(calibrate.GRID_Y < 1) calibrate.GRID_Y = 1; calibrate.setGrid(calibrate.GRID_X, calibrate.GRID_Y);
                calibrate.calibrationStep = 0;
                break;
        }
    }
}

void Calibration::_keyReleased(ofKeyEventArgs &e){

	if(calibrating)
	{
		switch(e.key)
		{
			case 'w':
				bW = false;
				break;
			case 's':
				bS = false;
				break;
			case 'a':
				bA = false;
				break;
			case 'd':
				bD = false;
				break;
            case 'x': //Begin Calibrating
                tracker->passInCalibration(&calibrate);
                break;
			case OF_KEY_RIGHT: //Move bounding box right
                calibrate.computeCameraToScreenMap();
                tracker->passInCalibration(&calibrate);
                break;
            case OF_KEY_LEFT: //Move bounding box left
                calibrate.computeCameraToScreenMap();
                tracker->passInCalibration(&calibrate);
                break;
            case OF_KEY_DOWN: //Move bounding box down
                calibrate.computeCameraToScreenMap();
                tracker->passInCalibration(&calibrate);
                break;
            case OF_KEY_UP: //Move bounding box up
                calibrate.computeCameraToScreenMap();
                tracker->passInCalibration(&calibrate);
                break;
		}
	}
}
