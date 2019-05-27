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

#include "bmshape.h"
#include "freeformshape.h"

namespace Lottie {

class BMFreeFormShape : public BMShape {
public:
	BMFreeFormShape(BMBase *parent);
	BMFreeFormShape(BMBase *parent, const BMFreeFormShape &other);
	BMFreeFormShape(BMBase *parent, const JsonObject &definition);

	BMBase *clone(BMBase *parent) const override;

	void parse(const JsonObject &definition) override;

	void updateProperties(int frame) override;
	void render(Renderer &renderer, int frame) const override;

	bool acceptsTrim() const override;

protected:
	FreeFormShape m_shape;

};

} // namespace Lottie
