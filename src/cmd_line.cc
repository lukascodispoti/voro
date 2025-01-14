// Voro++, a cell-based Voronoi library
// By Chris H. Rycroft and the Rycroft Group

/** \file cmd_line.cc
 * \brief Source code for the command-line utility. */

#include <cstring>

#include "voro++.hh"
using namespace voro;

#ifdef _OPENMP
#include "omp.h"
#endif

enum blocks_mode {
    none,
    length_scale,
    specified
};

// A maximum allowed number of regions, to prevent too much memory being
// allocated
const int max_regions=1<<28;

// This message gets displayed if the user requests the help flag
void help_message() {
    puts("Voro++ version 1.0.0, by Chris H. Rycroft and the Rycroft Group\n\n"
         "Syntax: voro++ [options] <x_min> <x_max> <y_min>\n"
         "               <y_max> <z_min> <z_max> <input_file> [<output_file>]\n\n"
         "By default, the utility reads in the input file of particle IDs and positions,\n"
         "and computes the Voronoi cell for each.\n\n"
         "If not specified, the output is saved to \"<input_file>.vor\". Using '-' for any\n"
         "filename will read/write from standard input/output.\n\n"
         "Available options:\n"
         " -c <str>    : Specify a custom output string\n"
         " -g          : Turn on the Gnuplot output to <input_file.gnu>\n"
         " -G <gfile>  : Turn on the Gnuplot output to <gfile>\n"
         " -h/--help   : Print this information\n"
         " -hc         : Print information about custom output\n"
         " -l <len>    : Manually specify a length scale to configure the internal\n"
         "               computational grid\n"
         " -m <mem>    : Manually choose the memory allocation per grid block\n"
         "               (default 8)\n"
         " -n [3]      : Manually specify the internal grid size\n"
         " -o          : Ensure that the output file has the same order as the input\n"
         "               file\n"
         " -p          : Make container periodic in all three directions\n"
         " -px         : Make container periodic in the x direction\n"
         " -py         : Make container periodic in the y direction\n"
         " -pz         : Make container periodic in the z direction\n"
         " -r          : Assume the input file has an extra coordinate for radii\n"
#ifdef _OPENMP
         " -t <num>    : Use <num> threads for the computation [default: 1]\n"
#endif
         " -v          : Verbose output\n"
         " --version   : Print version information\n"
         " -wb [6]     : Add six plane wall objects to make rectangular box containing\n"
         "               the space x1<x<x2, x3<y<x4, x5<z<x6\n"
         " -wc [7]     : Add a cylinder wall object, centered on (x1,x2,x3),\n"
         "               pointing in (x4,x5,x6), radius x7\n"
         " -wo [7]     : Add a conical wall object, apex at (x1,x2,x3), axis\n"
         "               along (x4,x5,x6), angle x7 in radians\n"
         " -ws [4]     : Add a sphere wall object, centered on (x1,x2,x3),\n"
         "               with radius x4\n"
         " -wp [4]     : Add a plane wall object, with normal (x1,x2,x3),\n"
         "               and displacement x4\n"
         " -y          : Save POV-Ray particles to \"<input_file>_p.pov\" and POV-Ray\n"
         "               Voronoi cells to \"<input_file>_v.pov>\"\n"
         " -yp <pfile> : Save only POV-Ray particles to <pfile>\n"
         " -yv <vfile> : Save only POV-Ray Voronoi cells to <vfile>\n");
}

