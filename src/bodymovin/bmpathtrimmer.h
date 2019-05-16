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

#include <QList>

namespace Lottie {

class BMTrimPath;
class Renderer;
class BMBase;
class BMShape;

class BMPathTrimmer {
public:
	BMPathTrimmer(BMBase *root);

	void addTrim(BMTrimPath* trim);
	bool inUse() const;

	void applyTrim(BMShape *shape);

	void updateProperties(int frame);
	void render(Renderer &renderer, int frame) const;

private:
	BMBase *m_root = nullptr;

	QList<BMTrimPath*> m_trimPaths;
	BMTrimPath *m_appliedTrim = nullptr;

};

} // namespace Lottie
