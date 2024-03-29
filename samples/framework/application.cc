//============================================================================//
//                                                                            //
// ozz-animation, 3d skeletal animation libraries and tools.                  //
// https://code.google.com/p/ozz-animation/                                   //
//                                                                            //
//----------------------------------------------------------------------------//
//                                                                            //
// Copyright (c) 2012-2015 Guillaume Blanc                                    //
//                                                                            //
// This software is provided 'as-is', without any express or implied          //
// warranty. In no event will the authors be held liable for any damages      //
// arising from the use of this software.                                     //
//                                                                            //
// Permission is granted to anyone to use this software for any purpose,      //
// including commercial applications, and to alter it and redistribute it     //
// freely, subject to the following restrictions:                             //
//                                                                            //
// 1. The origin of this software must not be misrepresented; you must not    //
// claim that you wrote the original software. If you use this software       //
// in a product, an acknowledgment in the product documentation would be      //
// appreciated but is not required.                                           //
//                                                                            //
// 2. Altered source versions must be plainly marked as such, and must not be //
// misrepresented as being the original software.                             //
//                                                                            //
// 3. This notice may not be removed or altered from any source               //
// distribution.                                                              //
//                                                                            //
//============================================================================//

#define OZZ_INCLUDE_PRIVATE_HEADER  // Allows to include private headers.

#include "framework/application.h"

#include <cstdlib>
#include <cassert>
#include <cstring>

#ifdef __APPLE__
#include <unistd.h>
#endif  // __APPLE__

#if EMSCRIPTEN
#include "emscripten.h"
#endif // EMSCRIPTEN

#include "ozz/base/log.h"
#include "ozz/base/maths/box.h"
#include "ozz/base/memory/allocator.h"
#include "ozz/base/io/stream.h"
#include "ozz/options/options.h"
#include "framework/image.h"
#include "framework/renderer.h"
#include "framework/profile.h"
#include "framework/internal/camera.h"
#include "framework/internal/shooter.h"
#include "framework/internal/imgui_impl.h"
#include "framework/internal/renderer_impl.h"

OZZ_OPTIONS_DECLARE_INT(
  max_idle_loops,
  "The maximum number of idle loops the sample application can perform."
  " Application automatically exit when this number of loops is reached."
  " A negative value disables this feature.",
  -1,
  false);

OZZ_OPTIONS_DECLARE_BOOL(
  render,
  "Enables sample redering.",
  true,
  false);

namespace {
// Screen resolution presets.
const ozz::sample::Resolution resolution_presets[] = {
  {640, 360}, {640, 480}, {800, 450}, {800, 600}, {1024, 576},
  {1024, 768}, {1280, 720}, {1280, 800}, {1280, 960}, {1280, 1024},
  {1400, 1050}, {1440, 900}, {1600, 900}, {1600, 1200}, {1680, 1050},
  {1920, 1080}, {1920, 1200}
};
const int kNumPresets = OZZ_ARRAY_SIZE(resolution_presets);
}

// Check resolution argument is within 0 - kNumPresets
static bool ResolutionCheck(const ozz::options::Option& _option,
                            int /*_argc*/) {
  const ozz::options::IntOption& option =
    static_cast<const ozz::options::IntOption&>(_option);
  return option >=0 && option < kNumPresets;
}

OZZ_OPTIONS_DECLARE_INT_FN(
  resolution,
  "Resolution index (0 to 17).",
  5,
  false,
  &ResolutionCheck);

