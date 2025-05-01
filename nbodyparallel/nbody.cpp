#include <iostream>
#include <fstream>
#include <random>
#include <cmath>
#include <omp.h>  // openMP
#include <chrono> // timing

double G = 6.674*std::pow(10,-11);
//double G = 1;

struct simulation {
  size_t nbpart;
  
  std::vector<double> mass;

  //position
  std::vector<double> x;
  std::vector<double> y;
  std::vector<double> z;

  //velocity
  std::vector<double> vx;
  std::vector<double> vy;
  std::vector<double> vz;

  //force
  std::vector<double> fx;
  std::vector<double> fy;
  std::vector<double> fz;

  
  simulation(size_t nb)
    :nbpart(nb), mass(nb),
     x(nb), y(nb), z(nb),
     vx(nb), vy(nb), vz(nb),
     fx(nb), fy(nb), fz(nb) 
  {}
};


void random_init(simulation& s) {
  std::random_device rd;  
  std::mt19937 gen(rd());
  std::uniform_real_distribution dismass(0.9, 1.);
  std::normal_distribution dispos(0., 1.);
  std::normal_distribution disvel(0., 1.);

  for (size_t i = 0; i<s.nbpart; ++i) {
    s.mass[i] = dismass(gen);

    s.x[i] = dispos(gen);
    s.y[i] = dispos(gen);
    s.z[i] = dispos(gen);
    s.z[i] = 0.;
    
    s.vx[i] = disvel(gen);
    s.vy[i] = disvel(gen);
    s.vz[i] = disvel(gen);
    s.vz[i] = 0.;
    s.vx[i] = s.y[i]*1.5;
    s.vy[i] = -s.x[i]*1.5;
  }

  return;
  //normalize velocity (using normalization found on some physicis blog)
  double meanmass = 0;
  double meanmassvx = 0;
  double meanmassvy = 0;
  double meanmassvz = 0;
  for (size_t i = 0; i<s.nbpart; ++i) {
    meanmass += s.mass[i];
    meanmassvx += s.mass[i] * s.vx[i];
    meanmassvy += s.mass[i] * s.vy[i];
    meanmassvz += s.mass[i] * s.vz[i];
  }
  for (size_t i = 0; i<s.nbpart; ++i) {
    s.vx[i] -= meanmassvx/meanmass;
    s.vy[i] -= meanmassvy/meanmass;
    s.vz[i] -= meanmassvz/meanmass;
  }
  
}

void init_solar(simulation& s) {
  enum Planets {SUN, MERCURY, VENUS, EARTH, MARS, JUPITER, SATURN, URANUS, NEPTUNE, MOON};
  s = simulation(10);

  // Masses in kg
  s.mass[SUN] = 1.9891 * std::pow(10, 30);
  s.mass[MERCURY] = 3.285 * std::pow(10, 23);
  s.mass[VENUS] = 4.867 * std::pow(10, 24);
  s.mass[EARTH] = 5.972 * std::pow(10, 24);
  s.mass[MARS] = 6.39 * std::pow(10, 23);
  s.mass[JUPITER] = 1.898 * std::pow(10, 27);
  s.mass[SATURN] = 5.683 * std::pow(10, 26);
  s.mass[URANUS] = 8.681 * std::pow(10, 25);
  s.mass[NEPTUNE] = 1.024 * std::pow(10, 26);
  s.mass[MOON] = 7.342 * std::pow(10, 22);

  // Positions (in meters) and velocities (in m/s)
  double AU = 1.496 * std::pow(10, 11); // Astronomical Unit

  s.x = {0, 0.39*AU, 0.72*AU, 1.0*AU, 1.52*AU, 5.20*AU, 9.58*AU, 19.22*AU, 30.05*AU, 1.0*AU + 3.844*std::pow(10, 8)};
  s.y = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  s.z = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  s.vx = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  s.vy = {0, 47870, 35020, 29780, 24130, 13070, 9680, 6800, 5430, 29780 + 1022};
  s.vz = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
}

void reset_force(simulation& s) {
  for (size_t i=0; i<s.nbpart; ++i) {
    s.fx[i] = 0.;
    s.fy[i] = 0.;
    s.fz[i] = 0.;
  }
}

