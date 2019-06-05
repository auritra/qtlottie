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
#include "bmscene.h"

#include "bmasset.h"
#include "bmlayer.h"

namespace Lottie {
namespace {

constexpr auto kMaxFrameRate = 120;
constexpr auto kMaxSize = 3096;

} // namespace

BMScene::BMScene(const JsonObject &definition) : BMBase(nullptr) {
	parse(definition);
}

BMScene::~BMScene() {
}

BMBase *BMScene::clone(BMBase *parent) const {
	return nullptr;
}

BMScene *BMScene::resolveTopRoot() const {
	return const_cast<BMScene*>(this);
}

bool BMScene::isValid() const {
	return (_blueprint != nullptr)
		&& (_endFrame > _startFrame)
		&& (_frameRate > 0)
		&& (_frameRate <= kMaxFrameRate)
		&& (_width > 0)
		&& (_width <= kMaxSize)
		&& (_height > 0)
		&& (_height <= kMaxSize);
}

int BMScene::startFrame() const {
	return _startFrame;
}

int BMScene::endFrame() const {
	return _endFrame;
}

int BMScene::frameRate() const {
	return _frameRate;
}

int BMScene::width() const {
	return _width;
}

int BMScene::height() const {
	return _height;
}

void BMScene::parse(const JsonObject &definition) {
	_parsing = true;

	_startFrame = definition.value("ip").toInt();
	_endFrame = definition.value("op").toInt();
	_frameRate = definition.value("fr").toInt();
	_width = definition.value("w").toInt();
	_height = definition.value("h").toInt();

	const auto assets = definition.value("assets").toArray();
	for (const auto &entry : assets) {
		if (const auto asset = BMAsset::construct(this, entry.toObject())) {
			_assetIndexById.insert(asset->id(), _assets.size());
			_assets.emplace_back(asset);
		} else {
			_unsupported = true;
		}
	}

	if (!definition.value("chars").toArray().empty()) {
		_unsupported = true;
	}

	_blueprint = std::make_unique<BMBase>(this);
	const auto layers = definition.value("layers").toArray();
	for (auto i = layers.end(); i != layers.begin();) {
		const auto &entry = *(--i);
		if (const auto layer = BMLayer::construct(_blueprint.get(), entry.toObject())) {
			// Mask layers must be rendered before the layers they affect to
			// although they appear before in layer hierarchy. For this reason
			// move a mask after the affected layers, so it will be rendered first
			if (layer->isMaskLayer()) {
				_blueprint->prependChild(layer);
			} else {
				_blueprint->appendChild(layer);
			}
		} else {
			_unsupported = true;
		}
	}

	resolveAllAssets();

	_parsing = false;
}

void BMScene::updateProperties(int frame) {
	_current.reset(_blueprint->clone(this));
	_current->updateProperties(frame);
}

void BMScene::render(Renderer &renderer, int frame) const {
	Q_ASSERT(_current);
	_current->render(renderer, frame);
}

void BMScene::resolveAllAssets() {
	if (_assets.empty()) {
		return;
	}

	std::function<BMAsset*(BMBase*, QString)> resolver = [&](BMBase *parent, const QString &refId)
		-> BMAsset * {
		const auto i = _assetIndexById.constFind(refId);
		if (i == _assetIndexById.constEnd()) {
			return nullptr;
		}
		const auto result = _assets[i.value()].get();
		result->resolveAssets(resolver);
		return result->clone(parent);
	};
	for (const auto &asset : _assets) {
		asset->resolveAssets(resolver);
	}

	_blueprint->resolveAssets([&](BMBase *parent, const QString &refId) {
		const auto i = _assetIndexById.constFind(refId);
		return (i != _assetIndexById.constEnd())
			? _assets[i.value()]->clone(parent)
			: nullptr;
	});
}

} // namespace Lottie
