/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrGpu.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"

#include <cstdint>

struct GrContextOptions;

static bool check_read(skiatest::Reporter* reporter, const SkBitmap& bitmap) {
    bool result = true;
    for (int x = 0; x < 1000 && result; ++x) {
        const uint32_t srcPixel = *bitmap.getAddr32(x, 0);
        if (srcPixel != SK_ColorGREEN) {
            ERRORF(reporter, "Expected color of Green, but got 0x%08x, at pixel (%d, 0).",
                   srcPixel, x);
            result = false;
        }
    }
    return result;
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(OpsTaskFlushCount,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    auto context = ctxInfo.directContext();
    GrGpu* gpu = context->priv().getGpu();

    SkImageInfo imageInfo = SkImageInfo::Make(1000, 1, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    sk_sp<SkSurface> surface1 = SkSurfaces::RenderTarget(context, skgpu::Budgeted::kYes, imageInfo);
    if (!surface1) {
        return;
    }
    sk_sp<SkSurface> surface2 = SkSurfaces::RenderTarget(context, skgpu::Budgeted::kYes, imageInfo);
    if (!surface2) {
        return;
    }

    SkCanvas* canvas1 = surface1->getCanvas();
    SkCanvas* canvas2 = surface2->getCanvas();

    canvas1->clear(SK_ColorRED);
    canvas2->clear(SK_ColorRED);

    SkRect srcRect = SkRect::MakeWH(1, 1);
    SkRect dstRect = SkRect::MakeWH(1, 1);
    SkPaint paint;
    paint.setColor(SK_ColorGREEN);
    canvas1->drawRect(dstRect, paint);

    for (int i = 0; i < 1000; ++i) {
        srcRect.fLeft = i;
        srcRect.fRight = srcRect.fLeft + 1;

        sk_sp<SkImage> image = surface1->makeImageSnapshot();
        canvas2->drawImageRect(image.get(), srcRect, dstRect, SkSamplingOptions(), nullptr,
                               SkCanvas::kStrict_SrcRectConstraint);
        if (i != 999) {
            dstRect.fLeft = i+1;
            dstRect.fRight = dstRect.fLeft + 1;
            image = surface2->makeImageSnapshot();
            canvas1->drawImageRect(image.get(), srcRect, dstRect, SkSamplingOptions(), nullptr,
                                   SkCanvas::kStrict_SrcRectConstraint);
        }
    }
    context->flushAndSubmit();

    // In total we make 2000 oplists. Our current limit on max oplists between flushes is 100, so we
    // should do 20 flushes while executing oplists. Additionaly we always do 1 flush at the end of
    // executing all oplists. So in total we should see 21 flushes here.
    REPORTER_ASSERT(reporter, gpu->stats()->numSubmitToGpus() == 21);

    SkBitmap readbackBitmap;
    readbackBitmap.allocN32Pixels(1000, 1);
    REPORTER_ASSERT(reporter, surface1->readPixels(readbackBitmap, 0, 0));
    REPORTER_ASSERT(reporter, check_read(reporter, readbackBitmap));

    REPORTER_ASSERT(reporter, surface2->readPixels(readbackBitmap, 0, 0));
    REPORTER_ASSERT(reporter, check_read(reporter, readbackBitmap));
}
