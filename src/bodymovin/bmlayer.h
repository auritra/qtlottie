/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the lottie-qt module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/
#pragma once

#include "bmbase.h"
#include "bmbasictransform.h"

namespace Lottie {

class BMMasks;

#define BM_LAYER_PRECOMP_IX 0x10000
#define BM_LAYER_SOLID_IX   0x10001
#define BM_LAYER_IMAGE_IX   0x10002
#define BM_LAYER_NULL_IX    0x10004
#define BM_LAYER_SHAPE_IX   0x10008
#define BM_LAYER_TEXT_IX    0x1000f

class BMLayer : public BMBase {
public:
	enum MatteClipMode {NoClip, Alpha, InvertedAlpha, Luminence, InvertedLuminence};

	BMLayer(BMBase *parent);
	BMLayer(BMBase *parent, const BMLayer &other);
	~BMLayer() override;

	static BMLayer *construct(BMBase *parent, JsonObject definition);

	bool active(int frame) const override;

	void parse(const JsonObject &definition) override;

	void updateProperties(int frame) override;

	bool isClippedLayer() const;
	bool isMaskLayer() const;
	MatteClipMode clipMode() const;

	int layerId() const;
	void renderFullTransform(Renderer &renderer, int frame) const;

protected:
	void renderEffects(Renderer &renderer, int frame) const;

	virtual BMLayer *resolveLinkedLayer();
	virtual BMLayer *linkedLayer() const;

	int m_layerIndex = 0;
	int m_startFrame;
	int m_endFrame;
	qreal m_startTime;
	int m_blendMode;
	bool m_3dLayer = false;
	BMBase *m_effects = nullptr;
	qreal m_stretch;
	BMBasicTransform m_layerTransform;
	BMMasks *m_masks = nullptr;

	int m_parentLayer = 0;
	int m_td = 0;
	MatteClipMode m_clipMode = NoClip;

	bool m_updated = false;

private:
	void parseEffects(const JsonArray &definition, BMBase *effectRoot = nullptr);
	void parseMasks(const JsonArray &definition);

	BMLayer *m_linkedLayer = nullptr;

};

} // namespace Lottie
