/*
 *  CalibrationUtils.cpp
 */

#include "CalibrationUtils.h"
#include <cmath>

//! Constructeur
CalibrationUtils::CalibrationUtils()
{
	//Valeur par défaut
	_camWidth = 320;
	_camHeight = 240;
}

//!Destructeur
CalibrationUtils::~CalibrationUtils(){

    delete screenPoints;
	delete cameraPoints;
    delete triangles;
	delete cameraToScreenMap;
}

//! Chargement des paramètres de calibration depuis le fichier "config.xml"
void CalibrationUtils::loadXMLSettings(){

	bGoToNextStep = false;

	// Can this load via http?
	if( calibrationXML.loadFile("calibration.xml")){
		//WOOT!
		message = "Calibration Loaded!";
	}else{
		//FAIL!
		message = "No calibration Found...";
		// GENERATE DEFAULT XML DATA WHICH WILL BE SAVED INTO THE CONFIG
	}

	bool bboxRoot = true;
	bool screenRoot = true;

	bCalibrating = false;
	calibrationStep = 0;

	//Set grid and init everything that relates to teh grid
	GRID_X		= calibrationXML.getValue("SCREEN:GRIDMESH:GRIDX", 50);
	GRID_Y		= calibrationXML.getValue("SCREEN:GRIDMESH:GRIDY", 50);

    //setGrid(GRID_X, GRID_Y);
    setGrid(GRID_X, GRID_Y);

	//Bounding Box Points
	if(bboxRoot){
	    vector2df ul(calibrationXML.getValue("SCREEN:BOUNDINGBOX:ulx", 0.000000),calibrationXML.getValue("SCREEN:BOUNDINGBOX:uly", 0.000000));
	    vector2df lr(calibrationXML.getValue("SCREEN:BOUNDINGBOX:lrx", 1.000000),calibrationXML.getValue("SCREEN:BOUNDINGBOX:lry", 1.000000));
		rect2df boundingbox(ul, lr);
		setScreenBBox(boundingbox);
	}else{
		setScreenScale(1.0f);
	}



	//Calibration Points
	if(screenRoot)
	{
		//lets see how many <STROKE> </STROKE> tags there are in the xml file
		int numDragTags = calibrationXML.getNumTags("SCREEN:POINT");

			//if there is at least one <POINT> tag we can read the list of points
			if(numDragTags > 0){

				//we push into the last POINT tag this temporarirly treats the tag as the document root.
				calibrationXML.pushTag("SCREEN:POINT", numDragTags-1);

				//we see how many points we have stored in <POINT> tags
				int numPtTags = calibrationXML.getNumTags("POINT");

			if(numPtTags > 0){

				//We then read those x y values into our array
				for(int i = 0; i < numPtTags; i++){

					//the last argument of getValue can be used to specify
					//which tag out of multiple tags you are refering to.
					int x = calibrationXML.getValue("POINT:X", 0.000000, i);
					int y = calibrationXML.getValue("POINT:Y", 0.000000, i);

					cameraPoints[i].coord = vector2df(x,y);
					printf("Calibration: %f, %f\n", cameraPoints[i].coord.X, cameraPoints[i].coord.Y);

					bscreenPoints = true;
					bcameraPoints = true;
				}
			}
			calibrationXML.popTag(); //Set XML root back to highest level
		}
	}
	//End calibrationXML Calibration Settings

	//Set the camera calibated box.
	calculateBox();
	computeCameraToScreenMap();
}


