
#include "calibrationPoint.h"

std::ostream& operator<<(std::ostream& o, calibrationPoint const& c) { c.affichePoint(o); return o; }
std::ofstream& operator<<(std::ofstream& f, calibrationPoint const& c) { c.faffichePoint(f); return f; }

////////////////////////////////////////////////////////////////
// Affiche un point sur la sortie standard
////////////////////////////////////////////////////////////////
void calibrationPoint::affichePoint(std::ostream &o) const
{
	o << "(" << this->coord.X << " , " << this->coord.Y << " , " << this->ind_circle << ")";
}

////////////////////////////////////////////////////////////////
// Affiche un point dans un fichier
////////////////////////////////////////////////////////////////
void calibrationPoint::faffichePoint(std::ofstream& f) const
{
	f << "(" << this->coord.X << " , " << this->coord.Y << " , " << this->ind_circle << ")";
}

////////////////////////////////////////////////////////////////
// Retourne la distance entre deux points
////////////////////////////////////////////////////////////////
double calibrationPoint::distance(calibrationPoint p2) const
{
	return(sqrt( (this->coord.X-p2.coord.X)*(this->coord.X-p2.coord.X) + (this->coord.Y-p2.coord.Y)*(this->coord.Y-p2.coord.Y) ));
}
