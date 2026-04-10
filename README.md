# CBAR — Cambrian Augmented Reality Engine

**Real-time 3D scene understanding on mobile devices.** Surface normals, plane detection, semantic segmentation, optical flow, floor/wall recognition, and photorealistic rendering — all running at 60fps on an iPhone 7 (2015 hardware).

Built by [Joel Teply](https://github.com/joelteply), 2011-2020. ~50,000 lines of C++ across OpenCV, Eigen, OpenGL ES, TensorFlow Lite, and pthreads. Shipped in production AR apps on iOS and Android (paint visualization, flooring preview, 3D model placement).

---

## Architecture

CBAR is a **frame-driven pipeline** with three core abstractions:

### VideoFrame — Lazy-Eval Data Bus

Each frame is a shared data object with **cached lazy getters**. Downstream consumers request derived images (`getRGBImage()`, `getHSVImage()`, `getBWImage()`, `getEdgesImage()`, `getOpticalFlowImage()`, `uprightBWImage()`, etc.) and the frame computes them once on first access. This eliminates redundant work when multiple analyzers need the same derived data.

```
RawFrame (YUV/BGRA from camera)
    → CBAR_VideoFrame
        .getRGBAImage()        // computed once, cached
        .getRGBImage()         // computed once, cached
        .getHSVImage()         // computed once, cached
        .getBWImage()          // computed once, cached
        .getEdgesImage()       // computed once, cached
        .getOpticalFlowImage() // computed once, cached
        .uprightBWImage()      // rotation-corrected, cached
        .getEnhancedImage()    // line-aware enhancement, cached
```

Rotation-aware: when device orientation changes, downstream caches invalidate and recompute.

### QueueThread\<T\> — Priority-Tiered RTOS

All heavy work runs on background threads via `QueueThread<T>`, a bounded producer-consumer queue with **priority-tiered wake intervals**:

```cpp
// Higher priority = faster wake cycle
timedWait(10 + 100 * int(1 + priority))
// CBThreadPriorityHighest → 10ms   (optical flow, feature tracking)
// CBThreadPriorityDefault → 410ms  (semantic segmentation, normals)
// CBThreadPriorityLowest  → 610ms  (ambience sampling, elevation)
```

Bounded queue with backpressure — when the queue is full, the oldest frame is dropped, not the caller blocked. This guarantees the main thread never stalls and every analyzer always gets the most recent data.

### CBP_Analyzer — Subscriber Pipeline

Analyzers are **ProcessNode subscribers** that each run on their own `QueueThread`. The `CBP_RenderingEngine` broadcasts frames; each analyzer processes them at its own priority and cadence:

| Analyzer | Priority | Purpose |
|----------|----------|---------|
| `CBP_FeatureTracker` | Highest / Real-time | Optical flow, camera pose, point cloud |
| `CBP_PlaneAnalyzer` | High | RANSAC plane detection, ground plane tracking |
| `CBP_NormalsAnalyzer` | Default | CNN surface normals estimation |
| `CBP_FloorSegmenter` | Default | Floor surface segmentation for paint/flooring |
| `CBP_WallSegmenter` | Default | Wall surface segmentation |
| `CBP_SemanticAnalyzer` | Default | Full-scene semantic segmentation (150 classes) |
| `CBP_ShadowsAnalyzer` | Default | Light source detection, shadow estimation |
| `CBP_ElevationAnalyzer` | Default | Depth/elevation from monocular cues |
| `CBP_LineFinder` | Default | Structural line detection (walls, edges) |
| `CBP_AmbienceSampler` | Low | Ambient light color/intensity sampling |
| `CBP_ColorFinder` | Low | Dominant color extraction |
| `CBP_GroundSurfaceAnalyzer` | Low | Ground surface material classification |

Analyzers are **hot-pluggable** at runtime via `appendAnalyzer()` / `removeAnalyzer()`. The type system supports runtime discovery:

```cpp
auto normals = renderingEngine->getAnalyzerOfType<CBP_NormalsAnalyzer>();
auto segmenters = renderingEngine->getAnalyzersOfType<CBP_Segmenter>();
```

---

## Two-Tier Compute Strategy

**Tier 1 — Synchronous heartbeat** (every frame, GPU, quarter-resolution):
- Optical flow for motion estimation and camera drift detection
- Feature tracking for pose recovery

**Tier 2 — Lazy on-demand** (when needed, per-analyzer cadence):
- Surface normals (CNN inference)
- Semantic segmentation (CNN inference)
- Plane detection (RANSAC)
- Elevation estimation (CNN inference)
- Shadow/lighting estimation

The key insight: surface normals give 3D structure from **frame 1** (instant coarse), while SFM and multi-frame accumulation refine progressively over time.

---

## Machine Learning Pipeline

Deep models run via `CBP_DeepModel`, a polymorphic base with per-model subclasses:

| Model | Class | Output |
|-------|-------|--------|
| Surface Normals | `CBP_SurfaceNormalsEstimator` | Per-pixel XYZ normal vectors |
| Semantic Segmentation | `CBP_SemanticSegmenter` | 150-class ADE20K labels |
| Elevation/Depth | `CBP_ElevationEstimator` | Monocular depth map |
| Shadows | `CBP_ShadowsEstimator` | Light source + shadow mask |

Models are loaded lazily with singleton access (mutex-guarded). Inference runs off the main thread via the analyzer's own `QueueThread`.

---

## Scene Graph

```
CBAR_Scene
├── Assets (CBAR_Asset)
│   ├── CBAR_SurfaceAsset (paint, wallpaper, tile)
│   ├── CBAR_Floor (flooring materials)
│   ├── CBAR_Paint (wall paint with sheen/finish)
│   └── CBAR_Model (3D objects)
├── World Transform (6DOF camera pose)
├── Camera Projection
├── Lighting (10 presets: daylight, incandescent, LED, etc.)
├── Undo/Redo Stack
└── JSON Serialization (save/load scenes)
```

The `CBP_SurfaceAccumulator` maintains a **top-down accumulated surface map** that fuses evidence from multiple frames, multiple analyzers, and multiple evidence types (normals, semantic labels, RANSAC planes, optical flow) into a unified surface model. Image-type-agnostic — it builds planes from ANY evidence source.

---

## Rendering

`CBP_RenderingEngine` orchestrates the full pipeline:

1. Receive raw camera frame
2. Broadcast to all analyzers (parallel, priority-tiered)
3. Collect analysis results
4. Composite AR overlay via OpenGL ES:
   - Surface-aware paint/material application
   - Lighting-corrected color adjustment
   - Perspective-correct texture mapping
   - Shadow and highlight preservation

The renderer handles coordinate transforms between screen space, video space, and 3D world space, with proper handling of device rotation and camera intrinsics.

---

## Platform Bindings

| Platform | Location | Integration |
|----------|----------|-------------|
| **iOS** | `ios/CambrianSDK/` | Objective-C++ framework, ARKit bridge |
| **Android** | `android/Harmony/` | JNI via CMake, ARCore bridge |
| **Unity** | `unity/CambrianARVR/` | C# plugin wrapping native lib |
| **Web** | `web/` | TypeScript/React port (subset of functionality) |
| **Server** | `cpp/server/` | Headless mode for batch processing |

The C++ core (`cpp/`) is the single source of truth. All platform SDKs are thin wrappers.

---

## Dependencies

- **OpenCV** — Image processing, feature detection, matrix operations
- **Eigen** — 3D geometry, transforms, quaternions, RANSAC
- **OpenGL ES 3.0** — GPU rendering pipeline
- **TensorFlow Lite** — On-device CNN inference (normals, segmentation, elevation, shadows)
- **pthreads** — Threading primitives (CBThread, CBMutex, CBCondition)
- **LMDB** — On-device key-value storage for caching
- **poly2tri / triangle** — Constrained Delaunay triangulation for mesh generation
- **JSON (jsoncpp)** — Scene serialization

---

## Building

```bash
# iOS
open ios/CambrianSDK/CambrianSDK.xcodeproj

# Android
cd android/Harmony && ./gradlew assembleRelease

# C++ core (CMake)
cd cpp && mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

---

## History

Started in 2011 as a paint color visualization tool. By 2015, it was doing real-time 3D scene understanding on iPhone hardware — surface normals, plane detection, semantic segmentation, optical flow, and photorealistic AR compositing, all at 60fps. Won Launch KC ($100K, 2018). Licensed by paint and flooring companies for consumer AR apps.

The architecture — lazy-eval frames, priority-tiered RTOS threads, pluggable analyzer pipeline, image-type-agnostic surface accumulation — became the design seed for two successor projects:

- **[open-eyes](https://github.com/CambrianTech/open-eyes)** — Rust port of the 3D scene understanding pipeline for multi-camera security/metaverse applications
- **[Continuum](https://github.com/CambrianTech/continuum)** — Distributed AI platform whose daemon architecture descends from this RTOS threading model

---

## License

Copyright 2011-2020 Joel Teply / Cambrian Tech LLC. Apache-2.0.
