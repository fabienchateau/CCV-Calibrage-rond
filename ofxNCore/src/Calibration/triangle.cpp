
#include "triangle.h"

////////////////////////////////////////////////////////////////
// Cree et initialise un triangle avec les parametres passes dans la fonction
////////////////////////////////////////////////////////////////
triangle::triangle(calibrationPoint *s1,calibrationPoint *s2,calibrationPoint *s3)
{
	this->s1 = s1;
	this->s2 = s2;
	this->s3 = s3;
}

////////////////////////////////////////////////////////////////
// Retourne la présence ou l'absence d'un point dans le triangle
////////////////////////////////////////////////////////////////
bool triangle::isPointInTriangle(vector2df p)
{
	return (vector2df::isOnSameSide(p, this->s1->coord, this->s2->coord, this->s3->coord)
         && vector2df::isOnSameSide(p, this->s2->coord, this->s1->coord, this->s3->coord)
         && vector2df::isOnSameSide(p, this->s3->coord, this->s1->coord, this->s2->coord));
}

////////////////////////////////////////////////////////////////
// Affichage d'un triangle sur la sortie standard
////////////////////////////////////////////////////////////////
void triangle::afficheT(std::ostream &o) const
{
    o << *this->s1 << "," << *this->s2 << "," << *this->s3 << std::endl;
}

////////////////////////////////////////////////////////////////
// Affichage d'un triangle dans un fichier
////////////////////////////////////////////////////////////////
void triangle::fafficheT(std::ofstream& f) const
{
    f << *this->s1 << "," << *this->s2 << "," << *this->s3 << std::endl;
}


////////////////////////////////////////////////////////////////
// Affichage d'un triangle sur la sortie standard (Redéfinition <<)
////////////////////////////////////////////////////////////////
std::ostream& operator<<(std::ostream& o, triangle const& c) { c.afficheT(o); return o; }

////////////////////////////////////////////////////////////////
// Affichage d'un triangle dans un fichier (Redéfinition <<)
////////////////////////////////////////////////////////////////
std::ofstream& operator<<(std::ofstream& f, triangle const& c) { c.fafficheT(f); return f; }
