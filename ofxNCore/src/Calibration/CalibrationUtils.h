/*
 *  CalibrationUtils.h
 */

#ifndef CALIBRATIONUTILS_H
#define CALIBRATIONUTILS_H

//Chargement CONFIG.XML
#include "ofxXmlSettings.h"
//Utilisation de classe pour les "vecteurs" et les "rectangles"
#include "rect2d.h"
#include "vector2d.h"
//Utilisation de classe pour l'algorithme de delaunay, les points de calibration et les triangles
#include "table_delaunay.h"

class CalibrationUtils
{
	public:
		//! Constructeur
		CalibrationUtils();
		//! Destructeur
		~CalibrationUtils();

		//! Chargement des paramètres
		void loadXMLSettings();
		//! Sauvegarde des paramètres
		void saveCalibration();

		/**************************************** Méthode pour la calibration ****************************************/

		//! Initialisation de la grille des points de calibration (lecture et écriture)
		void setGrid(int x, int y);
		//! Modification de la taille de la zone d'affichage
		void setCamRes(int camWidth, int camHeight);
		//! Convertion des valeurs d'affichage théorique en valeur réelle de tout les points
		void computeCameraToScreenMap();
		//! Convertion des valeurs d'affichage d'un point théorique en valeur réelle
		void cameraToScreenPosition(float &x, float &y);
		//! Convertion des valeurs de lecture d'un point théorique en valeur réelle
		void cameraToScreenSpace(float &x, float &y);
		//! Modification de la taille de la zone de calibration
		void transformDimension(float &width, float &height);
		//! Initialisation des coordonnées des points de calibration à afficher
		void initScreenPoints();
		//! Initialisation des coordonnées des points de calibration à lire
		void initCameraPoints(int camWidth, int camHeight);
		//! Calcul les valeurs minimales et maximales des points de calibration à lire
		void calculateBox();

		//! Démarrage de la calibration
		virtual void beginCalibration();
		//! Passage à l'étape suivante
		virtual void nextCalibrationStep();
		//! Retour à l'étape précédente
		virtual void revertCalibrationStep();
		//! Statut de la calibration (en cours "true", terminée "false")
		bool bCalibrating;
		//! Autorisation du passage à l'étape suivante
		bool bGoToNextStep;
		//! Indice du point de calibration actif
		int  calibrationStep;

		//! Tableau des points de calibration affichés à l'écran (GRID_X * GRID_Y points)
		calibrationPoint* screenPoints;
		//! Accesseur vers le tableau de points de calibration affichés
		virtual calibrationPoint* getScreenPoints() { return screenPoints; };
		//! Tableau des points de calibration lus à l'écran (GRID_X * GRID_Y points)
		calibrationPoint* cameraPoints;
		//! Acceseur vers le tableau de points de calibration lus
		virtual calibrationPoint* getCameraPoints() { return cameraPoints; };
		//! Liste des triangles formé par les points de calibration
		std::vector<triangle*> *triangles;
		//! Nombre de points de calibration
		int  GRID_POINTS;
		//! Nombre de point de calibration sur chaque cercle de calibration (-1)
		int  GRID_X;
		//! Nombre de zone entre les cercles de calibration
		int  GRID_Y;

        //! Indique la présence ou non d'un point dans la zone de calibration
        bool inCalibrationSector(float &, float &);
		//! Recherche le triangle dans lequel se trouve le point (où les trois points les plus proches de celui-ci)
		triangle* findTriangleWithin(vector2df);
		//! initialisation du maillage de triangle (algorithme de delaunay)
		void initTriangles();

		/**************************************** Zone d'affichage ****************************************/
		//! Zone d'affichage
		rect2df screenBB;
		//! Initialisation de la zone d'affichage
		void setScreenBBox(rect2df & bbox);
		//! Accesseur vers la zone d'affichage
		rect2df getScreenBBox() { return screenBB; };
		//! Abscisse du point inférieur droit de la zone d'affichage
		float maxBoxX;
		//! Abscisse du point supérieur gauche de la zone d'affichage
		float minBoxX;
		//! Ordonnée du point inférieur droit de la zone d'affichage
		float maxBoxY;
		//! Ordonnée du point supérieur gauche de la zone d'affichage
		float minBoxY;
		//! Accesseur (écriture) vers l'échelle de la zone d'affichage
		void setScreenScale(float s);
		//! Accesseur vers l'échelle de la zone d'affichage
		float getScreenScale();

	private:
	    //!
		vector2df* cameraToScreenMap;
		//!
		int	_camWidth;
		//!
		int _camHeight;
		//!
		bool bscreenPoints;
		//!
		bool bcameraPoints;
		//!
		ofxXmlSettings		calibrationXML;
		//!
		string				xmlStructure;
		//!
		string				message;
};
#endif
