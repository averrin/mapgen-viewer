/* **************************************************************************************
 *    Author: Scott Mudge
 *    MAIL@SCOTTMUDGE.COM
 *
 *                                  MIT License
 *                                  -----------
 *                               MAIL@SCOTTMUDGE.COM
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * **************************************************************************************/
// File Name: imgui_tabs.cpp
// File Description: Custom ImGui tab system module


#include <string>
#include "imgui_tabs.h"

ImGui::ImGuiUserStyle::ImGuiUserStyle(){
    Colors[ImGuiUserCol_TabNormal] = ImVec4(0.65f, 0.65f, 0.68f, 1.00f);
    Colors[ImGuiUserCol_TabTitleTextNormal] = ImVec4(0.0f, 0.0f, 0.0f, 0.5f);
    Colors[ImGuiUserCol_TabTitleTextSelected] = ImVec4(1.0f, 1.0f, 1.0f, 1.00f);
    Colors[ImGuiUserCol_TabBorder] = Colors[ImGuiUserCol_TabNormal] * ImVec4(1.15f,1.15f,1.15f,1.0f);
    Colors[ImGuiUserCol_TabBorderShadow] = Colors[ImGuiUserCol_TabNormal] * ImVec4(0.65f,0.65f,0.65f,1.0f);
    Colors[ImGuiUserCol_TabHover] = Colors[ImGuiUserCol_TabNormal] * ImVec4(1.15f,1.15f,1.15f,1.0f);
}

const ImU32 ImGui::GetColorU32(ImGuiUserCol_ idx, float alpha_mul) {
    ImVec4 c = UserStyle.Colors[idx];
    if (alpha_mul > 0.0f)
        c.w *= GImGui->Style.Alpha * alpha_mul;
    return ColorConvertFloat4ToU32(c);
}

void ImGui::_drawPartialRect(const ImVec2 a, const ImVec2 b, const float rounding, const int rounding_corners,
                             ImDrawList *dl, const _EdgeType edges, const ImU32 color, const bool shadow,
                             const _EdgeType shadow_edges, const float shadow_offset, const float shadow_alpha) {
    float r = rounding;
    r = ImMin(r, fabsf(b.x-a.x) * ( ((rounding_corners&(1|2))==(1|2)) || ((rounding_corners&(4|8))==(4|8)) ? 0.5f : 1.0f ) - 1.0f);
    r = ImMin(r, fabsf(b.y-a.y) * ( ((rounding_corners&(1|8))==(1|8)) || ((rounding_corners&(2|4))==(2|4)) ? 0.5f : 1.0f ) - 1.0f);

    if (r <= 0.0f || rounding_corners == 0)
    {
        if (edges & EDGE_TOP) {
            dl->PathLineTo(a);
            dl->PathLineTo(ImVec2(b.x, a.y));
            dl->PathStroke(color, false, 1.0f);
        } if (edges & EDGE_RIGHT) {
            dl->PathLineTo(ImVec2(b.x, a.y));
            dl->PathStroke(color, false, 1.0f);
            dl->PathLineTo(b);
        } if (edges & EDGE_BOTTOM){
            dl->PathLineTo(b);
            dl->PathLineTo(ImVec2(a.x,b.y));
            dl->PathStroke(color, false, 1.0f);
        } if (edges & EDGE_LEFT){
            dl->PathLineTo(ImVec2(a.x,b.y));
            dl->PathLineTo(a);
            dl->PathStroke(color, false, 1.0f);
        }
    }
    else
    {
        const float r0 = (rounding_corners & 1) ? r : 0.0f;
        const float r1 = (rounding_corners & 2) ? r : 0.0f;
        const float r2 = (rounding_corners & 4) ? r : 0.0f;
        const float r3 = (rounding_corners & 8) ? r : 0.0f;
        if (edges & EDGE_TOP) {
            dl->PathArcToFast(ImVec2(a.x+r0,a.y+r0), r0, 6, 9);
            dl->PathArcToFast(ImVec2(b.x-r1,a.y+r1), r1, 9, 12);
            dl->PathStroke(color, false, 1.0f);
        } if (edges & EDGE_RIGHT) {
            dl->PathArcToFast(ImVec2(b.x-r1,a.y+r1), r1, 9, 12);
            dl->PathArcToFast(ImVec2(b.x-r2,b.y-r2), r2, 0, 3);
            dl->PathStroke(color, false, 1.0f);
        } if (edges & EDGE_BOTTOM){
            dl->PathArcToFast(ImVec2(b.x-r2,b.y-r2), r2, 0, 3);
            dl->PathArcToFast(ImVec2(a.x+r3,b.y-r3), r3, 3, 6);
            dl->PathStroke(color, false, 1.0f);
        } if (edges & EDGE_LEFT){
            dl->PathArcToFast(ImVec2(a.x+r3,b.y-r3), r3, 3, 6);
            dl->PathArcToFast(ImVec2(a.x+r0,a.y+r0), r0, 6, 9);
            dl->PathStroke(color, false, 1.0f);
        }
        if (shadow){
            const ImColor col = ImColor(0.0f,0.0f,0.0f,shadow_alpha);
            if (shadow_edges & EDGE_LEFT){
                dl->PathArcToFast(ImVec2(a.x+r3-shadow_offset,b.y-r3), r3, 3, 6);
                dl->PathArcToFast(ImVec2(a.x+r0-shadow_offset,a.y+r0), r0, 6, 9);
                dl->PathLineTo(ImVec2(a.x,b.y));
                dl->PathFillConvex(col);
            } if (shadow_edges & EDGE_RIGHT){
                dl->PathArcToFast(ImVec2(b.x-r1+shadow_offset,a.y+r1), r1, 9, 12);
                dl->PathArcToFast(ImVec2(b.x-r2+shadow_offset,b.y-r2), r2, 0, 3);
                dl->PathLineTo(ImVec2(b.x,b.y));
                dl->PathFillConvex(col);
            }
        }
    }
}

