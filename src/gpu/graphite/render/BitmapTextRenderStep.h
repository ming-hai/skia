/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_render_BitmapTextRenderStep_DEFINED
#define skgpu_graphite_render_BitmapTextRenderStep_DEFINED

#include "src/base/SkEnumBitMask.h"
#include "src/base/SkVx.h"
#include "src/gpu/graphite/Renderer.h"

#include <string>

namespace skgpu {
enum class MaskFormat : int;
}

namespace skgpu::graphite {

class DrawParams;
class DrawWriter;
class PipelineDataGatherer;
struct ResourceBindingRequirements;

class BitmapTextRenderStep final : public RenderStep {
public:
    BitmapTextRenderStep(skgpu::MaskFormat variant);

    ~BitmapTextRenderStep() override;

    std::string vertexSkSL() const override;
    std::string texturesAndSamplersSkSL(const ResourceBindingRequirements&,
                                        int* nextBindingIndex) const override;
    // For a given BitmapTextRenderStep instance,
    // we will only use either fragmentColorSkSL() (for color text)
    // or fragmentCoverageSKSL() (for grayscale and LCD masks), never both.
    const char* fragmentColorSkSL() const override;
    const char* fragmentCoverageSkSL() const override;
    bool usesUniformsInFragmentSkSL() const override;

    void writeVertices(DrawWriter*, const DrawParams&, skvx::uint2 ssboIndices) const override;
    void writeUniformsAndTextures(const DrawParams&, PipelineDataGatherer*) const override;

private:
    static SkEnumBitMask<Flags> Flags(skgpu::MaskFormat);
};

}  // namespace skgpu::graphite

#endif // skgpu_graphite_render_BitmapTextRenderStep_DEFINED
