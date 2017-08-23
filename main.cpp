// ImGui - standalone example application for SDL2 + OpenGL
// If you are new to ImGui, see examples/README.txt and documentation at the top of imgui.cpp.

#include <imgui.h>
#include "imgui_impl_sdl_gl3.h"
#include <stdio.h>
#include <GL/glew.h>    // This example is using gl3w to access OpenGL functions (because it is small). You may use glew/glad/glLoadGen/etc. whatever already works for you.
#include <SDL.h>

#include "voronoi_main.hpp"
#include <boost/random/mersenne_twister.hpp>

#include <boost/polygon/polygon.hpp>
#include <boost/polygon/voronoi.hpp>
typedef boost::polygon::default_voronoi_builder VB_BOOST;
typedef boost::polygon::voronoi_diagram<double> VD_BOOST;

using namespace boost::polygon;

const int RANDOM_SEED = 27;

typedef boost::int32_t int32;

class MapGenerator {
public:
  void draw_points() {
    // Draw input points and endpoints of the input segments.
    glColor3f(0.0f, 0.5f, 1.0f);
    glPointSize(9);
    glBegin(GL_POINTS);
    for (std::size_t i = 0; i < points.size(); ++i) {
      point_type point = points[i];
      deconvolve(point, shift_);
      glVertex2f(point.x(), point.y());
    }
    for (std::size_t i = 0; i < segments.size(); ++i) {
      point_type lp = low(segments[i]);
      lp = deconvolve(lp, shift_);
      glVertex2f(lp.x(), lp.y());
      point_type hp = high(segments[i]);
      hp = deconvolve(hp, shift_);
      glVertex2f(hp.x(), hp.y());
    }
    glEnd();
  }

  void draw_segments() {
    // Draw input segments.
    glColor3f(0.0f, 0.5f, 1.0f);
    glLineWidth(2.7f);
    glBegin(GL_LINES);
    for (std::size_t i = 0; i < segments.size(); ++i) {
      point_type lp = low(segments[i]);
      lp = deconvolve(lp, shift_);
      glVertex2f(lp.x(), lp.y());
      point_type hp = high(segments[i]);
      hp = deconvolve(hp, shift_);
      glVertex2f(hp.x(), hp.y());
    }
    glEnd();
  }

  void draw_vertices() {
    // Draw voronoi vertices.
    glColor3f(0.0f, 0.0f, 0.0f);
    glPointSize(6);
    glBegin(GL_POINTS);
    for (const_vertex_iterator it = vd.vertices().begin();
        it != vd.vertices().end(); ++it) {
      if (internal_edges_only_ && (it->color() == EXTERNAL_COLOR)) {
        continue;
      }
      point_type vertex(it->x(), it->y());
      vertex = deconvolve(vertex, shift_);
      glVertex2f(vertex.x(), vertex.y());
    }
    glEnd();
  }

  void draw_edges() {
    // Draw voronoi edges.
    glColor3f(0.0f, 0.0f, 0.0f);
    glLineWidth(1.7f);
    for (const_edge_iterator it = vd.edges().begin();
        it != vd.edges().end(); ++it) {
      if (primary_edges_only_ && !it->is_primary()) {
        continue;
      }
      if (internal_edges_only_ && (it->color() == EXTERNAL_COLOR)) {
        continue;
      }
      std::vector<point_type> samples;
      if (!it->is_finite()) {
        clip_infinite_edge(*it, &samples);
      } else {
        point_type vertex0(it->vertex0()->x(), it->vertex0()->y());
        samples.push_back(vertex0);
        point_type vertex1(it->vertex1()->x(), it->vertex1()->y());
        samples.push_back(vertex1);
        if (it->is_curved()) {
          sample_curved_edge(*it, &samples);
        }
      }
      glBegin(GL_LINE_STRIP);
      for (std::size_t i = 0; i < samples.size(); ++i) {
        point_type vertex = deconvolve(samples[i], shift_);
        glVertex2f(vertex.x(), vertex.y());
      }
      glEnd();
    }
  }

  void construct_brect() {
    double side = (std::max)(xh(brect_) - xl(brect_), yh(brect_) - yl(brect_));
    center(shift_, brect_);
    set_points(brect_, shift_, shift_);
    bloat(brect_, side * 1.2);
  }

