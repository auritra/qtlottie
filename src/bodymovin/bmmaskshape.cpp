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
#include "bmmaskshape.h"

#include "bmtrimpath.h"

#include <QJsonObject>

BMMaskShape::BMMaskShape(BMBase *parent) : BMShape(parent) {
}

BMMaskShape::BMMaskShape(BMBase *parent, const BMMaskShape &other)
: BMShape(parent, other)
, m_vertexMap(other.m_vertexMap)
, m_vertexList(other.m_vertexList)
, m_closedShape(other.m_closedShape) {
}

BMMaskShape::BMMaskShape(BMBase *parent, const QJsonObject &definition)
: BMShape(parent) {
	parse(definition);
}

BMBase *BMMaskShape::clone(BMBase *parent) const {
	return new BMMaskShape(parent, *this);
}

void BMMaskShape::parse(const QJsonObject &definition) {
	BMBase::parse(definition);

	m_inverted = definition.value(QStringLiteral("inv")).toBool();

	QJsonObject opacity = definition.value(QStringLiteral("o")).toObject();
	if (opacity.isEmpty()) {
		m_opacity.setValue(100.);
	} else {
		m_opacity.construct(opacity);
	}

	if (m_opacity.value() < 100. || m_opacity.animated()) {
		qWarning() << "Transparent mask shapes are not supported.";
	}

	QString mode = definition.value(QStringLiteral("mode")).toVariant().toString();
	if (mode == QStringLiteral("a")) {
		m_mode = Mode::Additive;
	} else if (mode == QStringLiteral("i")) {
		m_mode = Mode::Intersect;
	} else {
		qWarning() << "Unsupported mask mode.";
	}

	QJsonObject vertexObj = definition.value(QStringLiteral("pt")).toObject();
	if (vertexObj.value(QStringLiteral("a")).toInt()) {
		parseShapeKeyframes(vertexObj);
	} else {
		buildShape(vertexObj.value(QStringLiteral("k")).toObject());
	}
}

void BMMaskShape::updateProperties(int frame) {
	m_opacity.update(frame);
	if (m_vertexMap.count()) {
		QJsonObject keyframe = m_vertexMap.value(frame);
		// If this frame is a keyframe, so values must be updated
		if (!keyframe.isEmpty()) {
			buildShape(keyframe.value(QStringLiteral("s")).toArray().at(0).toObject());
		}
	} else {
		for (int i =0; i < m_vertexList.count(); i++) {
			VertexInfo vi = m_vertexList.at(i);
			vi.pos.update(frame);
			vi.ci.update(frame);
			vi.co.update(frame);
			m_vertexList.replace(i, vi);
		}
		buildShape(frame);
	}
}

void BMMaskShape::render(LottieRenderer &renderer, int frame) const {
	renderer.render(*this);
}

BMMaskShape::Mode BMMaskShape::mode() const {
	return m_mode;
}

bool BMMaskShape::inverted() const {
	return m_inverted;
}

void BMMaskShape::parseShapeKeyframes(QJsonObject &keyframes) {
	QJsonArray vertexKeyframes = keyframes.value(QStringLiteral("k")).toArray();
	for (int i = 0; i < vertexKeyframes.count(); i++) {
		QJsonObject keyframe = vertexKeyframes.at(i).toObject();
		if (keyframe.value(QStringLiteral("h")).toInt()) {
			m_vertexMap.insert(keyframe.value(QStringLiteral("t")).toVariant().toInt(), keyframe);
		} else {
			parseEasedVertices(keyframe, keyframe.value(QStringLiteral("t")).toVariant().toInt());
		}
	}
	if (m_vertexInfos.count()) {
		finalizeVertices();
	}
}