ImGui::TabBar::TabBar(const char *label, const ImVec2 tab_bar_size) {
    hash = ImHash(label,0);
    size = tab_bar_size;
}

void ImGui::TabBar::setActiveTab(const unsigned idx) {
    activeTab = (int)idx; // This get's checked at the beginning of
}

const int ImGui::TabBar::getActiveTab() {
    return activeTab;
}

void ImGui::TabBar::_drawTabBarTop(const char *label) {
    if (!hasBeenInitialized)
        return;
    // Gather pointers and references
    ImGuiWindow* wind = ImGui::GetCurrentWindow();
    ImDrawList* dl = wind->DrawList;
    ImGuiDrawContext& dc = wind->DC;
    ImGuiStyle& style = ImGui::GetStyle();
    std::string str = label;

    barTitle = str.substr(0,str.find_first_of("#"));
    if (barTitle.length()>0)
        ImGui::Text(std::string("\t"+barTitle).c_str(), "%s");

    // Get some geometric data
    const ImVec2 padding = style.WindowPadding;
    const ImVec2 frame_padding = style.FramePadding;
    ImVec2 pos = dc.CursorPos;
    const float division = ((wind->Size.x - 2*padding.x)/(float)tabCount);

    // Compile-time constants
    static constexpr const float rounding = 6.0f;
    // This is the border shrink in px. Not sure why it needs to be 1, but it works. Adjust to your border size
    static constexpr const float shrink = 1.0f;

    const float tab_height = CalcTextSize(tabTitles[0]).y + (frame_padding.y * 2);

    float selected_offset = 0;
    _TabType selected_tab_type;
    ImVec2 selected_expands;
    _EdgeType selected_shadow_edges = EDGE_NONE;
    int selected_idx = 0;
    // Store maximum/minimum x value for clipping rect.
    const float max_x = wind->Pos.x + wind->Size.x - padding.x; // Used for clipping rect adjustment.
    const float min_x = wind->Pos.x + padding.x;

    if (activeTab > tabCount-1)
        activeTab = tabCount-1;

    // Idx of the selected tab
    newSelected = -1;

    // Draw the tabs
    for (int i = 0; i<tabCount; i++) {


        // Calculate offset
        const float offs = i * division;
        const bool selected = (activeTab == i);
        ImVec2 expands;
        if (!selected) {
            if (i == 0) {
                expands = ImVec2(0,2);
            } else if (i == tabCount - 1) {
                expands = ImVec2(1,0);
            } else {
                expands = ImVec2(1,2);
            }

            const float xl = offs - expands.x + shrink;
            const float xr = offs + division - shrink + expands.y;
            const ImRect bb = ImRect(ImVec2(pos + ImVec2(xl, 0)),ImVec2( pos + ImVec2(xr, tab_height)));

            bool hovered, held;
            bool pressed = ButtonBehavior(bb, tabHashes[i], &hovered, &held);
            if (held)
                newSelected = i;


            ImU32 col;
            hovered ? col = GetColorU32(ImGuiUserCol_TabHover) : col = GetColorU32(ImGuiUserCol_TabNormal);

            // Draw background rect
            dl->AddRectFilled(bb.Min, bb.Max, col, rounding,
                              ImGuiCorner_TopLeft | ImGuiCorner_TopRight);
            // Draw the border
            _drawPartialRect(bb.Min, bb.Max,rounding, ImGuiCorner_TopLeft | ImGuiCorner_TopRight, dl,
                             (_EdgeType) (EDGE_LEFT | EDGE_RIGHT | EDGE_TOP), GetColorU32(ImGuiUserCol_TabBorderShadow));
            // Draw the text
            const ImVec2 text_size = CalcTextSize(tabTitles[i]);
            const ImVec2 text_pos = pos + ImVec2(offs+((xr-xl) - text_size.x)/2.0f,((text_size.y - frame_padding.y*2.0f)/2.0f));
            dl->AddText(text_pos,GetColorU32(ImGuiUserCol_TabTitleTextNormal),tabTitles[i]);


        } else{
            selected_offset = offs;
            selected_idx = i;
            if (i == 0) {
                selected_expands = ImVec2(0,rounding);
                selected_shadow_edges = EDGE_RIGHT;
            } else if (i == tabCount - 1) {
                selected_expands = ImVec2(rounding,0);
                selected_shadow_edges = EDGE_LEFT;
            } else {
                selected_expands = ImVec2(rounding,rounding);
                selected_shadow_edges = (_EdgeType)(EDGE_RIGHT | EDGE_LEFT);
            }

        }
    }
    const float xl = selected_offset - selected_expands.x + shrink;
    const float xr = selected_offset + division - shrink + selected_expands.y;
    const ImRect bb = ImRect(ImVec2(pos + ImVec2(xl, 0)),ImVec2( pos + ImVec2(xr, tab_height)));
    // Draw the selected tab on top of everything else
    dl->AddRectFilled(bb.Min, bb.Max, GetColorU32(ImGuiUserCol_TabNormal), rounding, ImGuiCorner_TopLeft | ImGuiCorner_TopRight);

    dl->AddRectFilled(bb.Min, bb.Max, ImColor(1.0f, 1.0f, 1.0f, 0.35f), rounding, ImGuiCorner_TopLeft | ImGuiCorner_TopRight);
    // Draw the border
    _drawPartialRect(bb.Min, bb.Max,rounding, ImGuiCorner_TopLeft | ImGuiCorner_TopRight, dl,
                     (_EdgeType) (EDGE_LEFT | EDGE_RIGHT | EDGE_TOP), GetColorU32(ImGuiUserCol_TabBorderShadow),
                     true,selected_shadow_edges);
    // Draw the text
    const ImVec2 text_size = CalcTextSize(tabTitles[selected_idx]);
    const ImVec2 text_pos = pos + ImVec2(selected_offset+((xr-xl) - text_size.x)/2.0f - selected_expands.x,((text_size.y - frame_padding.y*2.0f)/2.0f));
    dl->AddText(text_pos+ImVec2(1,1),GetColorU32(ImGuiUserCol_TabTitleTextNormal),tabTitles[selected_idx]); // Shadow
    dl->AddText(text_pos,GetColorU32(ImGuiUserCol_TabTitleTextSelected),tabTitles[selected_idx]);

    dc.CursorPos += ImVec2(0,tab_height + padding.y); // Add all the extra height used above.
    upperLeft = dc.CursorPos - ImVec2(0,padding.y);
    Indent(padding.x);
    corner_rounding = rounding;
    ImGui::PushClipRect(ImVec2(min_x,bb.Max.y),ImVec2(max_x,wind->ClipRect.Max.y),false);
}