  void build() {
    boost::mt19937 gen(RANDOM_SEED);
    VB_BOOST vb;
    VD_BOOST vd;
    for (int k = 0; k < 100; ++k)
      vb.insert_point(static_cast<int32>(gen()), static_cast<int32>(gen()));
    vb.construct(&vd);

    construct_brect();

    // Construct voronoi diagram.
    // construct_voronoi(
    //                   points.begin(), points.end(),
    //                   segments.begin(), segments.end(),
    //                   &vd);

    // Color exterior edges.
    for (const_edge_iterator it = vd.edges().begin();
         it != vd.edges().end(); ++it) {
      if (!it->is_finite()) {
        color_exterior(&(*it));
      }
    }
  }

private:
  typedef double coordinate_type;
  typedef point_data<coordinate_type> point_type;
  typedef segment_data<coordinate_type> segment_type;
  typedef rectangle_data<coordinate_type> rect_type;
  typedef voronoi_builder<int> VB;
  typedef voronoi_diagram<coordinate_type> VD;
  typedef VD::cell_type cell_type;
  typedef VD::cell_type::source_index_type source_index_type;
  typedef VD::cell_type::source_category_type source_category_type;
  typedef VD::edge_type edge_type;
  typedef VD::cell_container_type cell_container_type;
  typedef VD::cell_container_type vertex_container_type;
  typedef VD::edge_container_type edge_container_type;
  typedef VD::const_cell_iterator const_cell_iterator;
  typedef VD::const_vertex_iterator const_vertex_iterator;
  typedef VD::const_edge_iterator const_edge_iterator;

  static const std::size_t EXTERNAL_COLOR = 1;

  void clip_infinite_edge(
      const edge_type& edge, std::vector<point_type>* clipped_edge) {
    const cell_type& cell1 = *edge.cell();
    const cell_type& cell2 = *edge.twin()->cell();
    point_type origin, direction;
    // Infinite edges could not be created by two segment sites.
    if (cell1.contains_point() && cell2.contains_point()) {
      point_type p1 = retrieve_point(cell1);
      point_type p2 = retrieve_point(cell2);
      origin.x((p1.x() + p2.x()) * 0.5);
      origin.y((p1.y() + p2.y()) * 0.5);
      direction.x(p1.y() - p2.y());
      direction.y(p2.x() - p1.x());
    } else {
      origin = cell1.contains_segment() ?
          retrieve_point(cell2) :
          retrieve_point(cell1);
      segment_type segment = cell1.contains_segment() ?
          retrieve_segment(cell1) :
          retrieve_segment(cell2);
      coordinate_type dx = high(segment).x() - low(segment).x();
      coordinate_type dy = high(segment).y() - low(segment).y();
      if ((low(segment) == origin) ^ cell1.contains_point()) {
        direction.x(dy);
        direction.y(-dx);
      } else {
        direction.x(-dy);
        direction.y(dx);
      }
    }
    coordinate_type side = xh(brect_) - xl(brect_);
    coordinate_type koef =
        side / (std::max)(fabs(direction.x()), fabs(direction.y()));
    if (edge.vertex0() == NULL) {
      clipped_edge->push_back(point_type(
          origin.x() - direction.x() * koef,
          origin.y() - direction.y() * koef));
    } else {
      clipped_edge->push_back(
          point_type(edge.vertex0()->x(), edge.vertex0()->y()));
    }
    if (edge.vertex1() == NULL) {
      clipped_edge->push_back(point_type(
          origin.x() + direction.x() * koef,
          origin.y() + direction.y() * koef));
    } else {
      clipped_edge->push_back(
          point_type(edge.vertex1()->x(), edge.vertex1()->y()));
    }
  }

  void sample_curved_edge(
      const edge_type& edge,
      std::vector<point_type>* sampled_edge) {
    coordinate_type max_dist = 1E-3 * (xh(brect_) - xl(brect_));
    point_type point = edge.cell()->contains_point() ?
        retrieve_point(*edge.cell()) :
        retrieve_point(*edge.twin()->cell());
    segment_type segment = edge.cell()->contains_point() ?
        retrieve_segment(*edge.twin()->cell()) :
        retrieve_segment(*edge.cell());
    voronoi_visual_utils<coordinate_type>::discretize(
        point, segment, max_dist, sampled_edge);
  }