//! Sauvegarde des paramètres de calibration depuis le fichier "config.xml"
//! Sauvegarde lorsque l'on quitte le module de calibration
void CalibrationUtils::saveCalibration()
{
	// On comptre le nombre de balises <stroke></stroke> présentes dans le fichier XML
	int numDragTags = calibrationXML.getNumTags("SCREEN:POINT");

	//Si il y a au moins une balise <point>, on peut lire la liste des points
	if(numDragTags > 0)
	{
		calibrationXML.pushTag("SCREEN:POINT", numDragTags-1);
		calibrationXML.clear();

		//Sauvegarde du nombre de cercle et du nombre de points sur chaque cercle
		calibrationXML.setValue("GRIDMESH:GRIDX", GRID_X);
	    calibrationXML.setValue("GRIDMESH:GRIDY", GRID_Y);

		//Sauvegarde de la taille de la zone de calibration
		calibrationXML.setValue("BOUNDINGBOX:ulx", screenBB.upperLeftCorner.X);
		calibrationXML.setValue("BOUNDINGBOX:uly", screenBB.upperLeftCorner.Y);
		calibrationXML.setValue("BOUNDINGBOX:lrx", screenBB.lowerRightCorner.X);
		calibrationXML.setValue("BOUNDINGBOX:lry", screenBB.lowerRightCorner.Y);

		//Sauvegarde de l'ensemble des points de calibration
		if(GRID_POINTS > 0)
		{
			for(int i = 0; i < GRID_POINTS; i++)
			{
				calibrationXML.setValue("POINT:X", cameraPoints[i].coord.X, i);
				calibrationXML.setValue("POINT:Y", cameraPoints[i].coord.Y, i);
			}
		}
		calibrationXML.popTag();
	}
	calibrationXML.saveFile("calibration.xml");
}

//! Taille de la zone d'affichage
void CalibrationUtils::setScreenBBox(rect2df &box)
{
	screenBB = box;
	initScreenPoints();
}

//! Convertion des valeurs d'affichage théorique en valeur réelle de tout les points
void CalibrationUtils::computeCameraToScreenMap()
{
	cameraToScreenMap = new vector2df[_camWidth * _camHeight];

	int p = 0;
	for(int y = 0; y < _camHeight; y++)
	{
		for(int x = 0; x < _camWidth; x++)
		{
			//On cast les valeurs en réel
			float transformedX = (float)x;
			float transformedY = (float)y;

			//Convertion de la valeur théorique de tout les points, en valeur réelle
			cameraToScreenSpace(transformedX, transformedY);
			cameraToScreenMap[p] = vector2df(transformedX, transformedY);
			p++;
		}
	}
}

//! Modification de la résolution de l'image
void CalibrationUtils::setCamRes(int camWidth = 320, int camHeight = 240)
{
	_camWidth = camWidth;
	_camHeight = camHeight;
}

//! Accesseur (écriture) vers l'échelle de la zone d'affichage
void CalibrationUtils::setScreenScale(float s)
{
	float offset = (1.0f - s)*0.5f;
	screenBB = rect2df(vector2df(offset,offset),vector2df(1.0f-offset,1.0f-offset));
	initScreenPoints();
}

//! Accesseur vers l'échelle de la zone d'affichage
float CalibrationUtils::getScreenScale()
{
	float minValL = MIN(screenBB.lowerRightCorner.X,screenBB.lowerRightCorner.Y);
	minValL = 1.0f - minValL;
	float minValU = MAX(screenBB.upperLeftCorner.X,screenBB.upperLeftCorner.Y);
	float minVal = MIN(minValL,minValU);
	return 1.0f - (2.0f * minVal);
}

//! Calcul de la zone d'affichage
void CalibrationUtils::calculateBox()
{
	//Mise à zéro des variables
	maxBoxX = 0;
	minBoxX = _camWidth;
	maxBoxY = 0;
	minBoxY = _camHeight;

	//Calcul des valeurs minimales et maximales des points
	for(int i = 0; i < GRID_POINTS; i++)
	{
		if(cameraPoints[i].coord.X > maxBoxX)
		{
			maxBoxX = cameraPoints[i].coord.X;
		}
		else if(cameraPoints[i].coord.X < minBoxX)
		{
			minBoxX = cameraPoints[i].coord.X;
		}
		if(cameraPoints[i].coord.Y > maxBoxY)
		{
			maxBoxY = cameraPoints[i].coord.Y;
		}
		else if(cameraPoints[i].coord.Y < minBoxY)
		{
			minBoxY = cameraPoints[i].coord.Y;
		}
	}
}

