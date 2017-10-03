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
// File Description: Custom ImGui tab system module added by me (Scott)
/*
 * Usage:
 *      * To start a tab bar, use: ImGui::BeginTabBar(const char* label)
 *          -> The label can be blank if you want, but you MUST!! pass a string of some length. Use the '#' character to
 *              give it an ID without a label.
 *
 *      * Add tabs by using:
 *          if (ImGui::AddTab("Tab Name")){
 *                 < Insert other draw commands here >
 *          }
 *          -> This is much like collapsing header. If the tab is active, then AddTab will return true, allowing drawing
 *              to occur.
 *
 *      * End the tab bar by using ImGui::EndTabBar()
 *
 *      And that's it! You can dynamically add or remove tabs, the bar will reset the ID and tab label vectors appropriately,
 *      but it DOESN'T continuously allocate/deallocate them in memory every time (unless the CRC32 hashes of the tab or tab bar change,
 *      computed from their respective labels).
 *
 *      Notes:
 *          * You can change the rounding by altering:
 *                  "static constexpr const float rounding = 6.0f;"
 *             in the _drawTabBarTop function. Alternatively, you can add it as a parameter, but to save on memory being passed
 *             around, and since I will be using 6 as a constant, I didn't include this.
 *
 *          * The constexpr:
 *                  "static constexpr const float shrink = 1.0f;"
 *             in the _drawTabBarTop function needs to be set to the width of your borders.
 *
 *          * I added custom colors and a new prototype for GetColorU32() to acquire these colors. You can change them here.
 *
 *          * I added extra operator functions for ImVec2 and ImVec4, _YOU MUST_ define IMGUI_DEFINE_MATH_OPERATORS in
 *             imgui_internal.h for this to work.
 *
 *          * Please attribute my work if used or added, along with the original ImGui license/attribution.
 */


#ifndef IMGUI_TABS_HPP
#define IMGUI_TABS_HPP

#include <vector>
#include <string>
#include <iostream>
#include "imgui.h"
#include "imgui_internal.h"


// Extra Math Helpers (Set the proper define below in imgui_internal.h)
#ifdef IMGUI_DEFINE_MATH_OPERATORS
static inline ImVec4 operator+(const ImVec4& lhs, const ImVec4& rhs)            { return ImVec4(lhs.x+rhs.x, lhs.y+rhs.y, lhs.z+rhs.z, lhs.w+rhs.w); }
static inline ImVec4 operator*(const ImVec4& lhs, const ImVec4& rhs)            { return ImVec4(lhs.x*rhs.x, lhs.y*rhs.y, lhs.z*rhs.z, lhs.w*rhs.w); }
static inline ImVec4 operator/(const ImVec4& lhs, const ImVec4& rhs)            { return ImVec4(lhs.x/rhs.x, lhs.y/rhs.y, lhs.z/rhs.z, lhs.w/rhs.w); }

static inline ImVec4& operator+=(ImVec4& lhs, const ImVec4& rhs)            { lhs.x += rhs.x; lhs.y += rhs.y; lhs.z += rhs.z; lhs.w += rhs.w; return lhs;}
static inline ImVec4& operator-=(ImVec4& lhs, const ImVec4& rhs)            { lhs.x -= rhs.x; lhs.y -= rhs.y; lhs.z -= rhs.z; lhs.w -= rhs.w; return lhs;}
static inline ImVec4& operator*=(ImVec4& lhs, const ImVec4& rhs)            { lhs.x *= rhs.x; lhs.y *= rhs.y; lhs.z *= rhs.z; lhs.w *= rhs.w; return lhs;}
static inline ImVec4& operator/=(ImVec4& lhs, const ImVec4& rhs)            { lhs.x /= rhs.x; lhs.y /= rhs.y; lhs.z /= rhs.z; lhs.w /= rhs.w; return lhs;}
#endif

namespace ImGui
{
    /// User Colors & Style Extensions
    enum ImGuiUserCol_ {
        ImGuiUserCol_TabBorder = 0,
        ImGuiUserCol_TabBorderShadow,
        ImGuiUserCol_TabNormal,
        ImGuiUserCol_TabHover,
        ImGuiUserCol_TabTitleTextNormal,
        ImGuiUserCol_TabTitleTextSelected,
        ImGuiUserCol_COUNT
    };

    /// Defines our user style attributes that don't fit within the standard ImGui stack
    IMGUI_API struct ImGuiUserStyle
    {
        ImVec4      Colors[ImGuiUserCol_COUNT];
        ImGuiUserStyle();
    };

    IMGUI_API static ImGuiUserStyle UserStyle;

