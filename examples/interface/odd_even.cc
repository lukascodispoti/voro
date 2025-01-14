// Voro++ odd/even face coloring code
// By Chris H. Rycroft and the Rycroft Group

#include <vector>

#include "voro++.hh"
using namespace voro;

// This function returns a random floating point number uniformly distributed
// over a range
inline double rnd(double a,double b) {
    return a+(b-a)/RAND_MAX*static_cast<double>(rand());
}

int main() {
    unsigned int i;
    double x,y,z,rsq,r;
    voronoicell_3d v;

    // Initialize the Voronoi cell to be a cube of side length 2, centered on
    // the origin
    v.init(-1,1,-1,1,-1,1);

    // Cut the cell by 250 random planes which are all a distance 1 away from
    // the origin, to make an approximation to a sphere
    for(i=0;i<250;i++) {
        x=rnd(-1,1);
        y=rnd(-1,1);
        z=rnd(-1,1);
        rsq=x*x+y*y+z*z;
        if(rsq>1e-4&&rsq<1) {
            r=1/sqrt(rsq);x*=r;y*=r;z*=r;
            v.plane(x,y,z,1);
        }
    }

    // Calculate the orders of the faces and the normal vectors
    std::vector<int> f_vert;
    std::vector<double> nor;
    v.face_orders(f_vert);
    v.normals(nor);

    // Output POV-Ray planes with textures based on whether a face is
    // composed of an odd or even number of edges
    const char* parity[2]={"even","odd"};
    FILE *fp=safe_fopen("odd_even_pl.pov","w");
    for(i=0;i<f_vert.size();i++)
        fprintf(fp,"plane{<%g,%g,%g>,0.5 texture{t_%s}}\n"
                  ,nor[3*i],nor[3*i+1],nor[3*i+2]
                  ,parity[f_vert[i]&1]);
    fclose(fp);

    // Save the Voronoi cell as a spheres and cylinders
    v.draw_pov(0,0,0,"odd_even_v.pov");
}
