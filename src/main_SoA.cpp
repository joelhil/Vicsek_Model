
#include <SDL2/SDL.h>
#include <stdlib.h>
#include <stdio.h>
#include <cmath>
#include <iostream>
#include <random>
#include <vector>


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


bool inRadius(float interactionRadius,float interactionRadius2, float x_pos, float y_pos,float x_pos_other,float y_pos_other){
    // Check if the point is outside square before checking circle
    // to get less computation for most points
    if (x_pos < x_pos_other-interactionRadius || x_pos > x_pos_other+interactionRadius)
       return false;
    if (y_pos < y_pos_other-interactionRadius || y_pos > y_pos_other+interactionRadius)
       return false;


    float diff = (x_pos-x_pos_other)*(x_pos-x_pos_other)+(y_pos-y_pos_other)*(y_pos-y_pos_other);
    if (diff < interactionRadius2)
    {
        return true;
    } else {
        return false;
    }
}

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
    float radius = 1;
    float velocity = 3;
    int nParticles = 2000;

    // Variables affecting behaviour
    float noise = 0.5;
    float interactionRadius = 20;
    float inRadiusSquared = interactionRadius*interactionRadius;

    //Create graphic window
    Framework fw(height, width);
    SDL_Event event;

    // Generator for random numbers
    std::random_device                  rand_dev;
    std::mt19937                        generator(rand_dev());

    std::uniform_real_distribution<float>  xRand(radius*2, width-radius*2);
    std::uniform_real_distribution<float>  wRand(-0.5, 0.5);
    float pi2 = 3.14159*2;
    std::uniform_real_distribution<float>  thetaRand(0, pi2);


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

        float x_pos = 0;
        float y_pos = 0;
        float angle = 0;
        for (int i = 0; i<nParticles; i++){
            x_pos = posX[i];
            y_pos = posY[i];
            angle = angles[i];

            newPosX[i] = x_pos;
            newPosY[i] = y_pos;
            newVelY[i] = velY[i];
            newVelX[i] = velX[i];
            newAngles[i] = angle;
            fillPadding(posXPad, posYPad, anglePad, x_pos, y_pos, angle, width, height, interactionRadius);
        }

        // Move particles and calculate new angle of velocity
        float vX = 0;
        float vY = 0;
        int parts = 0;
        float x_pos_other = 0;
        float y_pos_other = 0;

        for (int i = 0; i<nParticles; i++){
            vX = 0;
            vY = 0;
            parts = 0;

            x_pos = posX[i];
            y_pos = posY[i];
            // Check for interaction radius
            for (int j = 0; j<nParticles; j++){
                x_pos_other = posX[j];
                y_pos_other = posY[j];
                if  (inRadius(interactionRadius,inRadiusSquared, x_pos, y_pos, x_pos_other, y_pos_other)){
                    vX += velX[j];
                    vY += velY[j];
                    parts ++;
                }
            }
            for (int k = 0; k<posXPad.size(); k++){
                x_pos_other = posXPad[k];
                y_pos_other = posYPad[k];
                if  (inRadius(interactionRadius,inRadiusSquared, x_pos, y_pos, x_pos_other, y_pos_other)){
                    vX += velocity*std::cos(anglePad[k]);
                    vY += velocity*std::sin(anglePad[k]);
                    parts ++;
                }
            }

            // + wRand(generator)*noise;
            float newAngle = 0;
            vY = vY/(velocity*parts);// + sin(wRand(generator)*noise);
            vX = vX/(velocity*parts);// + cos(wRand(generator)*noise);

            newAngle = std::atan2((vY),(vX));
            newAngle += wRand(generator)*noise;

            // Change angle and velocity of particle COPY
            newVelX[i] = velocity*std::cos(newAngle);
            newVelY[i] = velocity*std::sin(newAngle);
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
        SDL_Delay(0.5);
        iteration++;
    }

    ///////////////////////////////////////////////////////////////////////
    while (1) {
        if (SDL_PollEvent(&event) && event.type == SDL_QUIT)
            break;
        SDL_Delay(2);
    }

}
