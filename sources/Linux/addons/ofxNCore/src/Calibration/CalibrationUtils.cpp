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
	bscreenPoints = false;
	bcameraPoints = false;
}

//!Destructeur
CalibrationUtils::~CalibrationUtils(){

    delete screenPoints;
	delete cameraPoints;
    delete trianglesC;
    delete trianglesS;
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

    cout << "GRID : " << GRID_X << " x " << GRID_Y << endl;
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

                if(numPtTags != screenPoints->size())
                {
                    cout << std::endl << "ERREUR FATALE LORS DE LA LECTURE DU FICHIER calibration.xml :" << std::endl;
                    cout << "Le nombre de points de calibration lus n'est pas bon." << std::endl;
                } else
                {
                    cameraPoints->clear();

                    //We then read those x y values into our array
                    for(int i = 0; i < numPtTags; i++)
                    {
                        //the last argument of getValue can be used to specify
                        //which tag out of multiple tags you are refering to.
                        int x = calibrationXML.getValue("POINT:X", 0.000000, i);
                        int y = calibrationXML.getValue("POINT:Y", 0.000000, i);

                        cameraPoints->push_back(calibrationPoint(vector2df(x,y),0));
                        //printf("Calibration: %f, %f\n", cameraPoints->at(i).coord.X, cameraPoints->at(i).coord.Y);
                    }
                }
            }
            calibrationXML.popTag(); //Set XML root back to highest level
        }
    }
	//End calibrationXML Calibration Settings

	bscreenPoints = true;
	bcameraPoints = true;

	//Set the camera calibated box.
    initTriangles();
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
		if(count() > 0)
		{
			for(int i = 0; i < cameraPoints->size(); i++)
			{
				calibrationXML.setValue("POINT:X", cameraPoints->at(i).coord.X, i);
				calibrationXML.setValue("POINT:Y", cameraPoints->at(i).coord.Y, i);
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
    /* debug
    cout << "NB triangles : " << trianglesC->size() << std::endl;
    for(int i = 0; i < trianglesC->size(); i++)
    {
        cout << "Triangles(1) : " << trianglesC->at(i)->s1->coord.X << ";" << trianglesC->at(i)->s1->coord.Y << std::endl;
        cout << "Triangles(2) : " << trianglesC->at(i)->s2->coord.X << ";" << trianglesC->at(i)->s2->coord.Y << std::endl;
        cout << "Triangles(3) : " << trianglesC->at(i)->s3->coord.X << ";" << trianglesC->at(i)->s3->coord.Y << std::endl;
    }
    */
    // Fichier de debug
    ofstream imageF("image_calibration.mat", ios::out);

	int p = 0;
	for(int y = 0; y < _camHeight; y++)
	{
		for(int x = 0; x < _camWidth; x++)
		{
		    type_point = 0; // non valide

			//On cast les valeurs en réel
			float transformedX = (float)x;
			float transformedY = (float)y;

			//Convertion de la valeur théorique de tout les points, en valeur réelle
			cameraToScreenSpace(transformedX, transformedY);

			if(transformedX < 0 || transformedY < 0) type_point = 0; // non valide

			cameraToScreenMap[p] = vector2df(transformedX, transformedY);

			//cout << "Point : (" << x << ";" << y << ")";
			//cout << " --> (" << transformedX << ";" << transformedY << ")" << endl;

			p++;

			// On ecrit le pixel
            //vector2df center = screenBB.upperLeftCorner + screenBB.lowerRightCorner/2;
            //float difx = (transformedX - center.X)*((float)ofGetScreenWidth());
            //float dify = (transformedY - center.Y)*((float)ofGetScreenHeight());
			//imageF <<  sqrt(difx*difx + dify*dify) << ",";
		}
        //imageF << endl;
	}

	imageF.close();
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
	for(int i = 0; i < cameraPoints->size(); i++)
	{
		if(cameraPoints->at(i).coord.X > maxBoxX)
		{
			maxBoxX = cameraPoints->at(i).coord.X;
		}
		else if(cameraPoints->at(i).coord.X < minBoxX)
		{
			minBoxX = cameraPoints->at(i).coord.X;
		}
		if(cameraPoints->at(i).coord.Y > maxBoxY)
		{
			maxBoxY = cameraPoints->at(i).coord.Y;
		}
		else if(cameraPoints->at(i).coord.Y < minBoxY)
		{
			minBoxY = cameraPoints->at(i).coord.Y;
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

		if(calibrationStep >= count())
		{
			bCalibrating = false;
			calibrationStep = 0;
			saveCalibration(); // VD : pourquoi maintenant si on le fait après ??
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
    int i,j;
    //Angle entre deux axes (en radian)
    float delta_degree = 2*PI/(GRID_X+1);
    //Distance entre deux cercles
    float delta_rayon = min(screenBB.lowerRightCorner.X-screenBB.upperLeftCorner.X,screenBB.lowerRightCorner.Y-screenBB.upperLeftCorner.Y)/2/(GRID_Y+1);
    //Point au centre du cercle
    vector2df center = screenBB.upperLeftCorner + screenBB.lowerRightCorner/2;
	//Degre et rayon du point à afficher
    float degree=0, rayon=delta_rayon;
    int nb_points = GRID_X;
    //float dist = (2*PI*delta_rayon)/GRID_X;
    float conv = ((float)ofGetScreenWidth())/ofGetScreenHeight();

    screenPoints->clear();
	screenPoints->push_back(calibrationPoint(center,0));
	for(j=1; j <= GRID_Y+1; j++) //Pour chaque cercle
    {
        delta_degree = 2*PI/nb_points;
        if (j%2 == 0) degree = 0;
        else degree = delta_degree/2;
        for(i=0; i < nb_points; i++) //Pour chaque points sur le cercle
        {
			screenPoints->push_back(calibrationPoint(center + vector2df(rayon*cos(degree)/conv,rayon*sin(degree)), j)); //Note : /1.25
			degree += delta_degree;
        }
        rayon += delta_rayon;
        nb_points += GRID_X; //(int)((2*PI*rayon)/dist)+1;
	}
}

//! Initialisation des points à lire
void CalibrationUtils::initCameraPoints(int camWidth, int camHeight)
{
	int i,j;
    //Angle entre deux axes (en radian)
    float delta_degree = 2*PI/(GRID_X+1);
    //Distance entre deux cercles
    float delta_rayon = min(screenBB.lowerRightCorner.X-screenBB.upperLeftCorner.X,screenBB.lowerRightCorner.Y-screenBB.upperLeftCorner.Y)/2/(GRID_Y+1);
    //Point au centre du cercle
    vector2df center = screenBB.upperLeftCorner + screenBB.lowerRightCorner/2;
	//Degre et rayon du point à afficher
    float degree=0, rayon=delta_rayon;
    int nb_points = GRID_X;
    //float dist = (2*PI*delta_rayon)/GRID_X;
    float conv = ((float)ofGetScreenWidth())/ofGetScreenHeight();

    cameraPoints->clear();
	cameraPoints->push_back(calibrationPoint(center,0));

	for(j=1; j <= GRID_Y+1; j++) //Pour chaque cercle
    {
        delta_degree = 2*PI/nb_points;
        if (j%2 == 0) degree = 0;
        else degree = delta_degree/2;
        for(i=0; i < nb_points; i++) //Pour chaque points sur le cercle
        {
			cameraPoints->push_back(calibrationPoint(center + vector2df(rayon*cos(degree)/conv,rayon*sin(degree)), j)); //Note : /1.25
			degree += delta_degree;
        }
        rayon += delta_rayon;
        nb_points += GRID_X; //(int)((2*PI*rayon)/dist)+1;
	}
}

//! Indique la présence ou non d'un point dans la zone de calibration
bool CalibrationUtils::inCalibrationSector(float &x, float &y)
{
	//Point valide
    bool valid = true;
    //Point au centre et à l'ectérieur du cercle
    calibrationPoint center = cameraPoints->front(), limit = cameraPoints->back();
    //Carré du rayon de la zone de calibrage
    float rayon2 = (center._x() - limit._x())*(center._x() - limit._x()) + (center._y() - limit._y())*(center._y() - limit._y());
    //Distance au carré entre le point et le centre du cercle
    float dist = (x - center._x())*(x - center._x()) + (y - center._y())*(y - center._y());
    //Si en dehors de la zone de calibration, on met la position à (-1;-1) pour le supprimer par la suite
	if (dist > rayon2)
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

	//Initialisation des listes de points, et des zones de calibration
	screenPoints = new std::vector<calibrationPoint>();
	cameraPoints = new std::vector<calibrationPoint>();
	trianglesC	 = new std::vector<triangle*>();
	trianglesS	 = new std::vector<triangle*>();

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

    //Lecture de la liste de points
    r8mat_header_read (screenPoints, &node_dim, &node_num );
    node_xy = r8mat_data_read (screenPoints, node_dim, node_num);

    //Application de l'algorithme de Delaunay
    triangle_order = 3;
    triangle_node = new int[triangle_order*3*node_num];
    triangle_neighbor = new int[triangle_order*3*node_num];

    dtris2( node_num, base, node_xy, &triangle_num, triangle_node, triangle_neighbor );

    //On affecte les points formant les trianges aux triangles
    i4mat_tomat (triangle_order, triangle_num, triangle_node, cameraPoints, trianglesC);
    i4mat_tomat (triangle_order, triangle_num, triangle_node, screenPoints, trianglesS);

    delete [] node_xy;
    delete [] triangle_node;
    delete [] triangle_neighbor;
}

//! Verifie si un point est sur une arete ou non. Met a jour pointA et pointB
bool CalibrationUtils::isOnEdge(float x, float y, float xa, float ya, int iA, float xb, float yb, int iB, int* pointA, int* pointB, float* K)
{
    bool isOnEdge = false;

    // Determination si le point est sur l'arrete AB
    if(xa == x)
    {
        if(xa == xb)
        {
            isOnEdge = true;
            *K = (y-ya)/(yb-ya);
            *pointA = iA;
            *pointB = iB;
        }
    }
    else if(ya == y)
    {
        if(ya == yb)
        {
            isOnEdge = true;
            *K = (x-xa)/(xb-xa);
            *pointA = iA;
            *pointB = iB;
        }
    }
    else
    {
        if(((xb-xa)/(x-xa)) == ((yb-ya)/(y-ya)))
        {
            isOnEdge = true;
            *K = (x-xa)/(xb-xa);
            *pointA = iA;
            *pointB = iB;
        }
        else
        {
            if(*pointA == -1) *pointA = iA; // On respecte la condition xa != x et ya != y => On prendra A = t.s1
        }
    }

    return isOnEdge;
}


//! Convertion des valeurs de lecture d'un point théorique en valeur réelle
void CalibrationUtils::cameraToScreenSpace(float &x, float &y)
{
	vector2df pt = vector2df(x, y);
	triangle t,ts;
    float alpha, beta, sigma;
    bool surUneArrete;
    int pointA,pointB,pointC;
    float K,xa,xb,ya,yb,xc,yc,xaS,xbS,xcS,yaS,ybS,ycS;

    //Si le point est dans la zone de calibration
	if (inCalibrationSector(x, y))
	{
        //Recherche du triangle contenant le point (ou les trois points les plus proche selon les cas)
	    findTriangleWithin(pt,&t,&ts);
        //
        // TROIS CAS POSSIBLES :
        //
        // - 1 : Le point pt est situe sur un point de calibration
        //       Dans ce cas, on affecte les coordonees ecran du point de calibration
        //
        // - 2 : Le point est sur une arrete du triangle => On utilise seulement deux points de calibration
        //
        // - 3 : Tous les autres cas. On prendra A = le point dont xa != x et ya != y (il y a au moins un des
        //       trois points du triangle qui respecte cette condition)
        //
        if ((t.s1->coord == pt) || (t.s2->coord == pt) || (t.s3->coord == pt)) // CAS 1
        {
            //std::cout << "---- type_point = 1" << std::endl;
            type_point = 1;

            if (t.s1->coord == pt)
            {
                x = ts.s1->_x();
                y = ts.s1->_y();
            }
            else if (t.s2->coord == pt)
            {
                x = ts.s2->_x();
                y = ts.s2->_y();
            }
            else
            {
                x = ts.s3->_x();
                y = ts.s3->_y();
            }
        }
        else
        {
            surUneArrete = false;
            pointA = -1;
            pointB = -1;
            pointC = -1;

            // Determination si le point est sur l'arrete AB
            surUneArrete = isOnEdge(pt.X, pt.Y, t.s1->_x(), t.s1->_y(), 1, t.s2->_x(), t.s2->_y(), 2, &pointA, &pointB,&K);

            // Determination si le point est sur l'arrete BC
            if(!surUneArrete) surUneArrete = isOnEdge(pt.X, pt.Y, t.s2->_x(), t.s2->_y(), 2, t.s3->_x(), t.s3->_y(), 3, &pointA, &pointB,&K);

            // Determination si le point est sur l'arrete CA
            if(!surUneArrete) surUneArrete = isOnEdge(pt.X, pt.Y, t.s3->_x(), t.s3->_y(), 3, t.s1->_x(), t.s1->_y(), 1, &pointA, &pointB,&K);


            if(pointA == -1)
            {
                std::cout << "---- ERREUR FATALE LORS DE LA CALIBRATION (pointA = -1)" << std::endl;
                std::cout << "DEBUG : input (" << pt.X << " ; "<< pt.Y << ")"<< std::endl;
                std::cout << "DEBUG : A (" << t.s1->_x() << " ; "<< t.s1->_y() << ")"<< std::endl;
                std::cout << "DEBUG : B (" << t.s2->_x() << " ; "<< t.s2->_y() << ")"<< std::endl;
                std::cout << "DEBUG : C (" << t.s3->_x() << " ; "<< t.s3->_y() << ")"<< std::endl;

                x = -1;
                y = -1;
            }

            if(t.s1->_x() == 0 && t.s1->_y() == 0)
            {
                std::cout << "---- ERREUR FATALE S1 = 0" << std::endl;
            }
            if(t.s2->_x() == 0 && t.s2->_y() == 0)
            {
                std::cout << "---- ERREUR FATALE S2 = 0" << std::endl;
            }
            if(t.s3->_x() == 0 && t.s3->_y() == 0)
            {
                std::cout << "---- ERREUR FATALE S3 = 0" << std::endl;
            }

            if(surUneArrete) // CAS 2
            {
                //std::cout << "SUR UNE ARRETE" << std::endl;
                //std::cout << "---- type_point = 2" << std::endl;
                type_point = 2;

                // Swap de points
                if(pointA == 1)
                {
                    xaS = ts.s1->_x();
                    yaS = ts.s1->_y();
                }
                else if(pointA == 2)
                {
                    xaS = ts.s2->_x();
                    yaS = ts.s2->_y();
                }
                else if(pointA == 3)
                {
                    xaS = ts.s3->_x();
                    yaS = ts.s3->_y();
                }
                if(pointB == 1)
                {
                    xbS = ts.s1->_x();
                    ybS = ts.s1->_y();
                }
                else if(pointB == 2)
                {
                    xbS = ts.s2->_x();
                    ybS = ts.s2->_y();
                }
                else if(pointB == 3)
                {
                    xbS = ts.s3->_x();
                    ybS = ts.s3->_y();
                }
                x = K*(xbS-xaS) + xaS;
                y = K*(ybS-yaS) + yaS;

            } else // CAS 3
            {

                // Swap de points
                if(pointA == 1)
                {
                    xa = t.s1->_x();
                    ya = t.s1->_y();
                    xb = t.s2->_x();
                    yb = t.s2->_y();
                    xc = t.s3->_x();
                    yc = t.s3->_y();
                    xaS = ts.s1->_x();
                    yaS = ts.s1->_y();
                    xbS = ts.s2->_x();
                    ybS = ts.s2->_y();
                    xcS = ts.s3->_x();
                    ycS = ts.s3->_y();
                }
                else if(pointA == 2)
                {
                    //std::cout << "SWAP NECESSAIRE" << std::endl;

                    xa = t.s2->_x();
                    ya = t.s2->_y();
                    xb = t.s1->_x();
                    yb = t.s1->_y();
                    xc = t.s3->_x();
                    yc = t.s3->_y();
                    xaS = ts.s2->_x();
                    yaS = ts.s2->_y();
                    xbS = ts.s1->_x();
                    ybS = ts.s1->_y();
                    xcS = ts.s3->_x();
                    ycS = ts.s3->_y();
                }
                else if(pointA == 3)
                {
                    //std::cout << "SWAP NECESSAIRE" << std::endl;

                    xa = t.s3->_x();
                    ya = t.s3->_y();
                    xb = t.s1->_x();
                    yb = t.s1->_y();
                    xc = t.s2->_x();
                    yc = t.s2->_y();
                    xaS = ts.s3->_x();
                    yaS = ts.s3->_y();
                    xbS = ts.s1->_x();
                    ybS = ts.s1->_y();
                    xcS = ts.s2->_x();
                    ycS = ts.s2->_y();
                }

                sigma = 1000;

                //beta  = .....
                beta = sigma * ( ( ( yc - pt.Y ) * ( xa - pt.X ) + ( pt.X - xc ) * ( ya - pt.Y ) ) / ( ( xb - pt.X ) * ( ya - pt.Y) + ( pt.Y - yb ) * ( xa - pt.X ) ));

                //alpha = .....
                alpha = ( beta * ( pt.X - xb ) + sigma * ( pt.X - xc )) / ( xa - pt.X );

                //pt    = (alpha*A + beta*B + C)/(alpha+beta+1000)
                x = ( alpha * xaS + beta * xbS + sigma * xcS ) / ( alpha + beta + sigma );
                y = ( alpha * yaS + beta * ybS + sigma * ycS ) / ( alpha + beta + sigma );

            }
            //std::cout << "---- POINT" << std::endl;
            //std::cout << "X = " << pt.X << std::endl;
            //std::cout << "x = " << x*ofGetScreenWidth() << std::endl;
            //std::cout << "Y = " << pt.Y << std::endl;
            //std::cout << "y = " << y*ofGetScreenHeight() << std::endl;
        }
	}
	else
    {
        x = -1;
        y = -1;
    }
}

//! Recherche le triangle dans lequel se trouve le point (où les trois points les plus proches de celui-ci)
void CalibrationUtils::findTriangleWithin(vector2df pt, triangle* t, triangle* ts)
{
    //Recherche du point dans le maillage de triangle
	int nb_triangles = trianglesC->size();
	for (int i=0; i<nb_triangles; i++)
    {
        if (trianglesC->at(i)->isPointInTriangle(pt))
        {
            //std::cout << "---- type_point = inTriangle" << std::endl;
            type_point = 5+i;
            *(t) = *(trianglesC->at(i));
            *(ts) = *(trianglesS->at(i));
            return;
        }
    }

    //std::cout << "---- type_point = outTriangle" << std::endl;
    type_point = 4;
    //Si le point n'es dans aucun triangle (cad sur les bords du cercle)
    //On recherche les trois points les plus près sur au moins deux cercles
	//triangle *t = new triangle(), *ts = new triangle();
	float distanceMax=FLT_MAX;
	float distance;
	int t1,t2;
    // On cherche le plus pres
	for (int i=(cameraPoints->size()-(GRID_X*(GRID_Y+1))); i<cameraPoints->size(); i++)
    {
        distance = ((cameraPoints->at(i).coord.X-pt.X)*(cameraPoints->at(i).coord.X-pt.X)+(cameraPoints->at(i).coord.Y-pt.Y)*(cameraPoints->at(i).coord.Y-pt.Y));
        if(distance < distanceMax)
        {
            distanceMax = distance;
            t->s1 = &cameraPoints->at(i);
            ts->s1 = &screenPoints->at(i);
            t1 = i;
        }
    }
    // On cherche le deuxieme plus pres
	distanceMax=FLT_MAX;
	for (int i=(cameraPoints->size()-(GRID_X*(GRID_Y+1))); i<cameraPoints->size(); i++)
    {
        distance = ((cameraPoints->at(i).coord.X-pt.X)*(cameraPoints->at(i).coord.X-pt.X)+(cameraPoints->at(i).coord.Y-pt.Y)*(cameraPoints->at(i).coord.Y-pt.Y));
        if((distance < distanceMax) && (i != t1))
        {
            distanceMax = distance;
            t->s2 = &cameraPoints->at(i);
            ts->s2 = &screenPoints->at(i);
            t2 = i;
        }
    }
    // On cherche le troisieme plus pres mais n'étant pas sur le dernier cercle
	distanceMax=FLT_MAX;
	for (int i=0; i<(cameraPoints->size()-(GRID_X*(GRID_Y+1))); i++)
    {
        distance = ((cameraPoints->at(i).coord.X-pt.X)*(cameraPoints->at(i).coord.X-pt.X)+(cameraPoints->at(i).coord.Y-pt.Y)*(cameraPoints->at(i).coord.Y-pt.Y));
        if((distance < distanceMax) && (i != t1) && (i != t2))
        {
            distanceMax = distance;
            t->s3 = &cameraPoints->at(i);
            ts->s3 = &screenPoints->at(i);
        }
    }
}
