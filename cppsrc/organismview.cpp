#include "organismview.h"
#include <SDL2/SDL.h>

#include "imgui.h"
#include "implot.h"
#include "lifeforms.h"
#include "util.h"

#define ORGANISM_VIEWER_SCALE_FACTOR 32.0

OrganismView::OrganismView(Organism *o, SDL_Renderer *r)
{
    this->myOrganism = o;
    this->mySpecies = o->species;
    for (int i = 0; i < cell_null; i++)
    {
        this->cellCounts[i] = o->cellCounts[i];
    }
    // memcpy(this->cellCounts, o->cellCounts, cell_null * sizeof(uint64_t));
    this->nCells = o->nCells();
    int maxX = 1;
    int maxY = 1;
    int organismX = o->x;
    int organismY = o->y;
    for (Cell *c : o->myCells)
    {
        int x_rel = c->x - organismX;
        int y_rel = c->y - organismY;

        if (abs(x_rel) > maxX)
            maxX = abs(x_rel);
        if (abs(y_rel) > maxY)
            maxY = abs(y_rel);
    }
    this->dim_x = (maxX * 2) + 1;
    this->dim_y = (maxY * 2) + 1;
    this->t = SDL_CreateTexture(r, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_TARGET,
                                this->dim_x * ORGANISM_VIEWER_SCALE_FACTOR, this->dim_y * ORGANISM_VIEWER_SCALE_FACTOR);

    SDL_SetRenderTarget(r, this->t);
    SDL_RenderSetScale(r, ORGANISM_VIEWER_SCALE_FACTOR, ORGANISM_VIEWER_SCALE_FACTOR);
    SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
    for (Cell *c : o->myCells)
    {
        SetColorForCell(r, c);
        int x_rel = c->x - o->x;
        int y_rel = c->y - o->y;
        SDL_RenderDrawPoint(r, x_rel + maxX, y_rel + maxY);
    }
    SDL_RenderSetScale(r, 1.0, 1.0);
    SDL_SetRenderTarget(r, nullptr);

    this->open = true;
    sprintf(this->name, "Organism Viewer (%p)", o);
}

void OrganismView::OnFrame()
{
    ImGui::Begin(this->name);
    ImGui::SetWindowSize(ImVec2(240, -1.0));

    if (ImGui::Button("Close", ImVec2(125, 25)))
    {
        printf("window closed!\n");
        this->open = false;
    }
    ImGui::Image(this->t, ImVec2(this->dim_x * ORGANISM_VIEWER_SCALE_FACTOR, this->dim_y * ORGANISM_VIEWER_SCALE_FACTOR));
    // ImGui::Text("Test text");
    const char *cellNames[cell_null] = {"Empty", "Plantmass", "Biomass", "Leaf", "Bark", "Flower", "Fruit", "Herbivore", "Carnivore", "Mover", "Killer", "Armor", "Touch sensor"};
    for (int i = 1; i < cell_null; i++)
    {
        ImGui::Text("%s:%d", cellNames[i], cellCounts[i]);
    }
    // static const uint32_t xs[cell_null] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
        ImPlot::PushColormap(CellColormap);
    int cmapsize = ImPlot::GetColormapSize();
        ImGui::Text("Colormap size is %d", cmapsize);
        for(int i = 0; i < cmapsize; i++)
        {
            ImVec4 thisColor = ImPlot::GetColormapColor(i);
            printf("%f/%f/%f/%f\n", thisColor.x, thisColor.y, thisColor.z, thisColor.w);
        }
    
    if (ImPlot::BeginPlot("Organism Makeup"))
    {
        ImPlot::PushColormap(CellColormap);

        ImPlot::SetupAxes("Cell Type", "Cell Count", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
        // ImPlot::SetupAxisLimits(ImAxis_X1, 1, cell_null);
        ImPlot::NextColormapColor();
        // for (int i = 1; i < cell_null; i++)
        // {
        // ImPlot::PlotPieChart(cellNames + i, this->cellCounts + i, 1, 240, 240, 100);
        // ImPlot::NextColormapColor();
        // }
        // ImPlot::PlotBars("Organism Makeup", this->cellCounts, this->cellCounts, cell_null, 20);
        // ImPlot::PlotStairs("Organism Makeup", xs, this->cellCounts, cell_null);
        // ImPlot::Plot
        // ImPlot::PlotLine("Organism Makeup", xs, this->cellCounts, cell_null);
        // ImPlot::PlotHistogram("Organism Makeup", this->cellCounts, cell_null);
        ImPlot::PlotPieChart(cellNames + 1, this->cellCounts + 1, cell_null - 1, 240, 240, 100);
        ImPlot::PopColormap();

        ImPlot::EndPlot();
    }
        ImPlot::PopColormap();
    

    ImGui::End();
}

OrganismView::~OrganismView()
{
    SDL_DestroyTexture(this->t);
}