void BMMaskShape::buildShape(const QJsonObject &shape) {
	bool needToClose = shape.value(QStringLiteral("c")).toBool();
	QJsonArray bezierIn = shape.value(QStringLiteral("i")).toArray();
	QJsonArray bezierOut = shape.value(QStringLiteral("o")).toArray();
	QJsonArray vertices = shape.value(QStringLiteral("v")).toArray();

	// If there are less than two vertices, cannot make a bezier curve
	if (vertices.count() < 2) {
		return;
	}

	QPointF s(
		vertices.at(0).toArray().at(0).toDouble(),
		vertices.at(0).toArray().at(1).toDouble());
	QPointF s0(s);

	m_path.moveTo(s);
	int i=0;

	while (i < vertices.count() - 1) {
		QPointF v = QPointF(
			vertices.at(i + 1).toArray().at(0).toDouble(),
			vertices.at(i + 1).toArray().at(1).toDouble());
		QPointF c1 = QPointF(
			bezierOut.at(i).toArray().at(0).toDouble(),
			bezierOut.at(i).toArray().at(1).toDouble());
		QPointF c2 = QPointF(
			bezierIn.at(i + 1).toArray().at(0).toDouble(),
			bezierIn.at(i + 1).toArray().at(1).toDouble());
		c1 += s;
		c2 += v;

		m_path.cubicTo(c1, c2, v);

		s = v;
		i++;
	}

	if (needToClose) {
		QPointF v = s0;
		QPointF c1 = QPointF(
			bezierOut.at(i).toArray().at(0).toDouble(),
			bezierOut.at(i).toArray().at(1).toDouble());
		QPointF c2 = QPointF(
			bezierIn.at(0).toArray().at(0).toDouble(),
			bezierIn.at(0).toArray().at(1).toDouble());
		c1 += s;
		c2 += v;

		m_path.cubicTo(c1, c2, v);
	}

	m_path.setFillRule(Qt::WindingFill);

	if (m_direction) {
		m_path = m_path.toReversed();
	}
}

void BMMaskShape::buildShape(int frame) {
	auto it = m_closedShape.constBegin();
	bool found = false;

	if (frame <= it.key()) {
		found = true;
	} else {
		while (it != m_closedShape.constEnd()) {
			if (it.key() <= frame) {
				found = true;
				break;
			}
			++it;
		}
	}

	bool needToClose = false;
	if (found) {
		needToClose = (*it);
	}

	// If there are less than two vertices, cannot make a bezier curve
	if (m_vertexList.count() < 2) {
		return;
	}

	QPointF s(m_vertexList.at(0).pos.value());
	QPointF s0(s);

	m_path.moveTo(s);
	int i=0;

	while (i < m_vertexList.count() - 1) {
		QPointF v = m_vertexList.at(i + 1).pos.value();
		QPointF c1 = m_vertexList.at(i).co.value();
		QPointF c2 = m_vertexList.at(i + 1).ci.value();
		c1 += s;
		c2 += v;

		m_path.cubicTo(c1, c2, v);

		s = v;
		i++;
	}

	if (needToClose) {
		QPointF v = s0;
		QPointF c1 = m_vertexList.at(i).co.value();
		QPointF c2 = m_vertexList.at(0).ci.value();
		c1 += s;
		c2 += v;

		m_path.cubicTo(c1, c2, v);
	}

	m_path.setFillRule(Qt::WindingFill);

	if (m_direction) {
		m_path = m_path.toReversed();
	}
}