// This message gets displayed if the user requests information about doing
// custom output
void custom_output_message() {
    puts("The \"-c\" option allows a string to be specified that will customize the output\n"
         "file to contain a variety of statistics about each computed Voronoi cell. The\n"
         "string is similar to the standard C printf() function, made up of text with\n"
         "additional control sequences that begin with percentage signs that are expanded\n"
         "to different statistics. See http://math.lbl.gov/voro++/doc/custom.html for more\n"
         "information.\n"
         "\nParticle-related:\n"
         "  %i The particle ID number\n"
         "  %x The x coordinate of the particle\n"
         "  %y The y coordinate of the particle\n"
         "  %z The z coordinate of the particle\n"
         "  %q The position vector of the particle, short for \"%x %y %z\"\n"
         "  %r The radius of the particle (only printed if -p enabled)\n"
         "\nVertex-related:\n"
         "  %w The number of vertices in the Voronoi cell\n"
         "  %p A list of the vertices of the Voronoi cell in the format (x,y,z),\n"
         "     relative to the particle center\n"
         "  %P A list of the vertices of the Voronoi cell in the format (x,y,z),\n"
         "     relative to the global coordinate system\n"
         "  %o A list of the orders of each vertex\n"
         "  %m The maximum radius squared of a vertex position, relative to the\n"
         "     particle center\n"
         "\nEdge-related:\n"
         "  %g The number of edges of the Voronoi cell\n"
         "  %E The total edge distance\n"
         "  %e A list of perimeters of each face\n"
         "\nFace-related:\n"
         "  %s The number of faces of the Voronoi cell\n"
         "  %F The total surface area of the Voronoi cell\n"
         "  %A A frequency table of the number of edges for each face\n"
         "  %a A list of the number of edges for each face\n"
         "  %f A list of areas of each face\n"
         "  %t A list of bracketed sequences of vertices that make up each face\n"
         "  %l A list of normal vectors for each face\n"
         "  %n A list of neighboring particle or wall IDs corresponding to each face\n"
         "\nVolume-related:\n"
         "  %v The volume of the Voronoi cell\n"
         "  %c The centroid of the Voronoi cell, relative to the particle center\n"
         "  %C The centroid of the Voronoi cell, in the global coordinate system\n\n"
         "Using a blank custom string switches off all output. Each output string\n"
         "for floating point numbers can be modified to begin with \"%.prec\" to\n"
         "output the numbers to <prec> digits of precision (e.g. \"%.12x\").");
}

// Ths message is displayed if the user requests version information
void version_message() {
    puts("Voro++ version 1.0.0 (September 20th 2021)");
}

// Prints an error message. This is called when the program is unable to make
// sense of the command-line options.
void error_message() {
    fputs("voro++: Unrecognized command-line options; type \"voro++ -h\" for more\ninformation.\n",stderr);
}

// Checks to see if two strings are equal
inline bool se(const char* f1,const char* f2) {
    return strcmp(f1,f2)==0;
}

// Opens an extra output file
FILE* open_extra(char *buffer,char **argv,wall_list_3d &wl,int f_output,const char *ext,const char* base_fn,bool &stdout_used) {
    if(f_output>=0) {

        // If no specific filename was given then assemble the default one
        if(f_output==0) {
            sprintf(buffer,"%s%s",base_fn,ext);
            return safe_fopen(buffer,"w");
        }

        // If the "-" filename was given, then use standard output. Since only
        // one output type can use standard output, check to make sure that
        // standard output isn't already in use.
        if(se(argv[f_output],"-")) {
           if(stdout_used) {
               delete [] buffer;
               wl.deallocate();
               fputs("voro++: Standard output specified for more than one output file\n",stderr);
               exit(VOROPP_CMD_LINE_ERROR);
           }
           stdout_used=true;
           return stdout;
        }
    }
    return NULL;
}

// Closes a file if it's a valid file handle
inline void fclosec(FILE *fp) {
    if(fp!=NULL&&fp!=stdout) fclose(fp);
}

