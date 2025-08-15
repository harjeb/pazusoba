#include <pazusoba/core.h>

int main(int argc, char* argv[]) {
    auto solver = pazusoba::solver();
    solver.parse_args(argc, argv);

    // Force 3x3 square mode - override any other profiles
    pazusoba::profile profiles[1];
    profiles[0].name = pazusoba::shape_square;  // Force 3x3 square shape
    profiles[0].stop_threshold = 200;  // Allow more steps to find 3x3
    
    // Enable all orb types for 3x3 detection
    for (int i = 0; i < ORB_COUNT; i++) {
        profiles[0].orbs[i] = true;
    }
    
    solver.set_profiles(profiles, 1);

    pazusoba::Timer timer("force 3x3 search");
    auto state = solver.adventure();
    solver.print_state(state);
    return 0;
}