//! Convertion des valeurs d'affichage théorique en valeur réelle de tout les points
void CalibrationUtils::cameraToScreenPosition(float &x, float &y)
{
	//Vérification que le point est dans la zone d'affichage
	if(y > _camHeight) y = _camHeight;
	if(y < 0) y = 0;
	if(x > _camWidth) x = _camWidth;
	if(x < 0) x = 0;

	int pos = (int)y * (int)_camWidth + (int)x;

	x = cameraToScreenMap[pos].X;
	y = cameraToScreenMap[pos].Y;

	return;
}

//! Démarrage de la calibration
void CalibrationUtils::beginCalibration()
{
	bCalibrating = true;
	calibrationStep = 0;
}

//! Passage à l'étape suivante de calibration
void CalibrationUtils::nextCalibrationStep()
{
	if(bCalibrating)
	{
		calibrationStep++;

		if(calibrationStep >= GRID_POINTS)
		{
			bCalibrating = false;
			calibrationStep = 0;
			saveCalibration();
			calculateBox();
			computeCameraToScreenMap();

            saveCalibration();
		}
	}
}

//! Retour à l'étape précédente de calibration
void CalibrationUtils::revertCalibrationStep()
{
	if(bCalibrating)
	{
		calibrationStep--;
		if(calibrationStep < 0)
		{
			calibrationStep = 0;
		}
	}
}

//! Modification de la taille de la zone de calibration
void CalibrationUtils::transformDimension(float &width, float &height)
{
	//Transformation de la hauteur et de la largeur
    float halfX = width * 0.5f;
    float halfY = height * 0.5f;

	//Extraction des coordonnées du centre de la zone de calibration
	float centerX = ((maxBoxX - minBoxX)/2) + minBoxX;
	float centerY = ((maxBoxY - minBoxY)/2) + minBoxY;

	//Calcul de l'abscisse et de l'ordonnée des points en inférieur droit et supérieur gauche
    float ulX = centerX - halfX;
    float ulY = centerY - halfY;
    float lrX = centerX + halfX;
    float lrY = centerY + halfY;

	//Convertion des coordonnées théoriques en coordonnées réelles
	cameraToScreenPosition(ulX, ulY);
	cameraToScreenPosition(lrX, lrY);

	//Calcul de la nouvelle hauteur et largeur de la zone de calibration
	width = std::fabs(lrX - ulX);
	height = std::fabs(ulY - lrY);
}




















/*************************************************************************************************************************************************************************/
/*************************************************************************************************************************************************************************/
/*************************************************************************************************************************************************************************/
/*************************************************************************************************************************************************************************/
/*************************************************************************************************************************************************************************/

//! Initialisation des coordonnées des points de calibration à afficher
void CalibrationUtils::initScreenPoints()
{
    int i,j, p = 0;
    //Angle entre deux axes (en radian)
    float delta_degree = 2*PI/(GRID_X+1);
    //Distance entre deux cercles
    float delta_rayon = min(screenBB.lowerRightCorner.X-screenBB.upperLeftCorner.X,screenBB.lowerRightCorner.Y-screenBB.upperLeftCorner.Y)/2/(GRID_Y+1);
    //Point au centre du cercle
    vector2df center = screenBB.upperLeftCorner + screenBB.lowerRightCorner/2;
	//Degre et rayon du point à afficher
    float degree=0, rayon=delta_rayon;
    int nb_points = GRID_X;
    float dist = (2*PI*delta_rayon)/GRID_X;
    float conv = ((float)ofGetScreenWidth())/ofGetScreenHeight();

	screenPoints[p] = calibrationPoint(center,0);
	p++;
	for(j=1; j <= GRID_Y+1; j++) //Pour chaque cercle
    {
        delta_degree = 2*PI/nb_points;
        if (j%2 == 0) degree = 0;
        else degree = delta_degree/2;
        for(i=0; i < nb_points; i++) //Pour chaque points sur le cercle
        {
			screenPoints[p] = calibrationPoint(center + vector2df(rayon*cos(degree)/conv,rayon*sin(degree)), j); //Note : /1.25
			p++;
			degree += delta_degree;
        }
        rayon += delta_rayon;
        nb_points += GRID_X; //(int)((2*PI*rayon)/dist)+1;
	}
}

