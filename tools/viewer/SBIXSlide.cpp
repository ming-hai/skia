/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkTypeface.h"
#include "include/private/base/SkTemplates.h"
#include "src/base/SkTime.h"
#include "src/sfnt/SkOTTable_glyf.h"
#include "src/sfnt/SkOTTable_head.h"
#include "src/sfnt/SkOTTable_hhea.h"
#include "src/sfnt/SkOTTable_hmtx.h"
#include "src/sfnt/SkOTTable_loca.h"
#include "src/sfnt/SkOTTable_maxp.h"
#include "src/sfnt/SkOTTable_sbix.h"
#include "src/sfnt/SkSFNTHeader.h"
#include "tools/Resources.h"
#include "tools/fonts/FontToolUtils.h"
#include "tools/timer/TimeUtils.h"
#include "tools/viewer/ClickHandlerSlide.h"

#if defined(SK_FONTMGR_FREETYPE_EMPTY_AVAILABLE)
#include "include/ports/SkFontMgr_empty.h"
#endif
#if defined(SK_FONTMGR_FONTATIONS_AVAILABLE)
#include "include/ports/SkFontMgr_Fontations.h"
#endif

namespace {

constexpr SkScalar DX = 100;
constexpr SkScalar DY = 300;
constexpr int kPointSize = 5;
constexpr SkScalar kFontSize = 200;

constexpr char kFontFile[] = "fonts/sbix_uncompressed_flags.ttf";
constexpr SkGlyphID kGlyphID = 2;
constexpr uint16_t kStrikeIndex = 2;

//constexpr char kFontFile[] = "fonts/HangingS.ttf";
//constexpr SkGlyphID kGlyphID = 4;

/**
 *  Return the closest int for the given float. Returns SK_MaxS32FitsInFloat for NaN.
 */
static inline int16_t sk_float_saturate2int16(float x) {
    x = x < SK_MaxS16 ? x : SK_MaxS16;
    x = x > SK_MinS16 ? x : SK_MinS16;
    return (int16_t)x;
}

struct ShortCoordinate { bool negative; uint8_t magnitude; };
static inline ShortCoordinate sk_float_saturate2sm8(float x) {
    bool negative = x < 0;
    x = x <  255 ? x :  255;
    x = x > -255 ? x : -255;
    return ShortCoordinate{ negative, negative ? (uint8_t)-x : (uint8_t)x };
}

struct SBIXSlide : public ClickHandlerSlide {
    struct Point {
        SkPoint location;
        SkColor color;
    } fPts[5] = {
        {{0, 0}, SK_ColorBLACK }, // glyph x/y min
        {{0, 0}, SK_ColorWHITE }, // glyph x/y max
        {{0, 20}, SK_ColorGREEN }, // lsb (x only, y ignored)
        {{0, 0}, SK_ColorBLUE }, // first point of glyph contour
        {{0, 0}, SK_ColorRED }, // sbix glyph data's origin offset
    };
    static constexpr const int kGlyfXYMin = 0;
    static constexpr const int kGlyfXYMax = 1;
    static constexpr const int kGlyfLSB = 2;
    static constexpr const int kGlyfFirstPoint = 3;
    static constexpr const int kOriginOffset = 4;

    std::vector<sk_sp<SkFontMgr>> fFontMgr;
    std::vector<SkFont> fFonts;
    sk_sp<SkData> fSBIXData;
    bool fInputChanged = false;
    bool fDirty = true;

public:
    SBIXSlide() { fName = "SBIX"; }

    void load(SkScalar w, SkScalar h) override {
        fFontMgr.emplace_back(ToolUtils::TestFontMgr());
#if defined(SK_FONTMGR_FREETYPE_EMPTY_AVAILABLE)
        fFontMgr.emplace_back(SkFontMgr_New_Custom_Empty());
#endif
#if defined(SK_FONTMGR_FONTATIONS_AVAILABLE)
        fFontMgr.emplace_back(SkFontMgr_New_Fontations_Empty());
#endif

        // GetResourceAsData may be backed by a read only file mapping.
        // For sanity always make a copy.
        fSBIXData = GetResourceAsData(kFontFile);

        updateSBIXData(fSBIXData.get(), true);
    }