template<class v_class,class i_class>
inline void cell_output(v_class &c,i_class &cli,const int ps,double** conp,uint64_t **conid,const char* format,FILE* out_file,FILE* gnu_file,FILE* povp_file,FILE* povv_file) {
    int ijk=cli->ijk,q=cli->q,pid=conid[ijk][q];
    double *pp=conp[ijk]+ps*q,x=*pp,y=pp[1],z=pp[2],r=ps==4?pp[3]:0.5;
    if(out_file!=NULL) c.output_custom(format,pid,x,y,z,r,out_file);
    if(gnu_file!=NULL) c.draw_gnuplot(x,y,z,gnu_file);
    if(povp_file!=NULL) {
        fprintf(povp_file,"// id %d\n",pid);
        if(ps==4) fprintf(povp_file,"sphere{<%g,%g,%g>,%g}\n",x,y,z,r);
        else fprintf(povp_file,"sphere{<%g,%g,%g>,s}\n",x,y,z);
    }
    if(povv_file!=NULL) {
        fprintf(povv_file,"// cell %d\n",pid);
        c.draw_pov(x,y,z,povv_file);
    }
}

// Carries out the Voronoi computation and outputs the results to the requested
// files
template<class c_class,class v_class>
void cmd_line_output(c_class &con,v_class &c,const char* format,FILE* out_file,FILE* gnu_file,FILE* povp_file,FILE* povv_file,bool verbose,double &vol,int &vcc,int &tp) {
    container_base_3d::iterator cli;
    double **conp=con.p;uint64_t **conid=con.id;
    for(cli=con.begin();cli<con.end();cli++) if(con.compute_cell(c,cli)) {
        cell_output(c,cli,con.ps,conp,conid,format,out_file,gnu_file,povp_file,povv_file);
        if(verbose) {vol+=c.volume();vcc++;}
    }
    if(verbose) tp=con.total_particles();
}

// Carries out the Voronoi computation and outputs the results to the requested
// files, for the case when a particle order has been computed
template<class c_class,class v_class>
void cmd_line_output(particle_order vo,c_class &con,v_class &c,const char* format,FILE* out_file,FILE* gnu_file,FILE* povp_file,FILE* povv_file,bool verbose,double &vol,int &vcc,int &tp) {
    container_base_3d::iterator_order cli;
    double **conp=con.p;uint64_t **conid=con.id;
    for(cli=con.begin(vo);cli<con.end(vo);cli++) if(con.compute_cell(c,cli)) {
        cell_output(c,cli,con.ps,conp,conid,format,out_file,gnu_file,povp_file,povv_file);
        if(verbose) {vol+=c.volume();vcc++;}
    }
    if(verbose) tp=con.total_particles();
}

