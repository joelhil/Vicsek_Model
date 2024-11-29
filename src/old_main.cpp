
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




//When drawing square, use -1 on both positions and -2 on width for grid net
void draw_square_white(SDL_Renderer *renderer,int blCorner_x, int blCorner_y, int width_){
    // Setting the color to be RED with 100% opaque (0% trasparent).
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    // Drawing square
    for(int x=blCorner_x; x<=blCorner_x+width_; x++){
        for(int y=blCorner_y; y<=blCorner_y+width_; y++){
            SDL_RenderDrawPoint(renderer, x, y);
        }
    }
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

void draw_circle_black(SDL_Renderer *renderer,int center_x, int center_y, int radius_){
    // Setting the color to be RED with 100% opaque (0% trasparent).
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
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



// A particle has velocity and position both split into x and y directions
// Random start and angle is given to constructor since this was hard to
// do otherwise.
// Velocity is assumed to be in right direction before a random angle is
// generetad and the particle velocity is rotated using rotaion matrix.
class Particle{
    public:
        float pos[2];
        float vel[2];
        float currentAngle;

        // Dummy constructor
        Particle() {}

        // Real constructor
        Particle(float xMax, float yMax, float radius, float velocity,float xPos,float yPos,float angle)
        {
            pos[0] = xPos;
            pos[1] = yPos;
            //pos[1] = (pos[1]*yMax+xMax/2)/xMax;
            pos[1]=pos[1]*yMax/xMax;

            vel[0]=velocity*std::cos(angle);
            vel[1]=velocity*std::sin(angle);
            currentAngle = angle;

        }

        // Move position and draw new circle
        void move_p(float xMax, float yMax)
        {
            pos[0]+=vel[0];
            pos[1]+=vel[1];
            if (pos[0] > xMax) {
                pos[0]=0;
            } else if (pos[0] < 0) {
                pos[0] = xMax;
            } else if (pos[1] > yMax) {
                pos[1]=0;
            } else if (pos[1] < 0) {
                pos[1] = yMax;
            }
        }

        // Change angle of velocity
        void updateVel(float angle,float velocity){
            vel[0]=velocity*std::cos(angle);
            vel[1]=velocity*std::sin(angle);
            currentAngle=angle;
        }

        void updateVelNew(float Vx, float Vy){
            vel[0] = Vx;
            vel[1] = Vy;
        }

};


bool inRadius(float interactionRadius,float interactionRadius2, float posCenter[], float posOther[]){
    // Check if the point is outside square before checking circle
    // to get less computation for most points
    if (posCenter[0] < posOther[0]-interactionRadius || posCenter[0] > posOther[0]+interactionRadius)
       return false;
    if (posCenter[1] < posOther[1]-interactionRadius || posCenter[1] > posOther[1]+interactionRadius)
       return false;


    float diff = std::pow(posCenter[0]-posOther[0],2)+std::pow(posCenter[1]-posOther[1],2);
    if (diff < interactionRadius2)
    {
        return true;
    } else {
        return false;
    }
}


// Add copies of particles outside of bound so that a particle close to an edge
// can interact with a particle on the opposite side of the periodic boundary
void fillPadding(int i, std::vector<Particle> &outsiders, Particle swarm[], float xMax, float yMax, float interactionRadius) {
    Particle p = swarm[i];
    bool add = false;

    if (p.pos[0] + interactionRadius > xMax) {
        p.pos[0] -= xMax;
        add = true;
    } else if (p.pos[0] - interactionRadius < 0) {
        p.pos[0] += xMax;
        add = true;
    }
    if (p.pos[1] + interactionRadius > yMax) {
        p.pos[1] -= yMax;
        add = true;
    } else if (p.pos[1] - interactionRadius < 0) {
        p.pos[1] += yMax;
        add = true;
    }

    if (add) {
        outsiders.push_back(p);
    }
}


int main(int argc, char * argv[]){
    // Creating the object by passing Height and Width value.
    // Physics variables
    float height = 1200;
    float width = 1200;
    float radius = 1;
    float velocity = 3;
    int nParticles = 5000;


    // Variables affecting behaviour
    float noise = 0.5;
    float interactionRadius = 15;
    float inRadiusSquared = interactionRadius*interactionRadius;


    //Create graphic window
    Framework fw(height, width);
    SDL_Event event;

    // Generator for random numbers
    std::random_device                  rand_dev;
    std::mt19937                        generator(rand_dev());

    //std::array<Particle,10> swarm[0](width, height, radius, velocity, generator, renderer);
    std::uniform_real_distribution<float>  xRand(radius*2, width-radius*2);
    std::uniform_real_distribution<float>  wRand(-0.5, 0.5);
    float pi2 = 3.14159*2;
    std::uniform_real_distribution<float>  thetaRand(0, pi2);


    // Initialize the swarm
    Particle* swarm = new Particle[nParticles];
    for (int i = 0; i<nParticles; i++){
        float xPos = xRand(generator);
        float yPos = xRand(generator) ;
        float angle = thetaRand(generator);
        swarm[i]=Particle(width, height, radius, velocity, xPos, yPos, angle);
        //std::cout << "\n" << swarm[i].vel[0] << " " << swarm[i].vel[1];
    }

    //Create empty new swarm
    Particle* newSwarm = new Particle[nParticles];


    // Main loop start here stops when key 'q' is pressed
    const Uint8* state = SDL_GetKeyboardState(nullptr);
    int iteration = 0;
    while (!state[SDL_SCANCODE_Q]) //&& iteration<10)
    {
        SDL_SetRenderDrawColor(fw.renderer, 0, 0, 0, 255);
        SDL_RenderClear(fw.renderer);

        std::vector <Particle> outsiders;

        //New swarm create

        for (int i = 0; i<nParticles; i++){
            //newSwarm[i]=Particle(width, height, radius, velocity, swarm[i].pos[0], swarm[i].pos[1], swarm[i].currentAngle, fw.renderer);
            newSwarm[i] = swarm[i];
            // Add particles close to edges and change position incase they affect interaction
            fillPadding(i,outsiders,swarm,width,height,interactionRadius);
        }

        // Move particles and calculate new angle of velocity
        //#pragma omp parallel for num_threads(6)
        for (int i = 0; i<nParticles; i++){
            float vX = 0;
            float vY = 0;
            int parts = 0;
            // Check for interaction radius
            for (int j = 0; j<nParticles; j++){
                if  (inRadius(interactionRadius,inRadiusSquared,swarm[i].pos,swarm[j].pos)){
                    vX += swarm[j].vel[0];
                    vY += swarm[j].vel[1];
                    //std::cout << "\n" << sinSum << " , " << vY;
                    parts ++;
                }
            }
            for (int k = 0; k<outsiders.size();k++){
                if  (inRadius(interactionRadius,inRadiusSquared,swarm[i].pos,outsiders[k].pos)){
                    vX += outsiders[k].vel[0];
                    vY += outsiders[k].vel[1];
                    parts ++;
                }

            }


            // + wRand(generator)*noise;
            float newAngle = 0;
            vY = vY/(velocity*parts);// + sin(wRand(generator)*noise);
            vX = vX/(velocity*parts);// + cos(wRand(generator)*noise);

            newAngle = std::atan2((vY),(vX));
            newAngle += wRand(generator)*noise;

            newSwarm[i].updateVel(newAngle,velocity);
            //newSwarm[i].updateVelNew(vX,vY);
            newSwarm[i].move_p(width,height);
        }


        // Update values for original swarm and draw them
        for (int i = 0; i<nParticles; i++){
            swarm[i].pos[0] = newSwarm[i].pos[0];
            swarm[i].pos[1] = newSwarm[i].pos[1];
            swarm[i].updateVel(newSwarm[i].currentAngle,velocity);
            draw_circle_white(fw.renderer,swarm[i].pos[0],swarm[i].pos[1],radius);
        }
        SDL_RenderPresent(fw.renderer);      // Update rendering
        SDL_PumpEvents();                 // Check if 'q' was pressed
        SDL_Delay(0.01);
        iteration++;

    }

    ///////////////////////////////////////////////////////////////////////
    while (1) {
        if (SDL_PollEvent(&event) && event.type == SDL_QUIT)
            break;
        SDL_Delay(2);
    }

}