    void draw(SkCanvas* canvas) override {
        canvas->clear(SK_ColorGRAY);

        canvas->translate(DX, DY);

        SkPaint paint;
        SkPoint position{0, 0};
        SkPoint origin{0, 0};

        if (fDirty) {
            sk_sp<SkData> data(updateSBIXData(fSBIXData.get(), false));
            fFonts.clear();
            for (auto&& fontmgr : fFontMgr) {
                fFonts.emplace_back(fontmgr->makeFromData(data), kFontSize);
            }
            fDirty = false;
        }
        for (auto&& font : fFonts) {
            paint.setStyle(SkPaint::kFill_Style);
            paint.setColor(SK_ColorBLACK);
            canvas->drawGlyphs({&kGlyphID, 1}, {&position, 1}, origin, font, paint);

            paint.setStrokeWidth(SkIntToScalar(kPointSize / 2));
            paint.setStyle(SkPaint::kStroke_Style);
            SkScalar advance;
            SkRect rect;
            font.getWidthsBounds(&kGlyphID, 1, &advance, &rect, &paint);

            paint.setColor(SK_ColorRED);
            canvas->drawRect(rect, paint);
            paint.setColor(SK_ColorGREEN);
            canvas->drawLine(0, 0, advance, 0, paint);
            paint.setColor(SK_ColorRED);
            canvas->drawPoint(0, 0, paint);
            canvas->drawPoint(advance, 0, paint);

            paint.setStrokeWidth(SkIntToScalar(kPointSize));
            for (auto&& pt : fPts) {
                paint.setColor(pt.color);
                canvas->drawPoints(SkCanvas::kPoints_PointMode, {&pt.location, 1}, paint);
            }

            canvas->translate(kFontSize, 0);
        }
    }

protected:
    static bool hittest(const SkPoint& pt, SkScalar x, SkScalar y) {
        return SkPoint::Length(pt.fX - x, pt.fY - y) < SkIntToScalar(kPointSize);
    }

    Click* onFindClickHandler(SkScalar x, SkScalar y, skui::ModifierKey modi) override {
        x -= DX;
        y -= DY;
        // Look for hit in opposite of paint order
        for (auto&& pt = std::rbegin(fPts); pt != std::rend(fPts); ++pt) {
            if (hittest(pt->location, x, y)) {
                return new PtClick(&*pt);
            }
        }
        return nullptr;
    }

    bool onClick(Click* click) override {
        ((PtClick*)click)->fPt->location.set(click->fCurr.fX - DX, click->fCurr.fY - DY);
        fDirty = true;
        return true;
    }

private:
    class PtClick : public Click {
    public:
        Point* fPt;
        PtClick(Point* pt) : fPt(pt) {}
    };