int main(int argc,char **argv) {
    int i=1,j=-7,custom_output=0,num_thread=1,nx,ny,nz,init_mem=8,
    gnuplot_output=-1,povv_output=-1,povp_output=-1;
    double ls=0;
    blocks_mode bm=none;
    bool polydisperse=false,x_prd=false,y_prd=false,z_prd=false,
         ordered=false,verbose=false,stdout_used=false;

    particle_list3 *plist3=NULL;particle_list4 *plist4=NULL;
    wall_list_3d wl;

    // If there's one argument, check to see if it's requesting help.
    // Otherwise, bail out with an error.
    if(argc==2) {
        if(se(argv[1],"-h")||se(argv[1],"--help")) {
            help_message();return 0;
        } else if(se(argv[1],"-hc")) {
            custom_output_message();return 0;
        } else if(se(argv[1],"--version")) {
            version_message();return 0;
        } else {
            error_message();
            return VOROPP_CMD_LINE_ERROR;
        }
    }

    // If there aren't enough command-line arguments, then bail out with an
    // error
    if(argc<7) {
           error_message();
           return VOROPP_CMD_LINE_ERROR;
    }

    // We have enough arguments. Now start searching for command-line options.
    while(i<argc-7) {
        if(se(argv[i],"-c")) {
            if(i>=argc-8) {error_message();wl.deallocate();return VOROPP_CMD_LINE_ERROR;}
            if(custom_output==0) {
                custom_output=++i;
            } else {
                fputs("voro++: multiple custom output strings detected\n",stderr);
                wl.deallocate();
                return VOROPP_CMD_LINE_ERROR;
            }
        } else if(se(argv[i],"-g")) {
            if(gnuplot_output==-1) gnuplot_output=0;
        } else if(se(argv[i],"-G")) {
            if(i>=argc-8) {error_message();wl.deallocate();return VOROPP_CMD_LINE_ERROR;}
            if(gnuplot_output>0) {
                fputs("voro++: multiple Gnuplot output filenames detected\n",stderr);
                wl.deallocate();
                return VOROPP_CMD_LINE_ERROR;
            } else gnuplot_output=++i;
        } else if(se(argv[i],"-h")||se(argv[i],"--help")) {
            help_message();wl.deallocate();return 0;
        } else if(se(argv[i],"-hc")) {
            custom_output_message();wl.deallocate();return 0;
        } else if(se(argv[i],"-l")) {
            if(i>=argc-8) {error_message();wl.deallocate();return VOROPP_CMD_LINE_ERROR;}
            if(bm!=none) {
                fputs("voro++: Conflicting options about grid setup (-l/-n)\n",stderr);
                wl.deallocate();
                return VOROPP_CMD_LINE_ERROR;
            }
            bm=length_scale;
            i++;ls=atof(argv[i]);
        } else if(se(argv[i],"-m")) init_mem=atoi(argv[++i]);
        else if(se(argv[i],"-n")) {
            if(i>=argc-10) {error_message();wl.deallocate();return VOROPP_CMD_LINE_ERROR;}
            if(bm!=none) {
                fputs("voro++: Conflicting options about grid setup (-l/-n)\n",stderr);
                wl.deallocate();
                return VOROPP_CMD_LINE_ERROR;
            }
            bm=specified;
            nx=atoi(argv[++i]);
            ny=atoi(argv[++i]);
            nz=atoi(argv[++i]);
            if(nx<=0||ny<=0||nz<=0) {
                fputs("voro++: Computational grid specified with -n must be greater than one\n"
                      "in each direction\n",stderr);
                wl.deallocate();
                return VOROPP_CMD_LINE_ERROR;
            }
        } else if(se(argv[i],"-o")) ordered=true;
        else if(se(argv[i],"-p")) x_prd=y_prd=z_prd=true;
        else if(se(argv[i],"-px")) x_prd=true;
        else if(se(argv[i],"-py")) y_prd=true;
        else if(se(argv[i],"-pz")) z_prd=true;
        else if(se(argv[i],"-r")) polydisperse=true;
#ifdef _OPENMP
        else if(se(argv[i],"-t")) {
            if(i>=argc-8) {error_message();wl.deallocate();return VOROPP_CMD_LINE_ERROR;}
            num_thread=atoi(argv[++i]);
            int max_thread=omp_get_max_threads();
            if(num_thread<1) {
                fputs("voro++: The number of threads must greater than zero\n",stderr);
                wl.deallocate();
                return VOROPP_CMD_LINE_ERROR;
            } else if(num_thread>max_thread) {
                fprintf(stderr,"voro++: The number of threads must be less than or equal to the maximum\n"
                        "available, which is %d on this system\n",max_thread);
                wl.deallocate();
                return VOROPP_CMD_LINE_ERROR;
            }
        }
#endif
        else if(se(argv[i],"-v")) verbose=true;
        else if(se(argv[i],"--version")) {
            version_message();
            wl.deallocate();
            return 0;
        } else if(se(argv[i],"-wb")) {
            if(i>=argc-13) {error_message();wl.deallocate();return VOROPP_CMD_LINE_ERROR;}
            double w0=atof(argv[++i]),w1=atof(argv[++i]),
                   w2=atof(argv[++i]),w3=atof(argv[++i]),
                   w4=atof(argv[++i]),w5=atof(argv[++i]);
            wl.add_wall(new wall_plane(-1,0,0,-w0,j--));
            wl.add_wall(new wall_plane(1,0,0,w1,j--));
            wl.add_wall(new wall_plane(0,-1,0,-w2,j--));
            wl.add_wall(new wall_plane(0,1,0,w3,j--));
            wl.add_wall(new wall_plane(0,0,-1,-w4,j--));
            wl.add_wall(new wall_plane(0,0,1,w5,j--));
        } else if(se(argv[i],"-ws")) {
            if(i>=argc-11) {error_message();wl.deallocate();return VOROPP_CMD_LINE_ERROR;}
            i++;
            double w0=atof(argv[++i]),w1=atof(argv[++i]),
                   w2=atof(argv[++i]),w3=atof(argv[++i]);
            wl.add_wall(new wall_sphere(w0,w1,w2,w3,j--));
        } else if(se(argv[i],"-wp")) {
            if(i>=argc-11) {error_message();wl.deallocate();return VOROPP_CMD_LINE_ERROR;}
            double w0=atof(argv[++i]),w1=atof(argv[++i]),
                   w2=atof(argv[++i]),w3=atof(argv[++i]);
            wl.add_wall(new wall_plane(w0,w1,w2,w3,j--));
        } else if(se(argv[i],"-wc")) {
            if(i>=argc-14) {error_message();wl.deallocate();return VOROPP_CMD_LINE_ERROR;}
            double w0=atof(argv[++i]),w1=atof(argv[++i]),
                   w2=atof(argv[++i]),w3=atof(argv[++i]),
                   w4=atof(argv[++i]),w5=atof(argv[++i]),
                   w6=atof(argv[++i]);
            wl.add_wall(new wall_cylinder(w0,w1,w2,w3,w4,w5,w6,j--));
        } else if(se(argv[i],"-wo")) {
            if(i>=argc-14) {error_message();wl.deallocate();return VOROPP_CMD_LINE_ERROR;}
            double w0=atof(argv[++i]),w1=atof(argv[++i]),
                   w2=atof(argv[++i]),w3=atof(argv[++i]),
                   w4=atof(argv[++i]),w5=atof(argv[++i]),
                   w6=atof(argv[++i]);
            wl.add_wall(new wall_cone(w0,w1,w2,w3,w4,w5,w6,j--));
        } else if(se(argv[i],"-y")) {
            if(povp_output==-1) povp_output=0;
            if(povv_output==-1) povv_output=0;
        } else if(se(argv[i],"-yp")) {
            if(i>=argc-8) {error_message();wl.deallocate();return VOROPP_CMD_LINE_ERROR;}
            if(povp_output>0) {
                fputs("voro++: multiple POV-Ray particle output filenames detected\n",stderr);
                wl.deallocate();
                return VOROPP_CMD_LINE_ERROR;
            } else povp_output=++i;
        } else if(se(argv[i],"-yv")) {
            if(i>=argc-8) {error_message();wl.deallocate();return VOROPP_CMD_LINE_ERROR;}
            if(povv_output>0) {
                fputs("voro++: multiple POV-Ray Voronoi cell output filenames detected\n",stderr);
                wl.deallocate();
                return VOROPP_CMD_LINE_ERROR;
            } else povv_output=++i;
        } else {
            if(i<argc-8) {
                wl.deallocate();
                error_message();
                return VOROPP_CMD_LINE_ERROR;
            }
            break;
        }
        i++;
    }

    // Check the memory guess is positive
    if(init_mem<=0) {
        fputs("voro++: The memory allocation must be positive\n",stderr);
        wl.deallocate();
        return VOROPP_CMD_LINE_ERROR;
    }

    // Read in the dimensions of the test box, and estimate the number of boxes
    // to divide the region up into
    double ax=atof(argv[i]),bx=atof(argv[i+1]),
           ay=atof(argv[i+2]),by=atof(argv[i+3]),
           az=atof(argv[i+4]),bz=atof(argv[i+5]),
           lx=bx-ax,ly=by-ay,lz=bz-az;

    // Check that for each coordinate, the minimum value is smaller than the
    // maximum value
    if(bx<ax) {
        fputs("voro++: Minimum x coordinate exceeds maximum x coordinate\n",stderr);
        wl.deallocate();
        return VOROPP_CMD_LINE_ERROR;
    }
    if(by<ay) {
        fputs("voro++: Minimum y coordinate exceeds maximum y coordinate\n",stderr);
        wl.deallocate();
        return VOROPP_CMD_LINE_ERROR;
    }
    if(bz<az) {
        fputs("voro++: Minimum z coordinate exceeds maximum z coordinate\n",stderr);
        wl.deallocate();
        return VOROPP_CMD_LINE_ERROR;
    }

    // Check that the output filename is a sensible length
    int flen=strlen(argv[i+6]);
    if(flen>4096) {
        fputs("voro++: Input filename too long\n",stderr);
        wl.deallocate();
        return VOROPP_CMD_LINE_ERROR;
    }

    // Open the main input file
    const char *base_fn,dflt_fname[]="stdin";
    FILE *in_file,*out_file;
    if(se(argv[i+6],"-")) {
        in_file=stdin;
        base_fn=dflt_fname;
    } else {
        in_file=safe_fopen(argv[i+6],"r");
        base_fn=argv[i+6];
    }

    // Calculate the number of blocks to use in the computation
    if(bm==none) {

        // If no information has been given, then read all the particles into
        // the a particle_list class in order to estimate the number of blocks
        if(polydisperse) {
            plist4=new particle_list4();
            plist4->import(in_file);
            plist4->guess_optimal(lx,ly,lz,nx,ny,nz);
        } else {
            plist3=new particle_list3();
            plist3->import(in_file);
            plist3->guess_optimal(lx,ly,lz,nx,ny,nz);
        }
        if(in_file!=stdin) fclose(in_file);
    } else {
        double nxf,nyf,nzf;
        if(bm==length_scale) {

            // Check that the length scale is positive and reasonably large
            if(ls*ls<tolerance*tolerance*(lx*lx+ly*ly+lz*lz)) {
                fputs("voro++: ",stderr);
                if(ls<0) {
                    fputs("The length scale must be positive\n",stderr);
                } else {
                    fputs("The ratio of the length scale to the container domain is too small\n",stderr);
                }
                wl.deallocate();
                return VOROPP_CMD_LINE_ERROR;
            }
            ls=0.6/ls;
            nxf=lx*ls+1;
            nyf=ly*ls+1;
            nzf=lz*ls+1;

            nx=int(nxf);ny=int(nyf);nz=int(nzf);
        } else {
            nxf=nx;nyf=ny;nzf=nz;
        }

        // Compute the number regions based on the length scale provided. If
        // the total number exceeds a cutoff then bail out, to prevent making a
        // massive memory allocation. Do this test using floating point
        // numbers, since huge integers could potentially wrap around to
        // negative values.
        if(nxf*nyf*nzf>max_regions) {
            fprintf(stderr,"voro++: Number of computational blocks exceeds the maximum allowed of %d.\n"
                           "Either increase the particle length scale, or recompile with an increased\n"
                           "maximum.\n",max_regions);
            wl.deallocate();
            return VOROPP_MEMORY_ERROR;
        }
    }

    // Open the main output file
    char *buffer=new char[flen+10];
    if(custom_output!=0&&se(argv[custom_output],"")) out_file=NULL;
    else {
        if(i+7<argc) {
            if(se(argv[i+7],"-")) {
                out_file=stdout;stdout_used=true;
            } else out_file=safe_fopen(argv[i+7],"w");
        } else {
            sprintf(buffer,"%s.vor",base_fn);
            out_file=safe_fopen(buffer,"w");
        }
    }

    // Open extra files for output
    FILE *gnu_file=open_extra(buffer,argv,wl,gnuplot_output,".gnu",base_fn,stdout_used),
         *povp_file=open_extra(buffer,argv,wl,povp_output,"_p.pov",base_fn,stdout_used),
         *povv_file=open_extra(buffer,argv,wl,povv_output,"_v.pov",base_fn,stdout_used);
    delete [] buffer;

    // Set up the output string
    const char *c_str=custom_output!=0?argv[custom_output]:(polydisperse?"%i %q %v %r":"%i %q %v");
    bool neigh=custom_output!=0&&voro_contains_neighbor(argv[custom_output]);

    // Now switch depending on whether polydispersity was enabled, and whether
    // output ordering is requested
    double vol=0;int tp=0,vcc=0;
    if(polydisperse) {
        container_poly_3d con(ax,bx,ay,by,az,bz,nx,ny,nz,x_prd,y_prd,z_prd,init_mem,num_thread);
        con.add_wall(wl);

        if(ordered) {
            particle_order vo;
            if(bm==none) {
                plist4->setup(vo,con);delete plist4;
            } else {
                con.import(vo,in_file);
                if(in_file!=stdin) fclose(in_file);
            }
            if(neigh) {
                voronoicell_neighbor_3d c(con);
                cmd_line_output(vo,con,c,c_str,out_file,gnu_file,povp_file,povv_file,verbose,vol,vcc,tp);
            } else {
                voronoicell_3d c(con);
                cmd_line_output(vo,con,c,c_str,out_file,gnu_file,povp_file,povv_file,verbose,vol,vcc,tp);
            }
        } else {
            if(bm==none) {
                plist4->setup(con);delete plist4;
            } else {
                con.import(in_file);
                if(in_file!=stdin) fclose(in_file);
            }
            if(neigh) {
                voronoicell_neighbor_3d c(con);
                cmd_line_output(con,c,c_str,out_file,gnu_file,povp_file,povv_file,verbose,vol,vcc,tp);
            } else {
                voronoicell_3d c(con);
                cmd_line_output(con,c,c_str,out_file,gnu_file,povp_file,povv_file,verbose,vol,vcc,tp);
            }
        }
    } else {
        container_3d con(ax,bx,ay,by,az,bz,nx,ny,nz,x_prd,y_prd,z_prd,init_mem,num_thread);
        con.add_wall(wl);

        if(ordered) {
            particle_order vo;
            if(bm==none) {
                plist3->setup(vo,con);delete plist3;
            } else {
                con.import(vo,in_file);
                if(in_file!=stdin) fclose(in_file);
            }
            if(neigh) {
                voronoicell_neighbor_3d c(con);
                cmd_line_output(vo,con,c,c_str,out_file,gnu_file,povp_file,povv_file,verbose,vol,vcc,tp);
            } else {
                voronoicell_3d c(con);
                cmd_line_output(vo,con,c,c_str,out_file,gnu_file,povp_file,povv_file,verbose,vol,vcc,tp);
            }
        } else {
            if(bm==none) {
                plist3->setup(con);delete plist3;
            } else {
                con.import(in_file);
                if(in_file!=stdin) fclose(in_file);
            }
            if(neigh) {
                voronoicell_neighbor_3d c(con);
                cmd_line_output(con,c,c_str,out_file,gnu_file,povp_file,povv_file,verbose,vol,vcc,tp);
            } else {
                voronoicell_3d c(con);
                cmd_line_output(con,c,c_str,out_file,gnu_file,povp_file,povv_file,verbose,vol,vcc,tp);
            }
        }
    }
    wl.deallocate();

    // Print information if verbose output requested
    if(verbose) {
        printf("Container geometry        : [%g:%g] [%g:%g] [%g:%g]\n"
               "Computational grid size   : %d by %d by %d (%s)\n"
               "Filename                  : %s\n"
               "Output string             : %s%s\n"
               "Total imported particles  : %d (%.2g per grid block)\n"
               "Total V. cells computed   : %d\n"
               "Total container volume    : %g\n"
               "Total V. cell volume      : %g\n",
               ax,bx,ay,by,az,bz,nx,ny,nz,
               bm==none?"estimated from file":(bm==length_scale?
               "estimated using length scale":"directly specified"),
               argv[i+6],c_str,custom_output==0?" (default)":"",
               tp,static_cast<double>(tp)/(nx*ny*nz),vcc,
               lx*ly*lz,vol);
    }

    // Close output files
    fclosec(out_file);
    fclosec(gnu_file);
    fclosec(povp_file);
    fclosec(povv_file);
    return 0;
}
