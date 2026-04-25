///from chatGPT


#include <stdio.h>
#include <math.h>

#define PI 3.14159265

// Function to calculate the new position of the car based on current position, speed, angle, and time elapsed
void moveCar(float *x, float *y, float speed, float angle, float time) {
    float newX, newY;

    // Convert angle from degrees to radians
    angle = angle * PI / 180.0;

    // Calculate the new position using trigonometry
    newX = *x + speed * cos(angle) * time;
    newY = *y + speed * sin(angle) * time;

    // Update the position
    *x = newX;
    *y = newY;
}

int main() {
    float carX = 0.0, carY = 0.0; // Initial position of the car
    float carSpeed = 0.0; // Initial speed of the car
    float carAngle = 0.0; // Initial angle of the car (in degrees)
    float deltaTime = 0.1; // Time interval between updates

    // Main game loop
    while (1) {
        // Check for user input to control the car's speed and angle (e.g., arrow keys)
        // For simplicity, let's assume carSpeed and carAngle are updated based on user input

        // Simulate drifting physics by adjusting the car's angle based on speed
        if (carSpeed > 0) {
            // Adjust the angle of the car when it's moving
            if (carSpeed > 5) {
                carAngle += 2; // Adjust the angle more aggressively when the speed is high
            } else {
                carAngle += 1; // Adjust the angle more gently when the speed is low
            }
        }

        // Move the car based on current speed, angle, and time interval
        moveCar(&carX, &carY, carSpeed, carAngle, deltaTime);

        // Display the new position of the car
        printf("Car position: (%.2f, %.2f)\n", carX, carY);

        // Introduce some delay to simulate frame rate
        // In a real game, this would be replaced by a function to control frame rate
        // For simplicity, we use a sleep function here
        // sleep(1); // Uncomment this line if you have sleep function available
    }

    return 0;
}