    sk_sp<SkData> updateSBIXData(SkData* originalData, bool setPts) {
        // Lots of unlikely to be aligned pointers in here, which is UB. Total hack.

        sk_sp<SkData> dataCopy = SkData::MakeWithCopy(originalData->data(), originalData->size());

        SkSFNTHeader* sfntHeader = static_cast<SkSFNTHeader*>(dataCopy->writable_data());

        SkASSERT_RELEASE(memcmp(sfntHeader, originalData->data(), originalData->size()) == 0);

        SkSFNTHeader::TableDirectoryEntry* tableEntry =
                SkTAfter<SkSFNTHeader::TableDirectoryEntry>(sfntHeader);
        SkSFNTHeader::TableDirectoryEntry* glyfTableEntry = nullptr;
        SkSFNTHeader::TableDirectoryEntry* headTableEntry = nullptr;
        SkSFNTHeader::TableDirectoryEntry* hheaTableEntry = nullptr;
        SkSFNTHeader::TableDirectoryEntry* hmtxTableEntry = nullptr;
        SkSFNTHeader::TableDirectoryEntry* locaTableEntry = nullptr;
        SkSFNTHeader::TableDirectoryEntry* maxpTableEntry = nullptr;
        SkSFNTHeader::TableDirectoryEntry* sbixTableEntry = nullptr;
        int numTables = SkEndian_SwapBE16(sfntHeader->numTables);
        for (int tableEntryIndex = 0; tableEntryIndex < numTables; ++tableEntryIndex) {
            if (SkOTTableGlyph::TAG == tableEntry[tableEntryIndex].tag) {
                glyfTableEntry = tableEntry + tableEntryIndex;
            }
            if (SkOTTableHead::TAG == tableEntry[tableEntryIndex].tag) {
                headTableEntry = tableEntry + tableEntryIndex;
            }
            if (SkOTTableHorizontalHeader::TAG == tableEntry[tableEntryIndex].tag) {
                hheaTableEntry = tableEntry + tableEntryIndex;
            }
            if (SkOTTableHorizontalMetrics::TAG == tableEntry[tableEntryIndex].tag) {
                hmtxTableEntry = tableEntry + tableEntryIndex;
            }
            if (SkOTTableIndexToLocation::TAG == tableEntry[tableEntryIndex].tag) {
                locaTableEntry = tableEntry + tableEntryIndex;
            }
            if (SkOTTableMaximumProfile::TAG == tableEntry[tableEntryIndex].tag) {
                maxpTableEntry = tableEntry + tableEntryIndex;
            }
            if (SkOTTableStandardBitmapGraphics::TAG == tableEntry[tableEntryIndex].tag) {
                sbixTableEntry = tableEntry + tableEntryIndex;
            }
        }
        SkASSERT_RELEASE(glyfTableEntry);
        SkASSERT_RELEASE(headTableEntry);
        SkASSERT_RELEASE(hheaTableEntry);
        SkASSERT_RELEASE(hmtxTableEntry);
        SkASSERT_RELEASE(locaTableEntry);
        SkASSERT_RELEASE(maxpTableEntry);

        size_t glyfTableOffset = SkEndian_SwapBE32(glyfTableEntry->offset);
        SkOTTableGlyph* glyfTable =
                SkTAddOffset<SkOTTableGlyph>(sfntHeader, glyfTableOffset);

        size_t headTableOffset = SkEndian_SwapBE32(headTableEntry->offset);
        SkOTTableHead* headTable =
                SkTAddOffset<SkOTTableHead>(sfntHeader, headTableOffset);

        size_t hheaTableOffset = SkEndian_SwapBE32(hheaTableEntry->offset);
        SkOTTableHorizontalHeader* hheaTable =
                SkTAddOffset<SkOTTableHorizontalHeader>(sfntHeader, hheaTableOffset);

        size_t hmtxTableOffset = SkEndian_SwapBE32(hmtxTableEntry->offset);
        SkOTTableHorizontalMetrics* hmtxTable =
                SkTAddOffset<SkOTTableHorizontalMetrics>(sfntHeader, hmtxTableOffset);

        size_t locaTableOffset = SkEndian_SwapBE32(locaTableEntry->offset);
        SkOTTableIndexToLocation* locaTable =
                SkTAddOffset<SkOTTableIndexToLocation>(sfntHeader, locaTableOffset);

        size_t maxpTableOffset = SkEndian_SwapBE32(maxpTableEntry->offset);
        SkOTTableMaximumProfile* maxpTable =
                SkTAddOffset<SkOTTableMaximumProfile>(sfntHeader, maxpTableOffset);

        SkASSERT_RELEASE(SkEndian_SwapBE32(maxpTable->version.version) == 0x00010000);
        int numGlyphs = SkEndian_SwapBE16(maxpTable->version.tt.numGlyphs);
        SkASSERT_RELEASE(kGlyphID < numGlyphs);

        int emSize = SkEndian_SwapBE16(headTable->unitsPerEm);
        SkScalar toEm = emSize / kFontSize;

        if (sbixTableEntry) {
            size_t sbixTableOffset = SkEndian_SwapBE32(sbixTableEntry->offset);
            SkOTTableStandardBitmapGraphics* sbixTable =
                    SkTAddOffset<SkOTTableStandardBitmapGraphics>(sfntHeader, sbixTableOffset);

            uint32_t numStrikes = SkEndian_SwapBE32(sbixTable->numStrikes);
            SkASSERT_RELEASE(kStrikeIndex < numStrikes);

            uint32_t strikeOffset = SkEndian_SwapBE32(sbixTable->strikeOffset(kStrikeIndex));
            SkOTTableStandardBitmapGraphics::Strike* strike =
                    SkTAddOffset<SkOTTableStandardBitmapGraphics::Strike>(sbixTable, strikeOffset);
            uint16_t strikePpem = SkEndian_SwapBE16(strike->ppem);
            SkScalar toStrikeEm = strikePpem / kFontSize;
            uint32_t glyphDataOffset = SkEndian_SwapBE32(strike->glyphDataOffset(kGlyphID));
            uint32_t glyphDataOffsetNext = SkEndian_SwapBE32(strike->glyphDataOffset(kGlyphID+1));
            SkASSERT_RELEASE(glyphDataOffset < glyphDataOffsetNext);
            SkOTTableStandardBitmapGraphics::GlyphData* glyphData =
                    SkTAddOffset<SkOTTableStandardBitmapGraphics::GlyphData>(strike, glyphDataOffset);
            if (setPts) {
                fPts[kOriginOffset].location.set((int16_t)SkEndian_SwapBE16(glyphData->originOffsetX) /  toStrikeEm,
                                                 (int16_t)SkEndian_SwapBE16(glyphData->originOffsetY) / -toStrikeEm);
            } else {
                glyphData->originOffsetX = SkEndian_SwapBE16(sk_float_saturate2int16( fPts[kOriginOffset].location.x()*toStrikeEm));
                glyphData->originOffsetY = SkEndian_SwapBE16(sk_float_saturate2int16(-fPts[kOriginOffset].location.y()*toStrikeEm));
            }
        }

        SkOTTableGlyph::Iterator glyphIter(*glyfTable, *locaTable, headTable->indexToLocFormat);
        glyphIter.advance(kGlyphID);
        SkOTTableGlyphData* glyphData = glyphIter.next();
        if (glyphData) {
            if (setPts) {
                fPts[kGlyfXYMin].location.set((int16_t)SkEndian_SwapBE16(glyphData->xMin) /  toEm,
                                              (int16_t)SkEndian_SwapBE16(glyphData->yMin) / -toEm);
                fPts[kGlyfXYMax].location.set((int16_t)SkEndian_SwapBE16(glyphData->xMax) /  toEm,
                                              (int16_t)SkEndian_SwapBE16(glyphData->yMax) / -toEm);
            } else {
                glyphData->xMin = SkEndian_SwapBE16(sk_float_saturate2int16( fPts[kGlyfXYMin].location.x()*toEm));
                glyphData->yMin = SkEndian_SwapBE16(sk_float_saturate2int16(-fPts[kGlyfXYMin].location.y()*toEm));
                glyphData->xMax = SkEndian_SwapBE16(sk_float_saturate2int16( fPts[kGlyfXYMax].location.x()*toEm));
                glyphData->yMax = SkEndian_SwapBE16(sk_float_saturate2int16(-fPts[kGlyfXYMax].location.y()*toEm));
            }

            int contourCount = SkEndian_SwapBE16(glyphData->numberOfContours);
            if (contourCount > 0) {
                SK_OT_USHORT* endPtsOfContours = SkTAfter<SK_OT_USHORT>(glyphData);
                SK_OT_USHORT* numInstructions = SkTAfter<SK_OT_USHORT>(endPtsOfContours,
                                                                       contourCount);
                SK_OT_BYTE* instructions = SkTAfter<SK_OT_BYTE>(numInstructions);
                SkOTTableGlyphData::Simple::Flags* flags =
                        SkTAfter<SkOTTableGlyphData::Simple::Flags>(
                                instructions, SkEndian_SwapBE16(*numInstructions));

                int numResultPoints = SkEndian_SwapBE16(endPtsOfContours[contourCount-1]) + 1;
                struct Coordinate {
                    SkOTTableGlyphData::Simple::Flags* flags;
                    size_t offsetToXDelta;
                    size_t xDeltaSize;
                    size_t offsetToYDelta;
                    size_t yDeltaSize;
                };
                std::vector<Coordinate> coordinates(numResultPoints);

                size_t offsetToXDelta = 0;
                size_t offsetToYDelta = 0;
                SkOTTableGlyphData::Simple::Flags* currentFlags = flags;
                for (int i = 0; i < numResultPoints; ++i) {
                    SkOTTableGlyphData::Simple::Flags* nextFlags;
                    int times = 1;
                    if (currentFlags->field.Repeat) {
                        SK_OT_BYTE* repeat = SkTAfter<SK_OT_BYTE>(currentFlags);
                        times += *repeat;
                        nextFlags = SkTAfter<SkOTTableGlyphData::Simple::Flags>(repeat);
                    } else {
                        nextFlags = SkTAfter<SkOTTableGlyphData::Simple::Flags>(currentFlags);
                    }

                    --i;
                    for (int time = 0; time < times; ++time) {
                        ++i;
                        coordinates[i].flags = currentFlags;
                        coordinates[i].offsetToXDelta = offsetToXDelta;
                        coordinates[i].offsetToYDelta = offsetToYDelta;

                        if (currentFlags->field.xShortVector) {
                            offsetToXDelta += 1;
                            coordinates[i].xDeltaSize = 1;
                        } else if (currentFlags->field.xIsSame_xShortVectorPositive) {
                            offsetToXDelta += 0;
                            if (i == 0) {
                                coordinates[i].xDeltaSize = 0;
                            } else {
                                coordinates[i].xDeltaSize = coordinates[i-1].xDeltaSize;
                            }
                        } else {
                            offsetToXDelta += 2;
                            coordinates[i].xDeltaSize = 2;
                        }

                        if (currentFlags->field.yShortVector) {
                            offsetToYDelta += 1;
                            coordinates[i].yDeltaSize = 1;
                        } else if (currentFlags->field.yIsSame_yShortVectorPositive) {
                            offsetToYDelta += 0;
                            if (i == 0) {
                                coordinates[i].yDeltaSize = 0;
                            } else {
                                coordinates[i].yDeltaSize = coordinates[i-1].yDeltaSize;
                            }
                        } else {
                            offsetToYDelta += 2;
                            coordinates[i].yDeltaSize = 2;
                        }
                    }
                    currentFlags = nextFlags;
                }
                SK_OT_BYTE* xCoordinates = reinterpret_cast<SK_OT_BYTE*>(currentFlags);
                SK_OT_BYTE* yCoordinates = xCoordinates + offsetToXDelta;

                int pointIndex = 0;
                if (coordinates[pointIndex].xDeltaSize == 0) {
                    // Zero delta relative to the origin. There is no data to modify.
                    SkDebugf("Failed to move point in X at all.\n");
                } else if (coordinates[pointIndex].xDeltaSize == 1) {
                    ShortCoordinate x = sk_float_saturate2sm8(fPts[kGlyfFirstPoint].location.x()*toEm);
                    xCoordinates[coordinates[pointIndex].offsetToXDelta] = x.magnitude;
                    coordinates[pointIndex].flags->field.xIsSame_xShortVectorPositive = !x.negative;
                } else {
                    *reinterpret_cast<SK_OT_SHORT*>(xCoordinates + coordinates[pointIndex].offsetToXDelta) =
                            SkEndian_SwapBE16(sk_float_saturate2int16(fPts[kGlyfFirstPoint].location.x()*toEm));
                }

                if (coordinates[pointIndex].yDeltaSize == 0) {
                    // Zero delta relative to the origin. There is no data to modify.
                    SkDebugf("Failed to move point in Y at all.\n");
                } else if (coordinates[pointIndex].yDeltaSize == 1) {
                    ShortCoordinate y = sk_float_saturate2sm8(-fPts[kGlyfFirstPoint].location.y()*toEm);
                    yCoordinates[coordinates[pointIndex].offsetToYDelta] = y.magnitude;
                    coordinates[pointIndex].flags->field.yIsSame_yShortVectorPositive = !y.negative;
                } else {
                    *reinterpret_cast<SK_OT_SHORT*>(yCoordinates + coordinates[pointIndex].offsetToYDelta) =
                            SkEndian_SwapBE16(sk_float_saturate2int16(-fPts[kGlyfFirstPoint].location.y()*toEm));
                }
            }
        }

        int numberOfFullMetrics = SkEndian_SwapBE16(hheaTable->numberOfHMetrics);
        SkOTTableHorizontalMetrics::FullMetric* fullMetrics = hmtxTable->longHorMetric;
        SK_OT_SHORT lsb = SkEndian_SwapBE16(sk_float_saturate2int16(fPts[kGlyfLSB].location.x()*toEm));
        if (kGlyphID < numberOfFullMetrics) {
            if (setPts) {
                fPts[kGlyfLSB].location.fX = (int16_t)SkEndian_SwapBE16(fullMetrics[kGlyphID].lsb) / toEm;
            } else {
                fullMetrics[kGlyphID].lsb = lsb;
            }
        } else {
            SkOTTableHorizontalMetrics::ShortMetric* shortMetrics =
                    SkTAfter<SkOTTableHorizontalMetrics::ShortMetric>(fullMetrics, numberOfFullMetrics);
            int shortMetricIndex = kGlyphID - numberOfFullMetrics;
            if (setPts) {
                fPts[kGlyfLSB].location.fX = (int16_t)SkEndian_SwapBE16(shortMetrics[shortMetricIndex].lsb) / toEm;
            } else {
                shortMetrics[shortMetricIndex].lsb = lsb;
            }
        }

        headTable->flags.field.LeftSidebearingAtX0 = false;
        return dataCopy;
    }
};
}  // namespace
DEF_SLIDE( return new SBIXSlide(); )