void ImGui::TabBar::_drawTabBarBottom() {
    ImGuiWindow* wind = ImGui::GetCurrentWindow();
    ImDrawList* dl = wind->DrawList;
    ImGuiStyle& style = ImGui::GetStyle();
    ImGuiDrawContext& dc = wind->DC;

    const ImVec2 padding = style.WindowPadding;

    // dc.CursorPos.y = bottom of tabs + padding.y
    // wind->Size.y gives the full window height
    // the zero below the tabs is dc.CursorPos.y - padding.y
    // and if we wnt to add the tab height, we'll have to remove a constant
    // (= all the additional offsets used for the layout). FIXME : use the real name(s) instead
    const float height = dc.CursorPos.y - padding.y + wind->Size.y - 92.0f;
    const ImVec2 pos = ImVec2(wind->Pos.x + wind->Size.x - padding.x, height);

    // Draw the background in a given color + alpha
    dl->AddRectFilled(upperLeft, pos,ImColor(1.0f,1.0f,1.0f,0.15f),corner_rounding,ImGuiCorner_BotLeft | ImGuiCorner_BotRight);
    // Draw the border in a given color + alpha
    dl->AddRect(upperLeft, pos,ImColor(1.0f,1.0f,1.0f,0.35f),corner_rounding,ImGuiCorner_BotLeft | ImGuiCorner_TopRight);

    dc.CursorPos += ImVec2(0,corner_rounding+2.0f); // Add all the extra height used above.
    ImGui::PopClipRect();
}

