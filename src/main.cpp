
#include <SDL2/SDL.h>
#include <stdlib.h>
#include <stdio.h>
#include <cmath>
#include <iostream>
#include <random>
#include <vector>
#include <algorithm>



// SDL2 framework class
class Framework{
public:
    // Contructor which initialize the parameters.
    SDL_Renderer *renderer = NULL;      // Pointer for the renderer
    SDL_Window *window = NULL;
    Framework(int height_, int width_): height(height_), width(width_){
        SDL_Init(SDL_INIT_VIDEO);       // Initializing SDL as Video
        SDL_CreateWindowAndRenderer(width, height, 0, &window, &renderer);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);      // setting draw color
        SDL_RenderClear(renderer);      // Clear the newly created window
        SDL_RenderPresent(renderer);    // Reflects the changes done in the

                                        //  window.
    }

    // Destructor
    ~Framework(){
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

private:
    int height;     // Height of the window
    int width;      // Width of the window
// Pointer for the window
};



void draw_pixel_white(SDL_Renderer *renderer,int x, int y){
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawPoint(renderer, x, y);
}


//When drawing square, use -1 on both positions and -2 on width for grid net
void draw_circle_white(SDL_Renderer *renderer,int center_x, int center_y, int radius_){
    // Setting the color to be RED with 100% opaque (0% trasparent).
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    // Drawing square
    for(int x=center_x-radius_; x<=center_x+radius_; x++)
    {
        for(int y=center_y-radius_; y<=center_y+radius_; y++)
        {
            if ((std::pow(center_x-x,2) +  std::pow(center_y-y,2)) <= std::pow(radius_,2) )
            {
                    SDL_RenderDrawPoint(renderer, x, y);
                }
        }
    }
}

// Define a struct for points
struct Point {
    float x, y;
    int index; // Original index in the arrays for reference
};

// Define a class for the Quadtree
class Quadtree {
public:
    // Define the boundary of the quadtree
    struct Boundary {
        float x, y; // Center of the boundary
        float halfWidth, halfHeight; // Half-dimensions
    } boundary;

    int capacity; // Maximum points in a node before subdivision
    std::vector<Point> points; // Points stored in this node
    bool divided = false; // Whether this node is subdivided
    Quadtree* northwest = nullptr;
    Quadtree* northeast = nullptr;
    Quadtree* southwest = nullptr;
    Quadtree* southeast = nullptr;

    // Constructor
    Quadtree(Boundary boundary, int capacity)
        : boundary(boundary), capacity(capacity) {}

    ~Quadtree() {
        delete northwest;
        delete northeast;
        delete southwest;
        delete southeast;
    }

    // Insert a point into the Quadtree
    bool insert(const Point& point) {
        // Check if the point is within the boundary
        if (!contains(point)) return false;

        // If there's space, add the point
        if (points.size() < capacity) {
            points.push_back(point);
            return true;
        }

        // Otherwise, subdivide if necessary
        if (!divided) subdivide();

        // Insert into the appropriate quadrant
        return northwest->insert(point) || northeast->insert(point) ||
               southwest->insert(point) || southeast->insert(point);
    }

    // Query points within a radius
    void query(float x, float y, float radius, std::vector<int>& found) const {
        // Check if the search area intersects this boundary
        if (!intersects(x, y, radius)) return;

        // Check points in this node
        for (const auto& point : points) {
            float dx = x - point.x;
            float dy = y - point.y;
            if (dx * dx + dy * dy <= radius * radius) {
                found.push_back(point.index);
            }
        }

        // Query children if divided
        if (divided) {
            northwest->query(x, y, radius, found);
            northeast->query(x, y, radius, found);
            southwest->query(x, y, radius, found);
            southeast->query(x, y, radius, found);
        }
    }

private:
    // Check if a point is within the boundary
    bool contains(const Point& point) const {
        return point.x >= boundary.x - boundary.halfWidth &&
               point.x < boundary.x + boundary.halfWidth &&
               point.y >= boundary.y - boundary.halfHeight &&
               point.y < boundary.y + boundary.halfHeight;
    }

