#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cmath>
#include <random>
#include <chrono> // for benchmark

const double G = 6.67430e-11;   // gravitational constant
const double SOFTENING = 1e9;   // softening factor

struct Particle {
    double mass;
    double x, y, z;
    double vx, vy, vz;
    double fx, fy, fz;
};

struct Simulation {
    std::vector<Particle> particles;

    void initialize_random(int n) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<double> dist_pos(-1e11, 1e11);
        std::uniform_real_distribution<double> dist_vel(-1e3, 1e3);
        std::uniform_real_distribution<double> dist_mass(1e22, 1e30);

        particles.clear();
        for (int i = 0; i < n; ++i) {
            Particle p;
            p.mass = dist_mass(gen);
            p.x = dist_pos(gen);
            p.y = dist_pos(gen);
            p.z = dist_pos(gen);
            p.vx = dist_vel(gen);
            p.vy = dist_vel(gen);
            p.vz = dist_vel(gen);
            p.fx = p.fy = p.fz = 0.0;
            particles.push_back(p);
        }
    }

    bool initialize_from_file(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) return false;

        int count;
        file >> count;
        particles.clear();
        for (int i = 0; i < count; ++i) {
            Particle p;
            file >> p.mass >> p.x >> p.y >> p.z
                 >> p.vx >> p.vy >> p.vz
                 >> p.fx >> p.fy >> p.fz;
            particles.push_back(p);
        }
        return true;
    }

    void compute_forces() {
        for (auto& p : particles) p.fx = p.fy = p.fz = 0.0;
        for (size_t i = 0; i < particles.size(); ++i) {
            for (size_t j = 0; j < particles.size(); ++j) {
                if (i == j) continue;
                double dx = particles[j].x - particles[i].x;
                double dy = particles[j].y - particles[i].y;
                double dz = particles[j].z - particles[i].z;
                double dist2 = dx*dx + dy*dy + dz*dz + SOFTENING*SOFTENING; // FIXED 
                double inv_r3 = 1.0 / (dist2 * std::sqrt(dist2));
                double force = G * particles[i].mass * particles[j].mass * inv_r3;
                particles[i].fx += force * dx;
                particles[i].fy += force * dy;
                particles[i].fz += force * dz;
            }
        }
    }

    void integrate(double dt) {
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

    void write_state(std::ofstream& out) {
        out << particles.size();
        for (const auto& p : particles) {
            out << "\t" << p.mass << "\t" << p.x << "\t" << p.y << "\t" << p.z
                << "\t" << p.vx << "\t" << p.vy << "\t" << p.vz
                << "\t" << p.fx << "\t" << p.fy << "\t" << p.fz;
        }
        out << "\n";
    }
};

int main(int argc, char* argv[]) {
    if (argc != 6) {
        std::cerr << "Usage: " << argv[0] << " <num_particles|filename> <dt> <steps> <interval> <output.tsv>\n";
        return 1;
    }

    std::string init_arg = argv[1];
    double dt = std::stod(argv[2]);
    int steps = std::stoi(argv[3]);
    int interval = std::stoi(argv[4]);
    std::string output_file = argv[5];

    Simulation sim;
    if (isdigit(init_arg[0])) {
        int n = std::stoi(init_arg);
        sim.initialize_random(n);
    } else {
        if (!sim.initialize_from_file(init_arg)) {
            std::cerr << "Failed to open input file: " << init_arg << "\n";
            return 1;
        }
    }

    std::ofstream out(output_file);
    if (!out.is_open()) {
        std::cerr << "Failed to open output file: " << output_file << "\n";
        return 1;
    }

    auto start_time = std::chrono::high_resolution_clock::now(); // start timing

    for (int step = 0; step < steps; ++step) {
        sim.compute_forces();
        sim.integrate(dt);
        if (step % interval == 0)
            sim.write_state(out);
    }

    auto end_time = std::chrono::high_resolution_clock::now(); // end timing and print elapsed
    std::chrono::duration<double> elapsed = end_time - start_time;
    std::cout << "Simulation time: " << elapsed.count() << " seconds" << std::endl;

    out.close();
    return 0;
}
