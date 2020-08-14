/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkOpPathEffect_DEFINED
#define SkOpPathEffect_DEFINED

#include "SkPathEffect.h"
#include "SkPaint.h"
#include "SkPathOps.h"

class SkMergePathEffect {
public:
	/*  Defers to two other patheffects, and then combines their outputs using the specified op.
	 *  e.g.
	 *      result = output_one op output_two
	 *
	 *  If either one or two is nullptr, then the original path is passed through to the op.
	 */
	static sk_sp<SkPathEffect> Make(sk_sp<SkPathEffect> one, sk_sp<SkPathEffect> two, SkPathOp op);
};

class SkMatrixPathEffect {
public:
	static sk_sp<SkPathEffect> MakeTranslate(SkScalar dx, SkScalar dy);
	static sk_sp<SkPathEffect> Make(const SkMatrix&);
};

class SkStrokePathEffect {
public:
	static sk_sp<SkPathEffect> Make(SkScalar width, SkPaint::Join, SkPaint::Cap,
									SkScalar miter = 4);
};

#endif
