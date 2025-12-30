#include <vector>
#include <SFML/Graphics.hpp>
#include "imgui.h"
#include "imgui-SFML.h"

class Simulation {
    using Grid = std::vector<std::vector<float>>;
    public:
        Simulation(int w, int h, float timeChange) : width(w), height(h), dt(timeChange){
            grid.resize(height, std::vector<int>(width, 0));
            u.resize(height, std::vector<float>(width, 0));
            uTemp.resize(height, std::vector<float>(width, 0));
            src.resize(height, std::vector<float>(width, 0));
            dst.resize(height, std::vector<float>(width, 0));
            v.resize(height, std::vector<float>(width, 0));
            vTemp.resize(height, std::vector<float>(width, 0));
            density.resize(height, std::vector<float>(width, 0));
            densityTemp.resize(height, std::vector<float>(width, 0));
            pressure.resize(height, std::vector<float>(width, 0));
            divergence.resize(height, std::vector<float>(width, 0));
        }
    void addPixel(int x, int y)
        {
            if (x < 0 || x >= width || y < 0 || y >= height)
                return;
            for (int z = -5; z < x+5; ++z){
                for (int c = -5; c < y+5; ++c){
                    if (x+z > 0 || x+z <= width || y+c > 0 || y+c <= height){
                        density[y+c][x+z] = 1.0f;   // add ink / fluid material
                        grid[y+c][x+z] = 1;
                    }
                }
            }
            // Optional: give it a small downward push
            v[y][x] += 5.0f;
        }
    
    void update(){
        float g = 2000.0f;
        
        for (int y = 0; y < height; ++y){
            for (int x = 0 ; x < width; ++x) {
                if (density[y][x] >  0.0f) {
                    v[y][x] += g * dt;
                }
            }
        }
        advect(u, uTemp, u, v);
        std::swap(u, uTemp);
        advect(v, vTemp, u, v);
        std::swap(v, vTemp);

        // Difuse Later
        //

        advect(density, densityTemp, u, v);
        std::swap(density, densityTemp);

        for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x)
        {
            grid[y][x] = (density[y][x] > 0.001f) ? 1 : 0;
        }   


    }
    void advect(
            const Grid& src,
            Grid& dst,
            const Grid& velU,
            const Grid& velV
            )
    {
        for (int y = 0; y < height; ++y){
            for (int x = 0; x < width;  ++x)
            {
                float vx = velU[y][x];
                float vy = velV[y][x];

                float prevX = x - vx * dt;
                float prevY = y - vy * dt;

                prevX = std::clamp(prevX, 0.5f, (float)width  - 1.5f);
                prevY = std::clamp(prevY, 0.5f, (float)height - 1.5f);

                int x0 = (int)prevX;
                int y0 = (int)prevY;
                int x1 = x0 + 1;
                int y1 = y0 + 1;

                float sx = prevX - x0;
                float sy = prevY - y0;

                float q00 = src[y0][x0];
                float q10 = src[y0][x1];
                float q01 = src[y1][x0];
                float q11 = src[y1][x1];

                dst[y][x] =
                    (1-sx)*(1-sy)*q00 +
                     sx   *(1-sy)*q10 +
                    (1-sx)* sy   *q01 +
                     sx   * sy   *q11;
            }
        }
    }
    void renderToPixels(std::vector<sf::Uint8>& pixels)
    {
        for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x)
        {
            float d = density[y][x];
            int i = 4 * (y * width + x); // RGBA index

            if (d > 0.001f)
            {
                pixels[i+0] = 0;   // R
                pixels[i+1] = 0;   // G
                pixels[i+2] = 255; // B
                pixels[i+3] = 255; // A
            }
            else
            {
                pixels[i+0] = pixels[i+1] = pixels[i+2] = 0;
                pixels[i+3] = 255;
            }
        }
    }


    private:
        int width;
        int height;
        float dt;
        std::vector<std::vector<int>> grid;
        std::vector<std::vector<float>> src; //source
        std::vector<std::vector<float>> dst; //destination
        std::vector<std::vector<float>> u; //horizontal velocity
        std::vector<std::vector<float>> uTemp; //source
        std::vector<std::vector<float>> v; //vertical velocity
        std::vector<std::vector<float>> vTemp; //source
        std::vector<std::vector<float>> density; //density
        std::vector<std::vector<float>> densityTemp; //source
        std::vector<std::vector<float>> pressure; //pressure
        std::vector<std::vector<float>> divergence; //Not sure yet
};

int main() {
    //Set up Window Sizes
    int width = 800;
    int height = 600;

    sf::RenderWindow window(
        sf::VideoMode(width, height),
        "Fluid Simulator",
        sf::Style::Default
    );
    window.setFramerateLimit(60);

    ImGui::SFML::Init(window);

    float viscosity = 0.1f;
    float dt = 0.01f;

    Simulation sim(width, height, dt);

    sf::Clock deltaClock;
    std::vector<sf::Uint8> pixels(width * height * 4, 0); // RGBA buffer
    sf::Texture tex;
    tex.create(width, height);

    sf::Sprite sprite(tex);
    window.draw(sprite);
    while (window.isOpen()) {
        sf::Event event{};
        while (window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(event);

            if (event.type == sf::Event::Closed){
                window.close(); 
            }
            if (event.type == sf::Event::MouseButtonPressed){
                sim.addPixel(event.mouseButton.x, event.mouseButton.y);
            }
        }
        if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
        {
            sf::Vector2i m = sf::Mouse::getPosition(window);
            sim.addPixel(m.x, m.y);
        }

        sim.update();
        sim.renderToPixels(pixels);
        tex.update(pixels.data());
        
        ImGui::SFML::Update(window, deltaClock.restart());

        // --- UI Panel ---
        ImGui::Begin("Simulation Controls");
        ImGui::SliderFloat("Viscosity", &viscosity, 0.0f, 1.0f);
        ImGui::SliderFloat("Time Step", &dt, 0.0f, 0.1f);
        ImGui::Text("Simulation coming soon...");
        ImGui::End();

        window.clear(sf::Color::Black);
        window.draw(sprite);
        // (later: draw your fluid field here)

        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();
    return 0;
}

