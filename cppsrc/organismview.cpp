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
    this->VisualizeBrain();
}

// graph theory OwO
void OrganismView::VisualizeBrain()
{
    // this->myOrganism->brain->Dump();

    printf("Layer sizes: ");
    for (size_t i = 0; i < this->myOrganism->brain->size(); i++)
    {
        printf("%lu, ", this->myOrganism->brain->size(i));
    }
    printf("\n%lu units, %lu post numbers, %lu connections\n", this->myOrganism->brain->units().size(), this->myOrganism->brain->PostNumbers().size(), this->myOrganism->brain->connections().size());

    this->units = this->myOrganism->brain->units();
    this->idsByPost = this->myOrganism->brain->PostNumbers();
    // postsByUnit.clear();
    for (auto ibp : this->idsByPost)
    {
        this->postsById[ibp.second] = ibp.first;
    }
    printf("PBI size after generation: %lu\n", this->postsById.size());
    // for (auto pbu : postsByUnit)
    // {
    // printf("unit%lu : POST %lu\n", pbu.first->Id(), pbu.second);
    // }postsById

    for (auto ibp : this->idsByPost)
    {
        for (auto c : units[ibp.second]->OutboundConnections())
        {
            this->connectionsByPost[this->postsById[c->from->Id()]].push_back({this->postsById[c->to->Id()], this->myOrganism->brain->GetWeight(c->from->Id(), c->to->Id())});
        }
    }
    this->graph.push_back(std::set<size_t>());

    // put all hidden nodes into the graph to start
    for (size_t i = 0; i < this->myOrganism->brain->size(1); i++)
    {
        this->graph[0].insert(this->postsById[(*this->myOrganism->brain)[1][i].Id()]);
    }

    // push problem vertices into the next layer
    // a problem vertex is any one which has a connection that would require drawing the edge directly through another vertex in that column
    for (size_t columnIndex = 1; (columnIndex < this->graph.size()) && (this->graph[columnIndex].size() > 0); columnIndex++)
    {
        // make sure we always have a next column to vertices forward into
        this->graph.push_back(std::set<size_t>());

        // iterate every vertex in this column
        for (auto postNum = this->graph[columnIndex].begin(); postNum != this->graph[columnIndex].end(); ++postNum)
        {
            // iterate all connections for the vertex
            for (auto dest : this->connectionsByPost[*postNum])
            {
                size_t toId = dest.first;
                // if the destination is in the same column
                if (this->graph[columnIndex].count(toId))
                {
                    // figure out if the edge will have to be drawn through other vertices, push vertices to next layer to eliminate these collisions
                    // auto postDriver = postNum;
                    // while ((postDriver != this->graph[columnIndex].end()) && (*postDriver != toId))
                    // {
                    // ++postDriver;
                    // }
                    // size_t erasedPost = *postDriver++;
                    // should be able to be more greedy with this - only move the problematic vertex to next layer
                    // this->graph[columnIndex].erase(erasedPost);
                    this->graph[columnIndex].erase(toId);
                    this->graph[columnIndex + 1].insert(toId);
                    // postNum = postDriver;
                }
            }
        }
    }

    {
        // set up input layer
        size_t nInputs = this->myOrganism->brain->size(0);
        for (size_t i = 0; i < nInputs; i++)
        {
            this->inputs.push_back(this->postsById[(*this->myOrganism->brain)[0][i].Id()]);
        }
        size_t nOutputs = this->myOrganism->brain->size(2);
        // set up output layer
        for (size_t i = 0; i < nOutputs; i++)
        {
            this->outputs.push_back(this->postsById[(*this->myOrganism->brain)[2][i].Id()]);
        }
    }

    for (size_t i = 0; i < this->graph.size(); i++)
    {
        printf("LAYER %lu\n", i);
        for (auto j : this->graph[i])
        {
            printf("\tPOST %lu for unit %lu - connected to ", j, this->idsByPost[j]);
            for (auto c : this->connectionsByPost[j])
            {
                printf("%lu ", c.first);
            }
            printf("\n");
        }
    }
}