    // Check if a circle intersects the boundary
    bool intersects(float x, float y, float radius) const {
        float dx = std::max(std::abs(x - boundary.x) - boundary.halfWidth, 0.0f);
        float dy = std::max(std::abs(y - boundary.y) - boundary.halfHeight, 0.0f);
        return (dx * dx + dy * dy) <= radius * radius;
    }

    // Subdivide the Quadtree into four quadrants
    void subdivide() {
        float hw = boundary.halfWidth / 2.0f;
        float hh = boundary.halfHeight / 2.0f;

        northwest = new Quadtree({boundary.x - hw, boundary.y - hh, hw, hh}, capacity);
        northeast = new Quadtree({boundary.x + hw, boundary.y - hh, hw, hh}, capacity);
        southwest = new Quadtree({boundary.x - hw, boundary.y + hh, hw, hh}, capacity);
        southeast = new Quadtree({boundary.x + hw, boundary.y + hh, hw, hh}, capacity);

        divided = true;
    }
};



// Add copies of particles outside of bound so that a particle close to an edge
// can interact with a particle on the opposite side of the periodic boundary
void fillPadding(std::vector<float> &posXPad, std::vector<float> &posYPad, 
                 std::vector<float> &anglePad, float x_pos, float y_pos, float angle, 
                 float xMax, float yMax, float interactionRadius) {

    // Check for boundary interactions and add padded particles
    if (x_pos + interactionRadius > xMax) {
        posXPad.push_back(x_pos - xMax); // Wrap around X (right to left)
        posYPad.push_back(y_pos);        // Same Y
        anglePad.push_back(angle);
    } else if (x_pos - interactionRadius < 0) {
        posXPad.push_back(x_pos + xMax); // Wrap around X (left to right)
        posYPad.push_back(y_pos);        // Same Y
        anglePad.push_back(angle);
    } 
    if (y_pos + interactionRadius > yMax) {
        posXPad.push_back(x_pos);        // Same X
        posYPad.push_back(y_pos); // Wrap around Y (top to bottom)
        anglePad.push_back(angle);
    } else if (y_pos - interactionRadius < 0) {
        posXPad.push_back(x_pos);        // Same X
        posYPad.push_back(y_pos + yMax); // Wrap around Y (bottom to top)
        anglePad.push_back(angle);
    }
}


