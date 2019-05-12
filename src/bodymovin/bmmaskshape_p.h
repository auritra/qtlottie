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

#ifndef BMMASKSHAPE_P_H
#define BMMASKSHAPE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QPainterPath>
#include <QJsonArray>

#include <QtBodymovin/bmglobal.h>
#include <QtBodymovin/private/bmshape_p.h>
#include <QtBodymovin/private/bmtrimpath_p.h>
#include <QtBodymovin/private/lottierenderer_p.h>

QT_BEGIN_NAMESPACE

class QJsonObject;

class BODYMOVIN_EXPORT BMMaskShape : public BMShape
{
public:
	BMMaskShape();
    explicit BMMaskShape(const BMMaskShape &other);
	BMMaskShape(const QJsonObject &definition, BMBase *parent = nullptr);

    BMBase *clone() const override;

    void construct(const QJsonObject &definition);

    void updateProperties(int frame) override;
    void render(LottieRenderer &renderer, int frame) const override;

	enum class Mode {
		Additive,
		Intersect,
	};

	Mode mode() const;

	bool inverted() const;

protected:
    struct VertexInfo {
        BMProperty2D<QPointF> pos;
        BMProperty2D<QPointF> ci;
        BMProperty2D<QPointF> co;
    };

    void parseShapeKeyframes(QJsonObject &keyframes);
    void buildShape(const QJsonObject &keyframe);
    void buildShape(int frame);
    void parseEasedVertices(const QJsonObject &keyframe, int startFrame);

    QHash<int, QJsonObject> m_vertexMap;
    QList<VertexInfo> m_vertexList;
    QMap<int, bool> m_closedShape;

	bool m_inverted = false;
	BMProperty<qreal> m_opacity;
	Mode m_mode = Mode::Additive;

private:
    struct VertexBuildInfo
    {
        QJsonArray posKeyframes;
        QJsonArray ciKeyframes;
        QJsonArray coKeyframes;
    };

    void finalizeVertices();

    QMap<int, VertexBuildInfo*> m_vertexInfos;

    QJsonObject createKeyframe(QJsonArray startValue, QJsonArray endValue,
                               int startFrame, QJsonObject easingIn,
                               QJsonObject easingOut);
};

QT_END_NAMESPACE

#endif // BMMASKSHAPE_P_H