    /// Additional prototype to retrieve user-defined colors in this header (to prevent foward compatilibity conflicts)
    IMGUI_API const ImU32 GetColorU32(ImGuiUserCol_ idx, float alpha_mul = 0.0f);

    ///Bitmask flags for telling _drawPartialRect() what edges to draw
    enum _EdgeType : char {
        EDGE_NONE   = 0,
        EDGE_LEFT   = 1 << 0,
        EDGE_TOP    = 1 << 1,
        EDGE_RIGHT  = 1 << 2,
        EDGE_BOTTOM = 1 << 3
    };

    /// Used internally to draw a rounded rect with the different borders disabled
    static void _drawPartialRect(const ImVec2 a, const ImVec2 b, const float rounding, const int rounding_corners,
                                 ImDrawList* dl, const _EdgeType edges, const ImU32 color, const bool shadow = false,
                                 const _EdgeType shadow_edges = EDGE_NONE, const float shadow_offset = 4.0f,
                                 const float shadow_alpha = 0.075f);

    /// Struct containing TabBar data
    IMGUI_API struct TabBar{

        /// Constructor
        TabBar(const char* label, const ImVec2 tab_bar_size);

        /// Stores the titles of the tabs
        std::vector<const char*> tabTitles;

        /// Stores hash IDs of the tabs
        std::vector<ImGuiID> tabHashes;

        /// Keeps a tally of how many tabs have been added
        int tabCount = 0; // Set to 0 in StartTab

        /// Index defining the active tab
        int activeTab = 0;

        /// Title of the bar. Use #<ID> to omit the title but pass in an ID
        std::string barTitle;

        /// Size
        ImVec2 size = ImVec2(0,0);

        /// Hash of the TabBar
        ImU32 hash = 0x0;

        /// Index counter, cleared every call to "BeginTabBar"
        int idxCounter = 0;

        /// This holds the upper left corner for the final draw sequence
        ImVec2 upperLeft = ImVec2(0,0);

        /// Set to true once the tab bar has been initialized
        bool hasBeenInitialized = false;

        /// Holds the rounding value for padding reasons
        float corner_rounding = 0;

        /// Holds the value of the new selected tab
        int newSelected = -1;

        /// Sets the active tab
        void setActiveTab(const unsigned idx);

        /// Returns the currently active tab
        const int getActiveTab();

        /// Useful enums
        enum _TabType : char {LEFTMOST_TAB, MIDDLE_TAB, RIGHTMOST_TAB};

        /// Used internally
        void _drawTabBarTop(const char* label);

        void _drawTabBarBottom();

        /// Used internally
        void _setTabCount();
    };

    /// Struct containing global TabBar status
    IMGUI_API struct TabBarStack{
        /// Vector containing all of our tab bars
        std::vector <TabBar> TabBars;

        /// The running count of our tab bar. Useful as it's only written to when a new tab bar is generated. Helps prevent useless calls to std::vector<>::size()
        unsigned TabBarCount = 0;

        /// Index pointing to the current tab
        unsigned CurrentTabBar = 0;

        /// Returns a pointer to the current tab bar
        TabBar* getCurrentTabBar();

        /// Clears the counting index for the current tab bar
        void clearIdxCountCurrentTabBar();

        /// Returns true if the tab bar already exists, pass idx if you want a return index value
        const bool doesTabBarExist(const ImU32 hash, unsigned* const idx = NULL);

        /// Returns true if the tab bar already exists (computes hash), pass idx if you want a return index value
        const bool doesTabBarExist(const char* id, unsigned* const idx = NULL);
    };

    /// Static instantiation of the TabBarStack
    IMGUI_API static TabBarStack TabStack;

// FIXME : unused ?
    /// Outwardly accessible way to set the active tab of the current stack.
//    IMGUI_API static void SetActiveTabOfCurrentTabBar(const unsigned idx);

    /// Star the Tab Bar with this function. It creates a tab bar based on the computed CRC32 hash and pushes it into the internal stack if so.
    IMGUI_API void BeginTabBar(const char *label, const ImVec2 size = ImVec2(0, 0));

    /// Returns true when the tab is active. To use, implement with if(ImGui::AddTab(...)){}
    IMGUI_API const bool AddTab(const char* title);

    /// To use, implement with if(ImGui::DrawTabsBackground(...)){} just after ImGui::BeginTabBar();
    IMGUI_API void DrawTabsBackground();

    /// Call this after you are done adding tabs
    IMGUI_API void  EndTabBar();
    
} // namespace ImGui
#endif //IMGUI_TABS_HPP
