
#ifndef __TRIANGLE__
#define __TRIANGLE__

#include "calibrationPoint.h"

//! Classe permettant de formé un triangle avec trois points de calibration
class triangle
{
    public :

    //! sommet A du triangle
    calibrationPoint *s1;
    //! sommet B du triangle
    calibrationPoint *s2;
    //! sommet C du triangle
    calibrationPoint *s3;

    //! Constructeur d'un triangle
    triangle() {};
    //! Destructeur d'un triangle
    ~triangle() {};

    //! Cree et initialise un triangle avec les parametres passes dans la fonction
    triangle(calibrationPoint *s1,calibrationPoint *s2,calibrationPoint *s3);

    //! Retourne la présence ou l'absence d'un point dans le triangle
    bool isPointInTriangle(vector2df p);

    //!Affichage d'un triangle sur le flux standard
    void afficheT(std::ostream &o=std::cout) const;

    //!Affichage d'un triangle dans un fichier
    void fafficheT(std::ofstream& f) const;
};

//! Redéfinition de l'opérateur << pour l'affichage d'un triangle sur le flux standard
std::ostream& operator<<(std::ostream& o, triangle const& c);
//! Redéfinition de l'opérateur << pour l'affichage d'un triangle dans un fichier
std::ofstream& operator<<(std::ofstream& f, triangle const& c);

#endif