namespace ozz {
namespace sample {
Application* Application::application_ = NULL;

Application::Application()
    : exit_(false),
      freeze_(false),
      fix_update_rate(false),
      fixed_update_rate(60.f),
      time_factor_(1.f),
      last_idle_time_(0.),
      camera_(NULL),
      shooter_(NULL),
      show_help_(false),
      capture_video_(false),
      capture_screenshot_(false),
      renderer_(NULL),
      im_gui_(NULL),
      fps_(memory::default_allocator()->New<Record>(128)),
      update_time_(memory::default_allocator()->New<Record>(128)),
      render_time_(memory::default_allocator()->New<Record>(128)),
      resolution_(resolution_presets[0]) {
#ifndef NDEBUG
  // Assert presets are correctly sorted.
  for (int i = 1; i < kNumPresets; ++i) {
    const Resolution& preset_m1 = resolution_presets[i - 1];
    const Resolution& preset = resolution_presets[i];
    assert(preset.width > preset_m1.width ||
           preset.height > preset_m1.height);
  }
#endif //  NDEBUG
}

Application::~Application() {
  memory::default_allocator()->Delete(fps_);
  memory::default_allocator()->Delete(update_time_);
  memory::default_allocator()->Delete(render_time_);
}

int Application::Run(int _argc, const char** _argv,
                     const char* _version,
                     const char* _title) {
  // Only one application at a time can be ran.
  if (application_) {
    return EXIT_FAILURE;
  }
  application_ = this;

  // Parse command line arguments.
  const char* usage = "Ozz animation sample. See README file for more details.";
  ozz::options::ParseResult result =
    ozz::options::ParseCommandLine(_argc, _argv, _version, usage);
  if (result != ozz::options::kSuccess) {
    exit_ = true;
    return result == ozz::options::kExitSuccess ? EXIT_SUCCESS : EXIT_FAILURE;
  }

  // Fetch initial resolution.
  resolution_ = resolution_presets[OPTIONS_resolution];

#ifdef __APPLE__
  // On OSX, when run from Finder, working path is the root path. This does not
  // allow to load resources from relative path.
  // The workaround is to change the working directory to application directory.
  // The proper solution would probably be to use bundles and load data from
  // resource folder.
  chdir(ozz::options::ParsedExecutablePath().c_str());
#endif  // __APPLE__

  // ( resourcesPath );

  // Initialize help.
  ParseReadme();

  // Open an OpenGL window
  bool success = true;
  if(OPTIONS_render) {

    // Initialize GLFW
    if (!glfwInit()) {
      application_ = NULL;
      return EXIT_FAILURE;
    }

    // Setup GL context.
    const int gl_version_major = 2, gl_version_minor = 0;
    glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, gl_version_major);
    glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, gl_version_minor);
    glfwOpenWindowHint(GLFW_FSAA_SAMPLES, 4);
#ifndef NDEBUG
    glfwOpenWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif  // NDEBUG

    // Initializes rendering before looping.
    if (!glfwOpenWindow(resolution_.width, resolution_.height,
                        8, 8, 8, 8, 32, 0, GLFW_WINDOW)) {
      log::Err() << "Failed to open OpenGL window. Required OpenGL version is "
        << gl_version_major << "." << gl_version_minor << "." << std::endl;
      success = false;
    } else {
      log::Out() << "Successfully opened OpenGL window version \"" <<
        glGetString(GL_VERSION) << "\"." << std::endl;

      // Allocates and initializes internal objects.
      camera_ = memory::default_allocator()->New<internal::Camera>();
      renderer_ = memory::default_allocator()->New<internal::RendererImpl>(camera_);
      success = renderer_->Initialize();

      if (success) {
        shooter_ = memory::default_allocator()->New<internal::Shooter>();
        im_gui_ = memory::default_allocator()->New<internal::ImGuiImpl>();

        // Setup the window and installs callbacks.
        glfwSwapInterval(1);  // Enables vertical sync by default.
        glfwSetWindowTitle(_title);
        glfwSetWindowSizeCallback(&ResizeCbk);
        glfwSetWindowCloseCallback(&CloseCbk);

        // Loop the sample.
        success = Loop();

        memory::default_allocator()->Delete(shooter_);
        shooter_ = NULL;
        memory::default_allocator()->Delete(im_gui_);
        im_gui_ = NULL;
      }
      memory::default_allocator()->Delete(renderer_);
      renderer_ = NULL;
      memory::default_allocator()->Delete(camera_);
      camera_ = NULL;
    }

    // Closes window and terminates GLFW.
    glfwTerminate();

  } else {
    // Loops without any rendering initialization.
    success = Loop();
  }

  // Notifies that an error occurred.
  if (!success) {
    log::Err() << "An error occurred during sample execution." <<
      std::endl;
  }

  application_ = NULL;

  return success ? EXIT_SUCCESS : EXIT_FAILURE;
}

void OneLoopCbk(void* _arg) {
  Application* app = reinterpret_cast<Application*>(_arg);
  static int loops = 0;
  app->OneLoop(loops++);
}

