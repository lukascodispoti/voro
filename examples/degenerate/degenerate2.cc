// Degenerate vertex example code
// By Chris H. Rycroft and the Rycroft Group

#include "voro++.hh"
using namespace voro;

const double pi=3.1415926535897932384626433832795;

// The total number of points to create as degenerate vertices
const int points=100;

// The number of planes that will be cut around each point
const int n=64;
const double step=2*pi/n;

// The angle (in radians) of the cutting planes from horizontal
const double theta=0.04;

// This function returns a random floating point number uniformly distributed
// over a range
inline double rnd(double a,double b) {
    return a+(b-a)/RAND_MAX*static_cast<double>(rand());
}

int main() {
    double x,y,z,rsq,r,phi;
    voronoicell_3d v;
    int n=0;

    // Initialize the Voronoi cell to be a cube of side length 2, centered on
    // the origin
    v.init(-1,1,-1,1,-1,1);

    // Plane cutting
    while(n<points) {

        // Choose a random point
        x=rnd(-1,1);
        y=rnd(-1,1);
        z=rnd(-1,1);

        // Skip it if it's outside the unit sphere or too close to the origin
        rsq=x*x+y*y+z*z;
        if(rsq<1e-4||rsq>1) continue;

        // Rescale the point so that it has modulus 1, and then apply
        // plane cuts around this point
        r=1/sqrt(rsq);x*=r;y*=r;z*=r;
        rsq=sqrt(x*x+y*y);r=z/rsq;
        for(phi=rnd(0,step);phi<2*pi;phi+=step)
            v.plane(x*cos(theta)+sin(theta)*(-y*cos(phi)/rsq-x*r*sin(phi)),
                y*cos(theta)+sin(theta)*(x*cos(phi)/rsq-y*r*sin(phi)),
                z*cos(theta)+sin(theta)*rsq*sin(phi),1);
        n++;
    }

    // Output the Voronoi cell to a file in Gnuplot format
    v.draw_gnuplot(0,0,0,"degenerate2.gnu");

    // Output the Voronoi cell in POV-Ray format
    v.draw_pov(0,0,0,"degenerate2_v.pov");
    v.draw_pov_mesh(0,0,0,"degenerate2_m.pov");
}
