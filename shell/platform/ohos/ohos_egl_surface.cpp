/*
 * Copyright (c) 2023 Hunan OpenValley Digital Industry Development Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "flutter/shell/platform/ohos/ohos_egl_surface.h"

#include <EGL/eglext.h>

#include <array>
#include <list>

#include "flutter/fml/trace_event.h"

namespace flutter {

void LogLastEGLError() {
  struct EGLNameErrorPair {
    const char* name;
    EGLint code;
  };

  const EGLNameErrorPair pairs[] = {
      {"EGL_SUCCESS", EGL_SUCCESS},
      {"EGL_NOT_INITIALIZED", EGL_NOT_INITIALIZED},
      {"EGL_BAD_ACCESS", EGL_BAD_ACCESS},
      {"EGL_BAD_ALLOC", EGL_BAD_ALLOC},
      {"EGL_BAD_ATTRIBUTE", EGL_BAD_ATTRIBUTE},
      {"EGL_BAD_CONTEXT", EGL_BAD_CONTEXT},
      {"EGL_BAD_CONFIG", EGL_BAD_CONFIG},
      {"EGL_BAD_CURRENT_SURFACE", EGL_BAD_CURRENT_SURFACE},
      {"EGL_BAD_DISPLAY", EGL_BAD_DISPLAY},
      {"EGL_BAD_SURFACE", EGL_BAD_SURFACE},
      {"EGL_BAD_MATCH", EGL_BAD_MATCH},
      {"EGL_BAD_PARAMETER", EGL_BAD_PARAMETER},
      {"EGL_BAD_NATIVE_PIXMAP", EGL_BAD_NATIVE_PIXMAP},
      {"EGL_BAD_NATIVE_WINDOW", EGL_BAD_NATIVE_WINDOW},
      {"EGL_CONTEXT_LOST", EGL_CONTEXT_LOST}};

  const auto count = sizeof(pairs) / sizeof(EGLNameErrorPair);

  EGLint last_error = eglGetError();

  for (size_t i = 0; i < count; i++) {
    if (last_error == pairs[i].code) {
      FML_LOG(ERROR) << "EGL Error: " << pairs[i].name << " (" << pairs[i].code
                     << ")";
      return;
    }
  }

  FML_LOG(ERROR) << "Unknown EGL Error";
}

namespace {

static bool HasExtension(const char* extensions, const char* name) {
  const char* r = strstr(extensions, name);
  auto len = strlen(name);
  // check that the extension name is terminated by space or null terminator
  return r != nullptr && (r[len] == ' ' || r[len] == 0);
}

}  // namespace

class OhosEGLSurfaceDamage {
 public:
  void init(EGLDisplay display, EGLContext context) {
    const char* extensions = eglQueryString(display, EGL_EXTENSIONS);
    if (HasExtension(extensions, "EGL_KHR_partial_update")) {
      set_damage_region_ = reinterpret_cast<PFNEGLSETDAMAGEREGIONKHRPROC>(
          eglGetProcAddress("eglSetDamageRegionKHR"));
    }
    if (HasExtension(extensions, "EGL_EXT_swap_buffers_with_damage")) {
        swap_buffers_with_damage_ = reinterpret_cast<PFNEGLSWAPBUFFERSWITHDAMAGEEXTPROC>(
              eglGetProcAddress("eglSwapBuffersWithDamageEXT"));
    } else if (HasExtension(extensions, "EGL_KHR_swap_buffers_with_damage")) {
        swap_buffers_with_damage_ = reinterpret_cast<PFNEGLSWAPBUFFERSWITHDAMAGEEXTPROC>(
              eglGetProcAddress("eglSwapBuffersWithDamageKHR"));
    }

    partial_redraw_supported_ = false;
  }

  void SetDamageRegion(EGLDisplay display,
                       EGLSurface surface,
                       const std::optional<SkIRect>& region) {
    if (partial_redraw_supported_ && set_damage_region_ && region) {
      auto rects = RectToInts(display, surface, *region);
      set_damage_region_(display, surface, rects.data(), 1);
    }
  }

  // Maximum damage history - for triple buffering we need to store damage for
  // last two frames; Some Ohos devices (Pixel 4) use quad buffering.
  static const int kMaxHistorySize = 10;

  bool SupportsPartialRepaint() const { return partial_redraw_supported_; }

  std::optional<SkIRect> InitialDamage(EGLDisplay display, EGLSurface surface) {
    if (!partial_redraw_supported_) {
      return std::nullopt;
    }

    EGLint age;
    eglQuerySurface(display, surface, EGL_BUFFER_AGE_EXT, &age);

    if (age == 0) {  // full repaint
      return std::nullopt;
    } else {
      // join up to (age - 1) last rects from damage history
      --age;
      auto res = SkIRect::MakeEmpty();
      for (auto i = damage_history_.rbegin();
           i != damage_history_.rend() && age > 0; ++i, --age) {
        res.join(*i);
      }
      return res;
    }
  }

  bool SwapBuffersWithDamage(EGLDisplay display,
                             EGLSurface surface,
                             const std::optional<SkIRect>& damage) {
    if (partial_redraw_supported_ && swap_buffers_with_damage_ && damage) {
      damage_history_.push_back(*damage);
      if (damage_history_.size() > kMaxHistorySize) {
        damage_history_.pop_front();
      }
      auto rects = RectToInts(display, surface, *damage);
      return swap_buffers_with_damage_(display, surface, rects.data(), 1);
    } else {
      return eglSwapBuffers(display, surface);
    }
  }

 private:
  std::array<EGLint, 4> static RectToInts(EGLDisplay display,
                                          EGLSurface surface,
                                          const SkIRect& rect) {
    EGLint height;
    eglQuerySurface(display, surface, EGL_HEIGHT, &height);

    std::array<EGLint, 4> res{rect.left(), height - rect.bottom(), rect.width(),
                              rect.height()};
    return res;
  }

  PFNEGLSETDAMAGEREGIONKHRPROC set_damage_region_ = nullptr;
  PFNEGLSWAPBUFFERSWITHDAMAGEEXTPROC swap_buffers_with_damage_ = nullptr;

  bool partial_redraw_supported_;

  std::list<SkIRect> damage_history_;
};

OhosEGLSurface::OhosEGLSurface(EGLSurface surface,
                               EGLDisplay display,
                               EGLContext context)
    : surface_(surface),
      display_(display),
      context_(context),
      damage_(std::make_unique<OhosEGLSurfaceDamage>()),
      presentation_time_proc_(nullptr) {
  FML_LOG(INFO) << "OhosEGLSurface start";
  damage_->init(display_, context);
  FML_LOG(INFO) << "OhosEGLSurface end";
}

OhosEGLSurface::~OhosEGLSurface() {
  [[maybe_unused]] auto result = eglDestroySurface(display_, surface_);
  FML_DCHECK(result == EGL_TRUE);
}

bool OhosEGLSurface::IsValid() const {
  return surface_ != EGL_NO_SURFACE;
}

bool OhosEGLSurface::IsContextCurrent() const {
  EGLContext current_egl_context = eglGetCurrentContext();
  if (context_ != current_egl_context) {
    return false;
  }

  EGLDisplay current_egl_display = eglGetCurrentDisplay();
  if (display_ != current_egl_display) {
    return false;
  }

  EGLSurface draw_surface = eglGetCurrentSurface(EGL_DRAW);
  if (draw_surface != surface_) {
    return false;
  }

  EGLSurface read_surface = eglGetCurrentSurface(EGL_READ);
  if (read_surface != surface_) {
    return false;
  }

  return true;
}

OhosEGLSurfaceMakeCurrentStatus OhosEGLSurface::MakeCurrent() const {
  if (IsContextCurrent()) {
    return OhosEGLSurfaceMakeCurrentStatus::kSuccessAlreadyCurrent;
  }
  if (eglMakeCurrent(display_, surface_, surface_, context_) != EGL_TRUE) {
    FML_LOG(ERROR) << "Could not make the context current";
    LogLastEGLError();
    return OhosEGLSurfaceMakeCurrentStatus::kFailure;
  }
  return OhosEGLSurfaceMakeCurrentStatus::kSuccessMadeCurrent;
}

void OhosEGLSurface::SetDamageRegion(
    const std::optional<SkIRect>& buffer_damage) {
  damage_->SetDamageRegion(display_, surface_, buffer_damage);
}

bool OhosEGLSurface::SetPresentationTime(
    const fml::TimePoint& presentation_time) {
  if (presentation_time_proc_) {
    const auto time_ns = presentation_time.ToEpochDelta().ToNanoseconds();
    return presentation_time_proc_(display_, surface_, time_ns);
  } else {
    return false;
  }
}

bool OhosEGLSurface::SwapBuffers(const std::optional<SkIRect>& surface_damage) {
  TRACE_EVENT0("flutter", "OhosContextGL::SwapBuffers");
  return damage_->SwapBuffersWithDamage(display_, surface_, surface_damage);
}

bool OhosEGLSurface::SupportsPartialRepaint() const {
  return damage_->SupportsPartialRepaint();
}

std::optional<SkIRect> OhosEGLSurface::InitialDamage() {
  return damage_->InitialDamage(display_, surface_);
}

SkISize OhosEGLSurface::GetSize() const {
  EGLint width = 0;
  EGLint height = 0;

  if (!eglQuerySurface(display_, surface_, EGL_WIDTH, &width) ||
      !eglQuerySurface(display_, surface_, EGL_HEIGHT, &height)) {
    FML_LOG(ERROR) << "Unable to query EGL surface size";
    LogLastEGLError();
    return SkISize::Make(0, 0);
  }
  return SkISize::Make(width, height);
}

}  // namespace flutter
