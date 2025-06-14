# Vicsek_Model
Example of Vicsek's model for simulating flocks, used in assignments in FFR120 at Chalmers university of technology.
Made in c++ with SDL2, hold 'q' key to quit and then close window.  


Example of model running with parameters:  
    // Physics variables  
    float height = 900;  
    float width = 900;  
    float radius = 2;  
    float velocity = 2;  
    int nParticles = 7000;  
    
    // Variables affecting behaviour  
    float noise = 0.6;  
    float interactionRadius = 10;  
![Demo](gif_Vicsek.gif)
