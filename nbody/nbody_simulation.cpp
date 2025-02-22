#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include <sstream>
#include <random>

const double G = 6.674e-11; // Gravitational constant
const double SOFTENING = 1e-9; // Softening factor to prevent singularities

struct Particle {
    double mass;
    double x, y, z;
    double vx, vy, vz;
    double fx, fy, fz;
};

// Function to compute forces
void compute_forces(std::vector<Particle>& particles) {
    size_t n = particles.size();
    for (size_t i = 0; i < n; i++) {
        particles[i].fx = particles[i].fy = particles[i].fz = 0.0;
        for (size_t j = 0; j < n; j++) {
            if (i != j) {
                double dx = particles[j].x - particles[i].x;
                double dy = particles[j].y - particles[i].y;
                double dz = particles[j].z - particles[i].z;
                double dist2 = dx*dx + dy*dy + dz*dz + SOFTENING;
                double inv_dist3 = G * particles[i].mass * particles[j].mass / (dist2 * sqrt(dist2));
                particles[i].fx += inv_dist3 * dx;
                particles[i].fy += inv_dist3 * dy;
                particles[i].fz += inv_dist3 * dz;
            }
        }
    }
}

// Function to integrate motion
void integrate(std::vector<Particle>& particles, double dt) {
    for (auto& p : particles) {
        double ax = p.fx / p.mass;
        double ay = p.fy / p.mass;
        double az = p.fz / p.mass;
        
        p.vx += ax * dt;
        p.vy += ay * dt;
        p.vz += az * dt;
        
        p.x += p.vx * dt;
        p.y += p.vy * dt;
        p.z += p.vz * dt;
    }
}

// Function to output state
void output_state(const std::vector<Particle>& particles, std::ofstream& file) {
    file << particles.size();
    for (const auto& p : particles) {
        file << "\t" << p.mass << "\t" << p.x << "\t" << p.y << "\t" << p.z
             << "\t" << p.vx << "\t" << p.vy << "\t" << p.vz
             << "\t" << p.fx << "\t" << p.fy << "\t" << p.fz;
    }
    file << "\n";
}

// Function to initialize particles randomly
void initialize_particles(std::vector<Particle>& particles, int num_particles) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dist(-1.0, 1.0);
    std::uniform_real_distribution<double> mass_dist(1.0, 10.0);
    
    for (int i = 0; i < num_particles; i++) {
        particles.push_back({mass_dist(gen), dist(gen), dist(gen), dist(gen),
                             dist(gen), dist(gen), dist(gen), 0.0, 0.0, 0.0});
    }
}

int main(int argc, char* argv[]) {
    if (argc != 5) {
        std::cerr << "Usage: " << argv[0] << " <num_particles> <dt> <iterations> <output_interval>\n";
        return 1;
    }

    int num_particles = std::stoi(argv[1]);
    double dt = std::stod(argv[2]);
    int iterations = std::stoi(argv[3]);
    int output_interval = std::stoi(argv[4]);

    std::vector<Particle> particles;
    initialize_particles(particles, num_particles);
    
    std::ofstream file("output.tsv");
    
    for (int step = 0; step < iterations; step++) {
        compute_forces(particles);
        integrate(particles, dt);
        if (step % output_interval == 0) {
            output_state(particles, file);
        }
    }
    
    file.close();
    return 0;
}

// Benchmarking function
void benchmark(int num_particles, double dt, int iterations) {
    std::vector<Particle> particles;
    initialize_particles(particles, num_particles);
    
    for (int step = 0; step < iterations; step++) {
        compute_forces(particles);
        integrate(particles, dt);
    }
    
    std::cout << "Benchmark completed: " << num_particles << " particles for " << iterations << " steps." << std::endl;
}

