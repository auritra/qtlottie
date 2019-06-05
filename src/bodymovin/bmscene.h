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

#include <QHash>
#include <vector>
#include <memory>

namespace Lottie {

class BMAsset;

class BMScene : public BMBase {
public:
	BMScene(const BMScene &other) = delete;
	BMScene &operator=(const BMScene &other) = delete;
	explicit BMScene(const JsonObject &definition);
	virtual ~BMScene();

	BMBase *clone(BMBase *parent) const override;

	void updateProperties(int frame) override;
	void render(Renderer &renderer, int frame) const override;

	bool isValid() const;
	int startFrame() const;
	int endFrame() const;
	int frameRate() const;
	int width() const;
	int height() const;

protected:
	BMScene *resolveTopRoot() const override;

private:
	void parse(const JsonObject &definition) override;
	void resolveAllAssets();

	std::vector<std::unique_ptr<BMAsset>> _assets;
	QHash<QString, int> _assetIndexById;

	std::unique_ptr<BMBase> _blueprint;
	std::unique_ptr<BMBase> _current;

	int _startFrame = 0;
	int _endFrame = 0;
	int _frameRate = 0;
	int _width = 0;
	int _height = 0;
	QHash<QString, int> _markers;

	bool _unsupported = false;

	// Parsing stage.
	bool _parsing = false;

};

} // namespace Lottie