void OrganismView::OnFrame()
{
    ImGui::Begin(this->name, nullptr, 0);
    // ImGui::SetWindowSize(ImVec2(240, 1000));

    if (ImGui::Button("Close", ImVec2(400, 25)))
    {
        printf("window closed!\n");
        this->open = false;
    }
    ImGui::Image(this->t, ImVec2(this->dim_x * ORGANISM_VIEWER_SCALE_FACTOR, this->dim_y * ORGANISM_VIEWER_SCALE_FACTOR));
    // ImGui::Text("Test text");

    // ImVec2 size;
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
        // size = ImPlot::GetPlotSize();
        ImPlot::EndPlot();
    }

    if (ImGui::BeginTable("Brain Visualization", this->graph.size()))
    {
        for (size_t col = 0; col < this->graph.size(); col++)
        {
            auto coli = this->graph[col].begin();
            ImGui::TableNextColumn();

            for (size_t row = 0; row < this->graph[col].size(); row++)
            {
                ImGui::Text("%lu\n", *coli++);

                // ImGui::TableNextRow();
            }
        }

        ImGui::EndTable();
    }

    if (ImGui::BeginChild("brain"))
    {
        ImVec2 basePos = ImGui::GetWindowPos();
        std::map<size_t, ImVec2>
            positionsByPost;
        auto dl = ImGui::GetWindowDrawList();
        static const int diameter = 30;
        static const float yStepMultiplier = 1.5;
        size_t maxY = this->inputs.size();
        size_t maxX = this->graph.size() + 2;
        if (this->outputs.size() > maxY)
        {
            maxY = this->outputs.size();
        }

        for (size_t col = 0; col < this->graph.size(); col++)
        {
            size_t thisY = this->graph[col].size();
            if (thisY > maxY)
            {
                maxY = thisY;
            }
        }

        dl->AddRectFilled(ImVec2(basePos.x + 0.0, basePos.y + 0.0), ImVec2(basePos.x + ((maxX * 5.0) * diameter), basePos.y + ((maxY * yStepMultiplier) * (diameter + yStepMultiplier))), IM_COL32(0, 0, 0, 255));

        // draw input layer
        float yStepThisCol = (static_cast<float>(maxY) / this->inputs.size()) * diameter * yStepMultiplier;
        for (size_t row = 0; row < this->inputs.size(); row++)
        {
            ImVec2 thisPos(basePos.x + 5.0 * static_cast<float>(0 * diameter) + diameter / 2,
                           basePos.y + (row * yStepThisCol) + ((yStepThisCol + diameter) / 2));

            size_t thisPost = this->inputs[row];
            positionsByPost[thisPost] = thisPos;
            float activation = this->units[this->idsByPost[thisPost]]->Activation();
            sprintf(this->labelsByPost[thisPost], "%0.3f", activation);
            dl->AddText(ImVec2(thisPos.x - (diameter / 1.8),
                               thisPos.y + (diameter / 2)),
                        IM_COL32(255, 255, 255, 255), this->labelsByPost[thisPost]);
            dl->AddCircle(thisPos, diameter / 2, IM_COL32(255, 255, 255, 255));
            ImU32 inputColor;
            if (row > 6)
            {
                const ImVec4 &thisCellColor = cellColors[(row - 6) % cell_null];

                inputColor = IM_COL32(thisCellColor.x, thisCellColor.y, thisCellColor.z, activation * 255);
            }
            else
            {
                inputColor = IM_COL32(255, 255, 255, activation * 255);
            }
            dl->AddCircleFilled(thisPos, (diameter / 2) - 1, inputColor);
        }

        yStepThisCol = (static_cast<float>(maxY) / this->outputs.size()) * diameter * yStepMultiplier;
        for (size_t row = 0; row < this->outputs.size(); row++)
        {
            ImVec2 thisPos(basePos.x + 5.0 * static_cast<float>((maxX - 1) * diameter) + diameter / 2,
                           basePos.y + (row * yStepThisCol) + ((yStepThisCol + diameter) / 2));
            size_t thisPost = this->outputs[row];
            positionsByPost[thisPost] = thisPos;
            float activation = this->units[this->idsByPost[thisPost]]->Activation();
            sprintf(this->labelsByPost[thisPost], "%0.3f", activation);
            dl->AddText(ImVec2(thisPos.x - (diameter / 1.8),
                               thisPos.y + (diameter / 2)),
                        IM_COL32(255, 255, 255, 255), this->labelsByPost[thisPost]);
            dl->AddCircle(thisPos, diameter / 2, IM_COL32(255, 255, 255, 255));
            dl->AddCircleFilled(thisPos, (diameter / 2) - 1, IM_COL32(255, 255, 255, activation * 255));
        }

        for (size_t col = 0; col < this->graph.size(); col++)
        {
            auto coli = this->graph[col].begin();
            yStepThisCol = (static_cast<float>(maxY) / this->graph[col].size()) * diameter * yStepMultiplier;
            for (size_t row = 0; row < this->graph[col].size(); row++)
            {
                ImVec2 thisPos(basePos.x + 5.0 * static_cast<float>((col + 1) * diameter) + diameter / 2,
                               basePos.y + (row * yStepThisCol) + ((yStepThisCol + diameter) / 2));
                size_t thisPost = *coli++;
                positionsByPost[thisPost] = thisPos;
                float activation = this->units[this->idsByPost[thisPost]]->Activation();
                sprintf(this->labelsByPost[thisPost], "%0.3f", activation);
                dl->AddText(ImVec2(thisPos.x - (diameter / 1.8), thisPos.y + (diameter / 2)), IM_COL32(255, 255, 255, 255), this->labelsByPost[thisPost]);
                dl->AddCircle(thisPos, diameter / 2, IM_COL32(255, 255, 255, 255));
                dl->AddCircleFilled(thisPos, (diameter / 2) - 1, IM_COL32(255, 255, 255, activation * 255));
                // Brain *b = this->myOrganism->brain;
                // b->units()
                // dl->AddCircleFilled(thisPos, diameter / 2, )
                // ImGui::TableNextRow();
            }
        }

        for (auto cbp : this->connectionsByPost)
        {
            for (auto destVertex : cbp.second)
            {
                ImVec2 src = positionsByPost[cbp.first];
                ImVec2 dst = positionsByPost[destVertex.first];
                float dist = sqrt(pow(dst.x - src.x, 2) + pow(dst.y - src.y, 2));
                ImVec2 unitSlopeVec((dst.x - src.x) / dist, (dst.y - src.y) / dist);
                src.x += (unitSlopeVec.x * (diameter / 2)) + 1;
                src.y += (unitSlopeVec.y * (diameter / 2)) + 1;
                dst.x -= (unitSlopeVec.x * (diameter / 2)) + 1;
                dst.y -= (unitSlopeVec.y * (diameter / 2)) + 1;
                dl->PathLineTo(src);
                dl->PathLineTo(dst);
                ImU32 edgeColor = (destVertex.second > 0.0) ? IM_COL32(0, 255.0 * destVertex.second, 0, 255) : IM_COL32(-255.0 * destVertex.second, 0, 0, 255);
                dl->PathStroke(edgeColor, 0, 1.0);
                // ImVec2 origin(dst.x - 0.9 * src.x)
                ImVec2 otherTPtStart(dst.x - (10.0 * unitSlopeVec.x), dst.y - (10.0 * unitSlopeVec.y));
                ImVec2 orthogonalSlopeVec(unitSlopeVec.y, -1.0 * unitSlopeVec.x);
                dl->AddTriangleFilled(dst,
                                      ImVec2(otherTPtStart.x + (5.0 * orthogonalSlopeVec.x), otherTPtStart.y + (5.0 * orthogonalSlopeVec.y)),
                                      ImVec2(otherTPtStart.x - (5.0 * orthogonalSlopeVec.x), otherTPtStart.y - (5.0 * orthogonalSlopeVec.y)),
                                      edgeColor);
            }
        }
        ImGui::EndChild();
    }

    // ImGui::SetWindowSize(size);

    ImGui::End();
}

OrganismView::~OrganismView()
{
    SDL_DestroyTexture(this->t);
}
