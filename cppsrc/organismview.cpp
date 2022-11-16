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
    ImGui::Begin(this->name, nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    // ImGui::SetWindowSize(ImVec2(240, 1000));

    if (ImGui::Button("Close", ImVec2(400, 25)))
    {
        printf("window closed!\n");
        this->open = false;
    }
    ImGui::Image(this->t, ImVec2(this->dim_x * ORGANISM_VIEWER_SCALE_FACTOR, this->dim_y * ORGANISM_VIEWER_SCALE_FACTOR));
    // ImGui::Text("Test text");
    
    ImVec2 size;
    if (ImPlot::BeginPlot("Organism Makeup"))
    {
        ImPlot::SetupLegend(ImPlotLocation_NorthEast);
        ImPlot::PushColormap(CellColormap);
        ImPlot::NextColormapColor();
        ImPlot::NextColormapColor();
        ImPlot::NextColormapColor();
        ImPlot::SetupAxes("Cell Type", "Cell Count", 0, 0);
        // skip empty/plantmass/biomass
        for (int i = cell_leaf; i < cell_null; i++)
        {
            ImPlot::PlotBars(cellNames[i], cellXs + i, this->cellCounts + i, 1, 1);
            // ImPlot::PlotHistogram(cellNames[i], this->cellCounts + i, 1, 1, 1.0, ImPlotRange(i - 1, i));
        }
        // ImPlot::PlotBarGroups(cellNames, this->cellCounts, cell_null, 1);
        // ImPlot::PlotBars()
        // ImPlot::PlotHistogram2D("abcd", )
        ImPlot::PopColormap();
        size = ImPlot::GetPlotSize();
        ImPlot::EndPlot();
    }

    ImGui::SetWindowSize(size);

    ImGui::End();
}

OrganismView::~OrganismView()
{
    SDL_DestroyTexture(this->t);
}