Application::LoopStatus Application::OneLoop(int _loops) {

  Profiler profile(fps_);  // Profiles frame.

  // Tests for a manual exit request.
  if (exit_ || glfwGetKey(GLFW_KEY_ESC) == GLFW_PRESS) {
    return kBreak;
  }

  // Test for an exit request.
  if (OPTIONS_max_idle_loops > 0 && _loops > OPTIONS_max_idle_loops) {
    return kBreak;
  }

  // Don't overload the cpu if the window is not active.
  if (OPTIONS_render && !glfwGetWindowParam(GLFW_ACTIVE)) {
    glfwWaitEvents();  // Wait...

    // Reset last update time in order to stop the time while the app isn't
    // active.
    last_idle_time_ = glfwGetTime();

    return kContinue;  // ...but don't do anything.
  }

  // Updates resolution if required.
  if (OPTIONS_render) {
    int width, height;
    glfwGetWindowSize(&width, &height);
    if (resolution_.width != width || resolution_.height !=height) {
      glfwSetWindowSize(resolution_.width, resolution_.height);
    }
  }

  // Enable/disable help on F1 key.
  static int previous_f1 = glfwGetKey(GLFW_KEY_F1);
  const int f1 = glfwGetKey(GLFW_KEY_F1);
  if (previous_f1 == GLFW_RELEASE && f1 == GLFW_PRESS) {
    show_help_ = !show_help_;
  }
  previous_f1 = f1;

  // Do the main loop.
  if (!Idle(_loops == 0)) {
    return kBreakFailure;
  }

  // Skips display if "no_render" option is enabled.
  if (OPTIONS_render) {
    if (!Display()) {
      return kBreakFailure;
    }
  }

  return kContinue;
}

bool Application::Loop() {
  // Initialize sample.
  bool success = OnInitialize();

  // Emscripten requires to manage the main loop on their own, as browsers don't
  // like infinite blocking functions.
#ifdef EMSCRIPTEN
  emscripten_set_main_loop_arg(OneLoopCbk, this, 0, 1);
#else // EMSCRIPTEN
  // Loops.
  for (int loops = 0; success; ++loops) {
    const LoopStatus status = OneLoop(loops);
    success = status != kBreakFailure;
    if (status != kContinue) {
      break;
    }
  }
#endif // EMSCRIPTEN

  // De-initialize sample, even in case of initialization failure.
  OnDestroy();

  return success;
}