  point_type retrieve_point(const cell_type& cell) {
    source_index_type index = cell.source_index();
    source_category_type category = cell.source_category();
    if (category == SOURCE_CATEGORY_SINGLE_POINT) {
      return points[index];
    }
    index -= points.size();
    if (category == SOURCE_CATEGORY_SEGMENT_START_POINT) {
      return low(segments[index]);
    } else {
      return high(segments[index]);
    }
  }

  segment_type retrieve_segment(const cell_type& cell) {
    source_index_type index = cell.source_index() - points.size();
    return segments[index];
  }

  point_type shift_;
  std::vector<point_type> points;
  std::vector<segment_type> segments;
  rect_type brect_;
  VB vb_;
  VD vd;
  bool brect_initialized_;
  bool primary_edges_only_;
  bool internal_edges_only_;

  void color_exterior(const VD::edge_type* edge) {
    if (edge->color() == EXTERNAL_COLOR) {
      return;
    }
    edge->color(EXTERNAL_COLOR);
    edge->twin()->color(EXTERNAL_COLOR);
    const VD::vertex_type* v = edge->vertex1();
    if (v == NULL || !edge->is_primary()) {
      return;
    }
    v->color(EXTERNAL_COLOR);
    const VD::edge_type* e = v->incident_edge();
    do {
      color_exterior(e);
      e = e->rot_next();
    } while (e != v->incident_edge());
  }

};

MapGenerator* mg;

int main(int, char**)
{
    // Setup SDL
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }
    mg = new MapGenerator();
    mg->build();

    // Setup window
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_DisplayMode current;
    SDL_GetCurrentDisplayMode(0, &current);
    SDL_Window *window = SDL_CreateWindow("ImGui SDL2+OpenGL3 example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);
    SDL_GLContext glcontext = SDL_GL_CreateContext(window);
    glewInit();

    // Setup ImGui binding
    ImGui_ImplSdlGL3_Init(window);

    // Load Fonts
    // (there is a default font, this is only if you want to change it. see extra_fonts/README.txt for more details)
    // ImGuiIO& io = ImGui::GetIO();
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("../../extra_fonts/Cousine-Regular.ttf", 15.0f);
    // io.Fonts->AddFontFromFileTTF("imgui/extra_fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../extra_fonts/ProggyClean.ttf", 13.0f);
    // io.Fonts->AddFontFromFileTTF("imgui/extra_fonts/ProggyTiny.ttf", 10.0f);
    // io.Fonts->AddFontFromFileTTF("imgui/extra_fonts/DroidSans.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());

    bool show_test_window = false;
    bool show_another_window = false;
    ImVec4 clear_color = ImColor(114, 144, 154);

    // std::vector<Point> points;
    // points.push_back(Point(0, 0));
    // points.push_back(Point(1, 6));
    // std::vector<Segment> segments;
    // segments.push_back(Segment(-4, 5, 5, -1));
    // segments.push_back(Segment(3, -11, 13, -1));

    // construct_brect();
    // voronoi_diagram<double> vd;
    // construct_voronoi(points.begin(), points.end(),
    //                   segments.begin(), segments.end(),
    //                   &vd);

    // Main loop
    bool done = false;
    while (!done)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSdlGL3_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
        }
        ImGui_ImplSdlGL3_NewFrame(window);

        // 1. Show a simple window
        // Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears in a window automatically called "Debug"
        {
            static float f = 0.0f;
            ImGui::Text("Hello, world!");
            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
            ImGui::ColorEdit3("clear color", (float*)&clear_color);
            if (ImGui::Button("Test Window")) show_test_window ^= 1;
            if (ImGui::Button("Another Window")) show_another_window ^= 1;
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        }

        // 2. Show another simple window, this time using an explicit Begin/End pair
        if (show_another_window)
        {
            ImGui::SetNextWindowSize(ImVec2(200,100), ImGuiCond_FirstUseEver);
            ImGui::Begin("Another Window", &show_another_window);
            ImGui::Text("Hello");
            ImGui::End();
        }

        // 3. Show the ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
        // if (show_test_window)
        // {
        //     ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver);
        //     ImGui::ShowTestWindow(&show_test_window);
        // }

        // Rendering
        glViewport(0, 0, (int)ImGui::GetIO().DisplaySize.x, (int)ImGui::GetIO().DisplaySize.y);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);





        ImGui::Render();
        SDL_GL_SwapWindow(window);
    }

    // Cleanup
    ImGui_ImplSdlGL3_Shutdown();
    SDL_GL_DeleteContext(glcontext);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
