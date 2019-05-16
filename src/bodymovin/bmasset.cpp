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

#include "bmasset.h"

#include "bmprecompasset.h"
#include "json.h"

namespace Lottie {

BMAsset::BMAsset(BMBase *parent) : BMBase(parent) {
}

BMAsset::BMAsset(BMBase *parent, const BMAsset &other)
: BMBase(parent, other)
, m_id(other.m_id)
, m_resolved(other.m_resolved) {
}

BMAsset *BMAsset::clone(BMBase *parent) const {
	return new BMAsset(parent, *this);
}

BMAsset *BMAsset::construct(BMBase *parent, JsonObject definition) {
	BMAsset *asset = nullptr;
	if (definition.contains("layers")) {
		asset = new BMPreCompAsset(parent, definition);
	}
	return asset;
}

void BMAsset::parse(const JsonObject &definition) {
	BMBase::parse(definition);
	if (m_hidden) {
		return;
	}

	m_id = definition.value("id").toString();
}

void BMAsset::resolveAssets(
		const std::function<BMAsset*(BMBase*, QByteArray)> &resolver) {
	if (!m_resolved) {
		m_resolved = true;
		BMBase::resolveAssets(resolver);
	}
}

QByteArray BMAsset::id() const {
	return m_id;
}

} // namespace Lottie