//! Initialisation des points à lire
void CalibrationUtils::initCameraPoints(int camWidth, int camHeight)
{
	int i,j, p = 0;
    //Angle entre deux axes (en radian)
    float delta_degree = 2*PI/(GRID_X+1);
    //Distance entre deux cercles
    float delta_rayon = min(screenBB.lowerRightCorner.X-screenBB.upperLeftCorner.X,screenBB.lowerRightCorner.Y-screenBB.upperLeftCorner.Y)/2/(GRID_Y+1);
    //Point au centre du cercle
    vector2df center = screenBB.upperLeftCorner + screenBB.lowerRightCorner/2;
	//Degre et rayon du point à afficher
    float degree=0, rayon=delta_rayon;
    int nb_points = GRID_X;
    float dist = (2*PI*delta_rayon)/GRID_X;
    float conv = ((float)ofGetScreenWidth())/ofGetScreenHeight();

	cameraPoints[p] = calibrationPoint(center,0);
	p++;
	for(j=0; j<=GRID_Y; j++) //Pour chaque cercle
    {
        delta_degree = 2*PI/nb_points;
        if (j%2 == 0) degree = 0;
        else degree = delta_degree/2;
        for(i=0; i < nb_points; i++) //Pour chaque points sur le cercle
        {
			cameraPoints[p] = calibrationPoint(center + vector2df(rayon*cos(degree)/conv,rayon*sin(degree)), j+1); //Note : /1.25
			p++;
			degree += delta_degree;
        }
        rayon += GRID_X;
	}
}

//! Indique la présence ou non d'un point dans la zone de calibration
bool CalibrationUtils::inCalibrationSector(float &x, float &y)
{
    //Point valide
    bool valid = true;
    //Point au centre du cercle
    vector2df center = screenBB.upperLeftCorner + screenBB.lowerRightCorner/2;
    //Rayon de la zone de calibrage
    float rayon = min(screenBB.lowerRightCorner.X-screenBB.upperLeftCorner.X,screenBB.lowerRightCorner.Y-screenBB.upperLeftCorner.Y)/2;
    //Distance entre le point et le centre du cercle
    float dist = sqrt((x-center.X)*(x-center.X) + (y-center.Y)*(y-center.Y));
    //Si en dehors de la zone de calibration, on met la position à (-1;-1) pour le supprimer par la suite
	if (dist > rayon)
	{
	    x = -1;
        y = -1;
        valid = false;
	}
    return valid;
}

//! Création de la grille
void CalibrationUtils::setGrid(int x, int y)
{
	//Nombre de points de calibration sur chaque cercle de calibration - 1
	GRID_X = x;
	//Nombre de cercle de calibration - 1
	GRID_Y = y;
	//Nombre de points de calibration
    GRID_POINTS = GRID_X+1;
	for (int i=2; i<= GRID_Y+1; i++) GRID_POINTS += i*GRID_X;
    //printf("\nX : %d\nY : %d\n",x,y);
    //printf("GRID_POINTS : %d\n",GRID_POINTS);

	//Initialisation des listes de points, et des zones de calibration
	screenPoints = new calibrationPoint[GRID_POINTS];
	cameraPoints = new calibrationPoint[GRID_POINTS];
	triangles	 = new std::vector<triangle*>();

	//Si les deux listes de points sont crées
	if(bscreenPoints && bcameraPoints)
	{
		//Initialisation des points à afficher
		initScreenPoints();
		//Initialisation des points à lire
		initCameraPoints(_camWidth, _camHeight);
        //Initialisation des triangles
        initTriangles();
	}
}

//! Initialisation des zones de calibration
void CalibrationUtils::initTriangles(){

    int base = 1, node_dim, node_num, triangle_num, triangle_order;
    int *triangle_neighbor, *triangle_node;
    double *node_xy;

    //Copie de la liste de points dans un vecteur
    std::vector<calibrationPoint*> *list = new std::vector<calibrationPoint*>();
	for(int i=0; i < GRID_POINTS; i++) { list->push_back(&screenPoints[i]); }

    //Lecture de la liste de points
    r8mat_header_read (list, &node_dim, &node_num );
    node_xy = r8mat_data_read (list, node_dim, node_num);

    //Application de l'algorithme de Delaunay
    triangle_order = 3;
    triangle_node = new int[triangle_order*3*node_num];
    triangle_neighbor = new int[triangle_order*3*node_num];
    dtris2( node_num, base, node_xy, &triangle_num, triangle_node, triangle_neighbor );

    //On affecte les points formant les trianges aux triangles
    i4mat_tomat (triangle_order, triangle_num, triangle_node, list, triangles);

    delete [] node_xy;
    delete [] triangle_node;
    delete [] triangle_neighbor;
}

