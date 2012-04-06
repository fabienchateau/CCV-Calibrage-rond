#ifndef __CALIBRATION_POINT__
#define __CALIBRATION_POINT__

#include <iostream>
#include <fstream>
#include "rect2d.h"

class calibrationPoint
{
	public:

    //! Coordonnées du point
    vector2df coord;

    //! Indice du cercle (entier positif ou nul)
    int ind_circle;

    //! Constructeurs d'un point de calibration sans paramètres
    calibrationPoint(): coord(vector2df(0,0)), ind_circle(0) {};

    //! Constructeurs d'un point de calibration à partir des coordonnées et d'un indice
    calibrationPoint(vector2df v, int ind=0): coord(v), ind_circle(ind) {};

    //! Constructeurs d'un point de calibration à partir d'une abscisse, d'une ordonnée et d'un indice (entier)
    calibrationPoint(int x, int y, int ind=0): coord(vector2df(x,y)), ind_circle(ind) {};

    //! Constructeurs d'un point de calibration à partir d'une abscisse, d'une ordonnée et d'un indice (réel)
    calibrationPoint(double x, double y, int ind=0): coord(vector2df((int)x,(int)y)), ind_circle(ind) {};

    //! Destructeur d'un point de calibration
    ~calibrationPoint() {};

    //! Accesseur vers l'abscisse du point
    float _x() { return this->coord.X; }

    //! Accesseur vers l'ordonnée du point
    float _y() { return this->coord.Y; }

    //! Affiche un point sur la sortie standard
    void affichePoint(std::ostream& o=std::cout) const;

    ///! Affiche un point dans un fichier
    void faffichePoint(std::ofstream& f) const;

    //! Retourne la distance entre deux points
    double distance(calibrationPoint p2) const;
};

//! Redéfinition de l'opérateur << pour l'affichage d'un point sur le flux standard
std::ostream& operator<<(std::ostream& o, calibrationPoint const& c);

//! Redéfinition de l'opérateur << pour l'affichage d'un point dans un fichier
std::ofstream& operator<<(std::ofstream& f, calibrationPoint const& c);

#endif
