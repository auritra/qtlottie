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
#include "bmshapelayer.h"

#include "bmbase.h"
#include "bmshape.h"
#include "bmtrimpath.h"
#include "bmbasictransform.h"
#include "bmmasks.h"
#include "renderer.h"

namespace Lottie {

BMShapeLayer::BMShapeLayer(BMBase *parent) : BMLayer(parent) {
}

BMShapeLayer::BMShapeLayer(BMBase *parent, const BMShapeLayer &other)
: BMLayer(parent, other)
, m_appliedTrim(other.m_appliedTrim) {
}

BMShapeLayer::BMShapeLayer(BMBase *parent, const JsonObject &definition)
: BMLayer(parent) {
	m_type = BM_LAYER_SHAPE_IX;

	BMLayer::parse(definition);
	if (m_hidden) {
		return;
	}

	const auto items = definition.value("shapes").toArray();
	auto it = items.end();
	while (it != items.begin()) {
		const auto shape = BMShape::construct(this, (*--it).toObject());
		if (shape) {
			appendChild(shape);
		}
	}
}

BMShapeLayer::~BMShapeLayer() = default;

BMBase *BMShapeLayer::clone(BMBase *parent) const {
	return new BMShapeLayer(parent, *this);
}

void BMShapeLayer::updateProperties(int frame) {
	if (m_updated) {
		return;
	}

	BMLayer::updateProperties(frame);

	for (BMBase *child : children()) {
		if (!child->active(frame)) {
			continue;
		}

		BMShape *shape = dynamic_cast<BMShape*>(child);

		if (!shape) {
			continue;
		}

		if (shape->type() == BM_SHAPE_TRIM_IX) {
			BMTrimPath *trim = static_cast<BMTrimPath*>(shape);
			if (m_appliedTrim) {
				m_appliedTrim->applyTrim(*trim);
			} else {
				m_appliedTrim = trim;
			}
		} else if (m_appliedTrim) {
			if (shape->acceptsTrim()) {
				shape->applyTrim(*m_appliedTrim);
			}
		}
	}
}

void BMShapeLayer::render(Renderer &renderer, int frame) const {
	renderer.saveState();

	renderEffects(renderer, frame);

	renderer.render(*this);

	if (BMLayer *ll = linkedLayer()) {
		ll->renderFullTransform(renderer, frame);
	}
	m_layerTransform.render(renderer, frame);

	if (m_masks) {
		m_masks->render(renderer, frame);
	}

	for (BMBase *child : children()) {
		if (child->active(frame)) {
			child->render(renderer, frame);
		}
	}

	if (m_appliedTrim && m_appliedTrim->active(frame)) {
		m_appliedTrim->render(renderer, frame);
	}

	renderer.restoreState();
}

} // namespace Lottie