void ImGui::TabBar::_setTabCount() {
    if (newSelected >= 0 ){
        activeTab = newSelected;
    }

    // If the tab count changes, reinitialize the list.
    if( hasBeenInitialized  && tabCount != idxCounter){
        hasBeenInitialized = false;
        tabTitles.clear();
        tabHashes.clear();
        return;
    }
    tabCount = idxCounter;
    hasBeenInitialized = true;
}

ImGui::TabBar* ImGui::TabBarStack::getCurrentTabBar() {
    if (CurrentTabBar > TabBarCount){
        std::cerr << "TabBar (Error @getCurrentTabBar()) -- CurrentTabBar > TabBarCount" << std::endl;
        return NULL;
    }
    return &TabBars[CurrentTabBar];
}

void ImGui::TabBarStack::clearIdxCountCurrentTabBar() {
    TabBars[CurrentTabBar].idxCounter = 0;
}

const bool ImGui::TabBarStack::doesTabBarExist(const ImU32 hash, unsigned int *const idx) {
    for (unsigned i = 0; i < TabBarCount; i++)
        if (hash == TabBars[i].hash) {
            if (idx)
                *idx = i;
            return true;
        }
    return false;
}

const bool ImGui::TabBarStack::doesTabBarExist(const char *id, unsigned int *const idx) {
    const ImU32 hash = ImHash(id,0);
    for (unsigned i = 0; i < TabBarCount; i++)
        if (hash == TabBars[i].hash) {
            if (idx)
                *idx = i;
            return true;
        }

    *idx = 0;
    return false;
}

void ImGui::BeginTabBar(const char *label, const ImVec2 size) {
    // Pass this to doesTabBarExist() to get the index of the started TabBar
    unsigned idx;
    // If a tab bar doesn't exist, let's create one.
    if (!TabStack.doesTabBarExist(label, &idx)){
        TabBar bar = TabBar(label,size);
        TabStack.TabBars.push_back(bar);
        TabStack.CurrentTabBar = TabStack.TabBarCount;
        TabStack.TabBarCount++;
    } else { // Set the current active tab bar
        TabStack.CurrentTabBar = idx;
        TabStack.clearIdxCountCurrentTabBar();
    }
    TabStack.getCurrentTabBar()->_drawTabBarTop(label);
}

const bool ImGui::AddTab(const char *title) {
    // Get current TabBar;
    TabBar* bar = TabStack.getCurrentTabBar();
    // Check to make sure there aren't any null pointers
    assert(bar!=NULL);

    bar->idxCounter++; // Always increase the counter;

    if(bar->idxCounter > bar->tabCount)
        bar->hasBeenInitialized = false;

    if (!bar->hasBeenInitialized) {
        bar->tabTitles.push_back(title);
        bar->tabHashes.push_back(ImHash(std::string(title + std::to_string(bar->idxCounter)).c_str(),0));
    }

    if (bar->activeTab != (bar->idxCounter-1))
        return false;

    return bar->hasBeenInitialized;
}

void ImGui::DrawTabsBackground()
{
    TabBar* bar = TabStack.getCurrentTabBar();
    if (bar->hasBeenInitialized)
        bar->_drawTabBarBottom();
}

void ImGui::EndTabBar() {
    TabBar* bar = TabStack.getCurrentTabBar();
    bar->_setTabCount();
}

/* FIXME : unused ?
void ImGui::SetActiveTabOfCurrentTabBar(const unsigned idx) {
    TabStack.getCurrentTabBar()->setActiveTab(idx);
}
*/