void BMMaskShape::parseEasedVertices(const QJsonObject &keyframe, int startFrame) {
	QJsonObject startValue = keyframe.value(QStringLiteral("s")).toArray().at(0).toObject();
	QJsonObject endValue = keyframe.value(QStringLiteral("e")).toArray().at(0).toObject();
	bool closedPathAtStart = keyframe.value(QStringLiteral("s")).toArray().at(0).toObject().value(QStringLiteral("c")).toBool();
	//bool closedPathAtEnd = keyframe.value(QStringLiteral("e")).toArray().at(0).toObject().value(QStringLiteral("c")).toBool();
	QJsonArray startVertices = startValue.value(QStringLiteral("v")).toArray();
	QJsonArray startBezierIn = startValue.value(QStringLiteral("i")).toArray();
	QJsonArray startBezierOut = startValue.value(QStringLiteral("o")).toArray();
	QJsonArray endVertices = endValue.value(QStringLiteral("v")).toArray();
	QJsonArray endBezierIn = endValue.value(QStringLiteral("i")).toArray();
	QJsonArray endBezierOut = endValue.value(QStringLiteral("o")).toArray();
	QJsonObject easingIn = keyframe.value(QStringLiteral("i")).toObject();
	QJsonObject easingOut = keyframe.value(QStringLiteral("o")).toObject();

	// if there are no vertices for this keyframe, they keyframe
	// is the last one, and it must be processed differently
	if (!startVertices.isEmpty()) {
		for (int i = 0; i < startVertices.count(); i++) {
			VertexBuildInfo *buildInfo = m_vertexInfos.value(i, nullptr);
			if (!buildInfo) {
				buildInfo = new VertexBuildInfo;
				m_vertexInfos.insert(i, buildInfo);
			}
			QJsonObject posKf = createKeyframe(
				startVertices.at(i).toArray(),
				endVertices.at(i).toArray(),
				startFrame,
				easingIn,
				easingOut);
			buildInfo->posKeyframes.push_back(posKf);

			QJsonObject ciKf = createKeyframe(
				startBezierIn.at(i).toArray(),
				endBezierIn.at(i).toArray(),
				startFrame,
				easingIn,
				easingOut);
			buildInfo->ciKeyframes.push_back(ciKf);

			QJsonObject coKf = createKeyframe(
				startBezierOut.at(i).toArray(),
				endBezierOut.at(i).toArray(),
				startFrame,
				easingIn,
				easingOut);
			buildInfo->coKeyframes.push_back(coKf);

			m_closedShape.insert(startFrame, closedPathAtStart);
		}
	} else {
		// Last keyframe

		int vertexCount = m_vertexInfos.count();
		for (int i = 0; i < vertexCount; i++) {
			VertexBuildInfo *buildInfo = m_vertexInfos.value(i, nullptr);
			if (!buildInfo) {
				buildInfo = new VertexBuildInfo;
				m_vertexInfos.insert(i, buildInfo);
			}
			QJsonObject posKf;
			posKf.insert(QStringLiteral("t"), startFrame);
			buildInfo->posKeyframes.push_back(posKf);

			QJsonObject ciKf;
			ciKf.insert(QStringLiteral("t"), startFrame);
			buildInfo->ciKeyframes.push_back(ciKf);

			QJsonObject coKf;
			coKf.insert(QStringLiteral("t"), startFrame);
			buildInfo->coKeyframes.push_back(coKf);

			m_closedShape.insert(startFrame, false);
		}
	}
}

void BMMaskShape::finalizeVertices() {

	for (int i = 0; i < m_vertexInfos.count(); i++) {
		QJsonObject posObj;
		posObj.insert(QStringLiteral("a"), 1);
		posObj.insert(QStringLiteral("k"), m_vertexInfos.value(i)->posKeyframes);

		QJsonObject ciObj;
		ciObj.insert(QStringLiteral("a"), 1);
		ciObj.insert(QStringLiteral("k"), m_vertexInfos.value(i)->ciKeyframes);

		QJsonObject coObj;
		coObj.insert(QStringLiteral("a"), 1);
		coObj.insert(QStringLiteral("k"), m_vertexInfos.value(i)->coKeyframes);

		VertexInfo vertexInfo;
		vertexInfo.pos.construct(posObj);
		vertexInfo.ci.construct(ciObj);
		vertexInfo.co.construct(coObj);
		m_vertexList.push_back(vertexInfo);
	}
	qDeleteAll(m_vertexInfos);
}

QJsonObject BMMaskShape::createKeyframe(
		QJsonArray startValue,
		QJsonArray endValue,
		int startFrame,
		QJsonObject easingIn,
		QJsonObject easingOut) {
	QJsonObject keyframe;
	keyframe.insert(QStringLiteral("t"), startFrame);
	keyframe.insert(QStringLiteral("s"), startValue);
	keyframe.insert(QStringLiteral("e"), endValue);
	keyframe.insert(QStringLiteral("i"), easingIn);
	keyframe.insert(QStringLiteral("o"), easingOut);
	return keyframe;
}