void apply_force(simulation& s, size_t i, double dt) {
  s.vx[i] += s.fx[i]/s.mass[i]*dt;
  s.vy[i] += s.fy[i]/s.mass[i]*dt;
  s.vz[i] += s.fz[i]/s.mass[i]*dt;
}

void update_position(simulation& s, size_t i, double dt) {
  s.x[i] += s.vx[i]*dt;
  s.y[i] += s.vy[i]*dt;
  s.z[i] += s.vz[i]*dt;
}

void dump_state(simulation& s) {
  static std::ofstream out("output.tsv");
  out<<s.nbpart<<'\t';
  for (size_t i=0; i<s.nbpart; ++i) {
    out<<s.mass[i]<<'\t';
    out<<s.x[i]<<'\t'<<s.y[i]<<'\t'<<s.z[i]<<'\t';
    out<<s.vx[i]<<'\t'<<s.vy[i]<<'\t'<<s.vz[i]<<'\t';
    out<<s.fx[i]<<'\t'<<s.fy[i]<<'\t'<<s.fz[i]<<'\t';
  }
  out<<'\n';
}

void load_from_file(simulation& s, std::string filename) {
  std::ifstream in (filename);
  size_t nbpart;
  in>>nbpart;
  s = simulation(nbpart);
  for (size_t i=0; i<s.nbpart; ++i) {
    in>>s.mass[i];
    in >>  s.x[i] >>  s.y[i] >>  s.z[i];
    in >> s.vx[i] >> s.vy[i] >> s.vz[i];
    in >> s.fx[i] >> s.fy[i] >> s.fz[i];
  }
  if (!in.good())
    throw "kaboom";
}

int main(int argc, char* argv[]) {
  if (argc != 5) {
    std::cerr
      <<"usage: "<<argv[0]<<" <input> <dt> <nbstep> <printevery>"<<"\n"
      <<"input can be:"<<"\n"
      <<"a number (random initialization)"<<"\n"
      <<"planet (initialize with solar system)"<<"\n"
      <<"a filename (load from file in singleline tsv)"<<"\n";
    return -1;
  }

  double dt = std::atof(argv[2]); //in seconds
  size_t nbstep = std::atol(argv[3]);
  size_t printevery = std::atol(argv[4]);

  simulation s(1);

  //parse command line
  {
    size_t nbpart = std::atol(argv[1]); //return 0 if not a number
    if ( nbpart > 0) {
      s = simulation(nbpart);
      random_init(s);
    } else {
      std::string inputparam = argv[1];
      if (inputparam == "planet") {
        init_solar(s);
      } else{
        load_from_file(s, inputparam);
      }
    }    
  }

  auto start = std::chrono::high_resolution_clock::now();

  for (size_t step = 0; step< nbstep; step++) {
    if (step %printevery == 0)
      dump_state(s);

    reset_force(s);

    #pragma omp parallel for schedule(dynamic)
    for (size_t i=0; i<s.nbpart; ++i) {
      double fx = 0.0, fy = 0.0, fz = 0.0;
      for (size_t j=0; j<s.nbpart; ++j) {
        if (i == j) continue;
        double dx = s.x[j] - s.x[i];
        double dy = s.y[j] - s.y[i];
        double dz = s.z[j] - s.z[i];
        double dist_sq = dx*dx + dy*dy + dz*dz + 0.1;
        double F = G * s.mass[i] * s.mass[j] / dist_sq;
        double norm = std::sqrt(dx*dx + dy*dy + dz*dz);
        fx += dx/norm * F;
        fy += dy/norm * F;
        fz += dz/norm * F;
      }
      s.fx[i] = fx;
      s.fy[i] = fy;
      s.fz[i] = fz;
    }

    for (size_t i=0; i<s.nbpart; ++i) {
      apply_force(s, i, dt);
      update_position(s, i, dt);
    }
  }

  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = end - start;

  std::ofstream log("timing.log", std::ios::app);
  log << "Threads=" << omp_get_max_threads()
      << ", Particles=" << s.nbpart
      << ", Steps=" << nbstep
      << ", Time=" << elapsed.count() << " seconds\n";
  std::cout << "Elapsed time: " << elapsed.count() << " seconds\n";

  return 0;
}
