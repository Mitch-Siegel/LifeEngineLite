#include "detailedstats.h"
#include <SDL.h>
#include <map>

#include "imgui.h"
#include "implot.h"

#include "util.h"
#include "implot.h"
#include "board.h"

bool showDetailedStats = false;
extern Board *board;
void DetailedStats()
{
    board->GetMutex();
    // static ImVec2 size(0, 0);
    // size.y = 0;

    double cellCounts[class_null][cell_null] = {{0}};
    int classCounts[class_null] = {0};

    for (Organism *o : board->Organisms)
    {
        OrganismClassifications classificaion = o->Classify();
        for (int i = 0; i < cell_null; i++)
        {
            cellCounts[classificaion][i] += o->cellCounts[i];
        }
        classCounts[classificaion]++;
    }

    for (int i = 0; i < class_null; i++)
    {
        for (int j = 0; j < cell_null; j++)
        {
            cellCounts[i][j] /= classCounts[i];
        }
    }

    ImGui::SetNextWindowSize(ImVec2(960, 960));
    ImGui::Begin("Detailed Statistics", &showDetailedStats, ImGuiWindowFlags_AlwaysAutoResize);
    /*if (ImGui::BeginTable("asdf", class_null))
    {
        for (int j = 0; j < cell_null; j++)
        {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            for (int i = 0; i < class_null; i++)
            {
                ImGui::TableSetColumnIndex(i);
                ImGui::Text("%.1f", cellCounts[i][j]);
            }
        }
        ImGui::EndTable();
    }*/
    // ImGui::SetWindowSize(ImVec2(960, 1000));

    const char *classNames[class_null] = {"Plant", "Mover"};
    for (int i = 0; i < class_null; i++)
    {

        // ImPlot::SetNextAxisToFit(ImAxis_X1);
        double max = 0.0;
        for (int j = 0; j < cell_null; j++)
        {
            if (cellCounts[i][j] > max)
            {
                max = cellCounts[i][j];
            }
        }
        max = ceil(max);

        // ImPlot::SetNextAxisToFit(ImAxis_X1);
        ImPlot::SetNextAxesToFit();
        ImPlot::SetNextAxisLimits(ImAxis_Y1, 0, max);
        if (ImPlot::BeginPlot(classNames[i]))
        {
            ImPlot::PushColormap(CellColormap);
            ImPlot::NextColormapColor();
            ImPlot::NextColormapColor();
            ImPlot::NextColormapColor();
            for (int j = cell_leaf; j < cell_null; j++)
            {
                ImPlot::PlotBars(cellNames[j], cellXs_double + j, cellCounts[i] + j, 1, 1);
            }
            ImPlot::PopColormap();
            ImPlot::EndPlot();
        }
    }

    ImGui::End();
    board->ReleaseMutex();
}