//! Convertion des valeurs de lecture d'un point théorique en valeur réelle
void CalibrationUtils::cameraToScreenSpace(float &x, float &y)
{
	vector2df pt = vector2df(x, y);
	triangle* t;
    float alpha, beta;

    //Si le point est dans la zone de calibration
	if (inCalibrationSector(x, y))
	{
        //Recherche du triangle contenant le point (ou les trois points les plus proche selon les cas)
	    t = findTriangleWithin(pt);
        //
        if ((t->s1->coord != pt) && (t->s2->coord != pt) && (t->s3->coord != pt))
        {
            //beta  = .....
            beta = ( ( t->s3->_y() - pt.Y ) * ( t->s1->_x() - pt.X ) + ( pt.X - t->s3->_x() ) * ( t->s1->_y() - pt.Y ) )
                   / ( ( t->s2->_x() - pt.X ) * ( t->s1->_y() - pt.Y) + ( pt.Y - t->s2->_y() ) * ( t->s1->_x() - pt.X ) );

            //alpha = .....
            alpha = ( beta * ( pt.X - t->s2->_x() ) + ( pt.X - t->s3->_x()) )
                    / ( t->s1->_x() - pt.X );

            //pt    = (alpha*A + beta*B + C)/(alpha+beta+1)
            x = ( alpha * t->s1->_x() + beta * t->s2->_x() + t->s3->_x() ) / ( alpha + beta + 1 );
            y = ( alpha * t->s1->_y() + beta * t->s2->_y() + t->s3->_y() ) / ( alpha + beta + 1 );
        }
	}
}

//! Recherche le triangle dans lequel se trouve le point (où les trois points les plus proches de celui-ci)
triangle* CalibrationUtils::findTriangleWithin(vector2df pt)
{
    //Recherche du point dans le maillage de triangle
	int nb_triangles = triangles->size();
	for (int t=0; t<nb_triangles; t++)
    {
        if (triangles->at(t)->isPointInTriangle(pt)) return triangles->at(t);
    }

    //Si le point n'es dans aucun triangle (cad sur les bords du cercle)
    //On recherche les trois points les plus près sur au moins deux cercles
	triangle *t = new triangle();
	float distance=0, value[3];
    t->s1 = new calibrationPoint(vector2df(0,0),-1); value[0] = 9999;
    t->s2 = new calibrationPoint(vector2df(0,0),-1); value[1] = 9999;
    t->s3 = new calibrationPoint(vector2df(0,0),-1); value[2] = 9999;
	for (int i=0; i<GRID_POINTS; i++)
    {
        distance = cameraPoints[i].coord.getDistanceFrom(pt);
        if (distance < value[0])
        {
            if ((t->s1->ind_circle != cameraPoints[i].ind_circle) || (t->s2->ind_circle != cameraPoints[i].ind_circle))
            {
                value[2] = value[1];
                t->s3=t->s2;
            }
            value[1] = value[0];
            value[0] = distance;
            t->s2 = t->s1;
            t->s1 = &cameraPoints[i];

        }
        else if (distance < value[1])
        {
            if ((t->s1->ind_circle != cameraPoints[i].ind_circle) || (t->s2->ind_circle != cameraPoints[i].ind_circle))
            {
                value[2] = value[1];
                t->s3 = t->s2;
            }
            t->s2  = &cameraPoints[i];
            value[1] = distance;
        }
        else if ((distance < value[2]) && ((cameraPoints[i].ind_circle != t->s1->ind_circle) || (cameraPoints[i].ind_circle != t->s2->ind_circle)))
        {
            t->s3 = &cameraPoints[i];
            value[2] = distance;
        }
    }
	return t;
}