bool Application::Display() {
  assert(OPTIONS_render);

  bool success = true;

  { // Profiles rendering excluding GUI.
    Profiler profile(render_time_);

    GL(ClearColor(.33f, .333f, .315f, 0.f));
    GL(Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    // Setup default states
    GL(Enable(GL_CULL_FACE));
    GL(CullFace(GL_BACK));
    GL(Enable(GL_DEPTH_TEST));
    GL(DepthMask(GL_TRUE));
    GL(DepthFunc(GL_LEQUAL));

    // Bind 3D camera matrices.
    camera_->Bind3D();

    // Forwards display event to the inheriting application.
    if (success) {
      success = OnDisplay(renderer_);
    }
  }  // Ends profiling.

  // Renders grid and axes at the end as they are transparent.
  GL(DepthMask(GL_FALSE));
  renderer_->DrawGrid(10, 1.f);
  GL(DepthMask(GL_TRUE));
  renderer_->DrawAxes(ozz::math::Float4x4::identity());

  // Bind 2D camera matrices.
  camera_->Bind2D();

  // Forwards gui event to the inheriting application.
  if (success) {
    success = Gui();
  }

  // Capture back buffer.
  if (capture_screenshot_ || capture_video_) {
    shooter_->Capture(GL_BACK);
  }

  // Swaps current window.
  glfwSwapBuffers();

  return success;
}

bool Application::Idle(bool _first_frame) {

  // Early out if displaying help.
  if (show_help_) {
    last_idle_time_ = glfwGetTime();
    return true;
  }

  // Compute elapsed time since last idle.
  double time = glfwGetTime();
  if (time == 0.) {  // Means glfw isn't initialized (rendering's disabled).
    time = last_idle_time_ + 1. / 60.;
  }

  // Real time dt.
  const float delta = static_cast<float>(time - last_idle_time_);
  last_idle_time_ = time;

  // Update dt, can be scaled, fixed, freezed...
  float update_delta;
  if (freeze_) {
    update_delta = 0.f;
  } else {
    if (fix_update_rate) {
      update_delta = 1.f / fixed_update_rate;
    } else {
      update_delta = delta * time_factor_;
    }
  }

  // Updates screen shooter object.
  if (shooter_) {
    shooter_->Update();
  }

  // Forwards update event to the inheriting application.
  bool update_result;
  {  // Profiles update scope.
    Profiler profile(update_time_);
    update_result = OnUpdate(update_delta);
  }

  // Update camera model-view matrix and disables auto-framing if the camera is
  // moved manually.
  // Update camera model-view matrix.
  if (camera_) {
    math::Box scene_bounds;
    GetSceneBounds(&scene_bounds);
    camera_->Update(scene_bounds, delta, _first_frame);
  }

  return update_result;
}

bool Application::Gui() {
  bool success = true;
  const float kFormWidth = 200.f;
  const float kHelpMargin = 16.f;

  // Finds gui area.
  const float kGuiMargin = 2.f;
  ozz::math::RectInt window_rect(0, 0, resolution_.width, resolution_.height);

  // Fills ImGui's input structure.
  internal::ImGuiImpl::Inputs input;
  int mouse_y;
  glfwGetMousePos(&input.mouse_x, &mouse_y);
  input.mouse_y = window_rect.height - mouse_y;
  input.lmb_pressed = glfwGetMouseButton(GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

  // Starts frame
  static_cast<internal::ImGuiImpl*>(im_gui_)->BeginFrame(input, window_rect,
                                                         renderer_);

  // Help gui.
  {
    math::RectFloat rect(kGuiMargin,
                         kGuiMargin,
                         window_rect.width - kGuiMargin * 2.f,
                         window_rect.height - kGuiMargin * 2.f);
    ImGui::Form form(im_gui_, "Show help", rect, &show_help_, false);
    if (show_help_) {
      im_gui_->DoLabel(help_.c_str(), ImGui::kLeft, false);
    }
  }
  
  // Do framework gui.
  if (!show_help_ &&
      success &&
      window_rect.width > (kGuiMargin + kFormWidth) * 2.f) {
    static bool open = true;
    math::RectFloat rect(kGuiMargin,
                         kGuiMargin,
                         kFormWidth,
                         window_rect.height - kGuiMargin * 2.f - kHelpMargin);
    ImGui::Form form(im_gui_, "Framework", rect, &open, true);
    if (open) {
      success = FrameworkGui();
    }
  }

  // Do sample gui.
  if (!show_help_ &&
      success &&
      window_rect.width > kGuiMargin + kFormWidth) {
    static bool open = true;
    math::RectFloat rect(window_rect.width - kFormWidth - kGuiMargin,
                         kGuiMargin,
                         kFormWidth,
                         window_rect.height - kGuiMargin * 2 - kHelpMargin);
    ImGui::Form form(im_gui_, "Sample", rect, &open, true);
    if (open) {
      // Forwards event to the inherited application.
      success = OnGui(im_gui_);
    }
  }

  // Ends frame
  static_cast<internal::ImGuiImpl*>(im_gui_)->EndFrame();

  return success;
}

bool Application::FrameworkGui() {
  { // Render statistics
    static bool open = true;
    ImGui::OpenClose stats(im_gui_, "Statistics", &open);
    if (open) {
      char szLabel[64];
      { // FPS
        Record::Statistics statistics = fps_->GetStatistics();
        std::sprintf(szLabel, "FPS: %.0f",
                     statistics.mean == 0.f ? 0.f : 1000.f / statistics.mean);
        static bool fps_open = false;
        ImGui::OpenClose stats(im_gui_, szLabel, &fps_open);
        if (fps_open) {
          std::sprintf(szLabel, "Frame: %.2f ms", statistics.mean);
          im_gui_->DoGraph(
            szLabel, 0.f, statistics.max, statistics.latest,
            fps_->cursor(),
            fps_->record_begin(), fps_->record_end());
        }
      }
      { // Update time
        Record::Statistics statistics = update_time_->GetStatistics();
        std::sprintf(szLabel, "Update: %.2f ms", statistics.mean);
        static bool update_open = true;  // This is the most relevant for ozz.
        ImGui::OpenClose stats(im_gui_, szLabel, &update_open);
        if (update_open) {
          im_gui_->DoGraph(
            NULL, 0.f, statistics.max, statistics.latest,
            update_time_->cursor(),
            update_time_->record_begin(), update_time_->record_end());
        }
      }
      { // Render time
        Record::Statistics statistics = render_time_->GetStatistics();
        std::sprintf(szLabel, "Render: %.2f ms", statistics.mean);
        static bool render_open = false;
        ImGui::OpenClose stats(im_gui_, szLabel, &render_open);
        if (render_open) {
          im_gui_->DoGraph(
            NULL, 0.f, statistics.max, statistics.latest,
            render_time_->cursor(),
            render_time_->record_begin(), render_time_->record_end());
        }
      }
    }
  }

  { // Time control
    static bool open = true;
    ImGui::OpenClose stats(im_gui_, "Time control", &open);
    if (open) {
      im_gui_->DoButton("Freeze", true, &freeze_);
      im_gui_->DoCheckBox("Fix update rate", &fix_update_rate, true);
      if (!fix_update_rate) {
        char sz_factor[64];
        std::sprintf(sz_factor, "Time factor: %.2f", time_factor_);
        im_gui_->DoSlider(sz_factor, -5.f, 5.f, &time_factor_);
        if (im_gui_->DoButton("Reset time factor", time_factor_ != 1.f)) {
          time_factor_ = 1.f;
        }
      } else {
        char sz_fixed_update_rate[64];
        std::sprintf(sz_fixed_update_rate, "Update rate: %.0f fps", fixed_update_rate);
        im_gui_->DoSlider(sz_fixed_update_rate, 1.f, 200.f, &fixed_update_rate, .5f, true);
        if (im_gui_->DoButton("Reset update rate", fixed_update_rate != 60.f)) {
          fixed_update_rate = 60.f;
        }
      }
    }
  }

  { // Rendering options
    static bool open = false;
    ImGui::OpenClose options(im_gui_, "Options", &open);
    if (open) {
      // Multi-sampling.
      static bool fsaa_available = glfwGetWindowParam(GLFW_FSAA_SAMPLES) != 0;
      static bool fsaa_enabled = fsaa_available;
      if (im_gui_->DoCheckBox("Anti-aliasing", &fsaa_enabled, fsaa_available)) {
        if (fsaa_enabled) {
          GL(Enable(GL_MULTISAMPLE));
        } else {
          GL(Disable(GL_MULTISAMPLE));
        }
      }
      // Vertical sync
      static bool vertical_sync_ = true;  // On by default.
      if (im_gui_->DoCheckBox("Vertical sync", &vertical_sync_, true)) {
        glfwSwapInterval(vertical_sync_ ? 1 : 0);
      }
    }

    // Searches for matching resolution settings.
    int preset_lookup = 0;
    for (; preset_lookup < kNumPresets - 1; ++preset_lookup) {
      const Resolution& preset = resolution_presets[preset_lookup];
      if (preset.width > resolution_.width) {
        break;
      } else if (preset.width == resolution_.width) {
        if (preset.height >= resolution_.height) {
            break;
        }
      }
    }

    char szResolution[64];
    std::sprintf(szResolution, "Resolution: %dx%d",
                 resolution_.width, resolution_.height);
    if (im_gui_->DoSlider(szResolution, 0, kNumPresets - 1, &preset_lookup)) {
      // Resolution changed.
      resolution_ = resolution_presets[preset_lookup];
    }
  }

  { // Capture
    static bool open = false;
    ImGui::OpenClose controls(im_gui_, "Capture", &open);
    if (open) {
      im_gui_->DoButton("Capture video", true, &capture_video_);
      capture_screenshot_ =
        im_gui_->DoButton("Capture screenshot", !capture_video_);
    }
  }

  { // Controls
    static bool open = false;
    ImGui::OpenClose controls(im_gui_, "Camera controls", &open);
    if (open) {
      camera_->OnGui(im_gui_);
    }
  }
  return true;
}

void Application::ResizeCbk(int _width, int _height) {
  // Stores new resolution settings.
  application_->resolution_.width = _width;
  application_->resolution_.height = _height;

  // Uses the full viewport.
  GL(Viewport(0, 0, _width, _height));

  // Forwards screen size to camera and shooter.
  application_->camera_->Resize(_width, _height);
  application_->shooter_->Resize(_width, _height);
}

int Application::CloseCbk() {
  application_->exit_ = true;
  return GL_FALSE;  // The window will be closed while exiting the main loop.
}

void Application::ParseReadme() {
  // Get README file
  ozz::io::File file("README", "rb");  // Opens as binary to avoid conversions.
  if (!file.opened()) {
    help_ = "Unable to find README help file.";
    return;
  }

  // Get file length.
  file.Seek(0, ozz::io::Stream::kEnd);
  size_t length = file.Tell();
  file.Seek(0, ozz::io::Stream::kSet);

  // Allocate enough space to store the whole file.
  ozz::memory::Allocator* allocator = ozz::memory::default_allocator();
  char* content = allocator->Allocate<char>(length);

  // Read the content
  if(file.Read(content, length) == length) {
    help_ = ozz::String::Std(content, content + length);
  } else {
    help_ = "Unable to find README help file.";
  }

  // Deallocate temporary buffer;
  allocator->Deallocate(content);
}
}  // sample
}  // ozz
