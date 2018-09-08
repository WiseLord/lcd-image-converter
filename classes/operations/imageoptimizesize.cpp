/*
 * LCD Image Converter. Converts images and fonts for embedded applications.
 * Copyright (C) 2016 riuson
 * mailto: riuson@gmail.com
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/
 */

#include "imageoptimizesize.h"
#include <QImage>
#include "idocument.h"
#include "datacontainer.h"
#include "bitmaphelper.h"

namespace Operations
{

ImageOptimizeSize::ImageOptimizeSize(QObject *parent)
  : QObject(parent)
{
  this->mMode = Mode::None;
}

bool ImageOptimizeSize::prepare(const Data::Containers::IDocument *doc, const QStringList &keys)
{
    Q_UNUSED(doc)
    Q_UNUSED(keys)

    return true;
}

void ImageOptimizeSize::applyDocument(Data::Containers::IDocument *doc, const QStringList &keys)
{
    Q_UNUSED(doc)
    Q_UNUSED(keys)
}

void ImageOptimizeSize::applyItem(Data::Containers::IDocument *doc, const QString &itemKey)
{
    const QImage *original = doc->dataContainer()->image(itemKey);
    QImage result;

    int left, right, top, bottom;

    Parsing::Conversion::BitmapHelper::findEmptyArea(original, &left, &top, &right, &bottom);
    QColor backgroundColor = Parsing::Conversion::BitmapHelper::detectBackgroundColor(original);

    switch (this->mMode) {
    case Mode::Width:
      result = Parsing::Conversion::BitmapHelper::crop(original, -left, 0, -right, 0, backgroundColor);
      break;
    case Mode::Height:
      result = Parsing::Conversion::BitmapHelper::crop(original, 0, -top, 0, -bottom, backgroundColor);
      break;
    case Mode::Size:
      result = Parsing::Conversion::BitmapHelper::crop(original, -left, -top, -right, -bottom, backgroundColor);
      break;
    default:
      result = *original;
      break;
    }
    doc->dataContainer()->setImage(itemKey, &result);
}

void ImageOptimizeSize::setMode(ImageOptimizeSize::Mode mode)
{
  this->mMode = mode;
}

} // namespace Operations