int main(int argc, char * argv[]){
    // Creating the object by passing Height and Width value.
    // Physics variables
    float height = 900;
    float width = 900;
    float radius = 2;
    float velocity = 2;
    int nParticles = 7000;

    // Variables affecting behaviour
    float noise = 0.7;
    float interactionRadius = 10;
    float inRadiusSquared = interactionRadius*interactionRadius;

    //Create graphic window
    Framework fw(height, width);
    SDL_Event event;

    // Generator for random numbers
    std::random_device                  rand_dev;
    std::mt19937                        generator(rand_dev());

    std::uniform_real_distribution<float>  xRand(radius*2, width-radius*2);
    std::uniform_real_distribution<float>  wRand(-0.5, 0.5);
    float pi = 3.14159;
    std::uniform_real_distribution<float>  thetaRand(-pi, pi);


    // Init our swarm arrays
    std::vector<float> posX(nParticles);
    std::vector<float> posY(nParticles);
    std::vector<float> velX(nParticles);
    std::vector<float> velY(nParticles);
    std::vector<float> angles(nParticles);

    for (int i = 0; i<nParticles; i++){
        posX[i] = xRand(generator);
        posY[i] = xRand(generator); // Depending on screen size Y might have to be scaled
        angles[i] = thetaRand(generator);
        velX[i]=velocity*std::cos(angles[i]);
        velY[i]=velocity*std::sin(angles[i]);
    }

    // Attributes of updated particles
    std::vector<float> newPosX(nParticles);
    std::vector<float> newPosY(nParticles);
    std::vector<float> newVelX(nParticles);
    std::vector<float> newVelY(nParticles);
    std::vector<float> newAngles(nParticles);

    // Padding (for interactions thorugh boundary)
    std::vector<float> posXPad, posYPad, anglePad;

    // Main loop start here stops when key 'q' is pressed
    const Uint8* state = SDL_GetKeyboardState(nullptr);
    int iteration = 0;
    while (!state[SDL_SCANCODE_Q]) //&& iteration<10)
    {
        SDL_SetRenderDrawColor(fw.renderer, 0, 0, 0, 255);
        SDL_RenderClear(fw.renderer);

        float x_pos = 0; // Maybe excessive amount of help variables?
        float y_pos = 0;
        float angle = 0;
        for (int i = 0; i<nParticles; i++){
            x_pos = posX[i]; 
            y_pos = posY[i];
            angle = angles[i];

            newPosX[i] = x_pos; // This should be vectorized
            newPosY[i] = y_pos;
            newVelY[i] = velY[i];
            newVelX[i] = velX[i];
            newAngles[i] = angle;
            fillPadding(posXPad, posYPad, anglePad, x_pos, y_pos, angle, width, height, interactionRadius);
        }

        // Define the quadtree boundary
        float halfWidth = width/2.0f; // Adjust to your simulation space
        float halfHeight = height/2.0f;
        Quadtree::Boundary boundary = {halfWidth, halfHeight, halfWidth, halfHeight};
        Quadtree tree(boundary, 8); // Set capacity per node

        // Insert points into the Quadtree
        for (int i = 0; i < nParticles; ++i) {
            tree.insert({posX[i], posY[i], i});
        }
        for (int i = 0; i < posXPad.size(); ++i) {
            tree.insert({posXPad[i], posYPad[i], -1}); // Use -1 or similar for padding points
        }

        // Move particles and calculate new angle of velocity
        float vX = 0;
        float vY = 0;
        int parts = 0;
        float x_pos_other = 0;
        float y_pos_other = 0;

        
        for (int i = 0; i<nParticles; i++){
            std::vector<int> neighbors;
            tree.query(posX[i], posY[i], interactionRadius, neighbors);

            float vX = 0.0f, vY = 0.0f;
            int parts = 0;

            // Process neighbors
            // #pragma omp parallel for reduction(+:vX,vY)
            for (int idx : neighbors) {
                if (idx >= 0) { // Regular particle
                    vX += velX[idx];
                    vY += velY[idx];
                } else { // Padding particle
                    int padIdx = -(idx + 1);
                    vX += velocity * std::cos(anglePad[padIdx]);
                    vY += velocity * std::sin(anglePad[padIdx]);
                }
                ++parts;
            }

            /* // + wRand(generator)*noise;
            float newAngle = 0;
            vY = vY/(velocity*parts);// + sin(wRand(generator)*noise);
            vX = vX/(velocity*parts);// + cos(wRand(generator)*noise);

            newAngle = std::atan2((vY),(vX));
            newAngle += wRand(generator)*noise;
 */
            // Update particle velocity and position
            if (parts > 0) {
                vX /= (velocity * parts);
                vY /= (velocity * parts);
            }
            float newAngle = std::atan2(vY, vX) + wRand(generator) * noise;

            newVelX[i] = velocity * std::cos(newAngle);
            newVelY[i] = velocity * std::sin(newAngle);
            newAngles[i] = newAngle;

            newPosX[i] += newVelX[i];
            newPosY[i] += newVelY[i];

            if (newPosX[i] > width) {
                newPosX[i]=0;
            } else if (newPosX[i] < 0) {
                newPosX[i] = width;
            } else if (newPosY[i] > height) {
                newPosY[i]=0;
            } else if (newPosY[i] < 0) {
                newPosY[i] = height;
            }
            draw_pixel_white(fw.renderer,newPosX[i],newPosY[i]);
        }

        std::swap(posX, newPosX);
        std::swap(posY, newPosY);
        std::swap(velX, newVelX);
        std::swap(velY, newVelY);
        std::swap(angles, newAngles);
        // Update values for original swarm and draw them
        /* 
        for (int i = 0; i<nParticles; i++){
            
        } */
        posXPad.clear();
        posYPad.clear();
        anglePad.clear();

        SDL_RenderPresent(fw.renderer);      // Update rendering
        SDL_PumpEvents();                 // Check if 'q' was pressed
        SDL_Delay(1);
        iteration++;
    }

    ///////////////////////////////////////////////////////////////////////
    while (1) {
        if (SDL_PollEvent(&event) && event.type == SDL_QUIT)
            break;
        SDL_Delay(2);
    }

}
