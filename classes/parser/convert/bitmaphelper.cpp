/*
 * LCD Image Converter. Converts images and fonts for embedded applications.
 * Copyright (C) 2010 riuson
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

#include "bitmaphelper.h"
#include "limits"

#include <QPainter>
#include <QPainterPath>
#include <QPainterPathStroker>
#include <QtSvg/QSvgRenderer>
#include <QDebug>

namespace Parsing
{
namespace Conversion
{

QImage BitmapHelper::rotate90(const QImage *source)
{
  QImage result = QImage(source->height(), source->width(), source->format());
  result.fill(QColor(0, 0, 0, 0));
  QPainter painter(&result);
  painter.translate(QPoint(result.width(), 0));
  painter.rotate(90);
  painter.drawImage(0, 0, *source);
  return result;
}

QImage BitmapHelper::rotate180(const QImage *source)
{
  QImage result = QImage(source->width(), source->height(), source->format());
  result.fill(QColor(0, 0, 0, 0));
  QPainter painter(&result);
  painter.translate(QPoint(result.width(), result.height()));
  painter.rotate(180);
  painter.drawImage(0, 0, *source);
  return result;
}

QImage BitmapHelper::rotate270(const QImage *source)
{
  QImage result = QImage(source->height(), source->width(), source->format());
  result.fill(QColor(0, 0, 0, 0));
  QPainter painter(&result);
  painter.translate(QPoint(0, result.height()));
  painter.rotate(270);
  painter.drawImage(0, 0, *source);
  return result;
}

QImage BitmapHelper::shiftUp(const QImage *source)
{
  QImage result = QImage(source->width(), source->height(), source->format());
  result.fill(QColor(0, 0, 0, 0));
  QPainter painter(&result);
  painter.drawImage(0, -1, *source);
  painter.drawImage(0, source->height() - 1, *source);
  return result;
}

QImage BitmapHelper::shiftRight(const QImage *source)
{
  QImage result = QImage(source->width(), source->height(), source->format());
  result.fill(QColor(0, 0, 0, 0));
  QPainter painter(&result);
  painter.drawImage(1, 0, *source);
  painter.drawImage(-source->width() + 1, 0, *source);
  return result;
}

QImage BitmapHelper::shiftDown(const QImage *source)
{
  QImage result = QImage(source->width(), source->height(), source->format());
  result.fill(QColor(0, 0, 0, 0));
  QPainter painter(&result);
  painter.drawImage(0, 1, *source);
  painter.drawImage(0, -source->height() + 1, *source);
  return result;
}

QImage BitmapHelper::shiftLeft(const QImage *source)
{
  QImage result = QImage(source->width(), source->height(), source->format());
  result.fill(QColor(0, 0, 0, 0));
  QPainter painter(&result);
  painter.drawImage(-1, 0, *source);
  painter.drawImage(source->width() - 1, 0, *source);
  return result;
}

QImage BitmapHelper::flipHorizontal(const QImage *source)
{
  QImage result = source->mirrored(true, false);
  return result;
}

QImage BitmapHelper::flipVertical(const QImage *source)
{
  QImage result = source->mirrored(false, true);
  return result;
}

QImage BitmapHelper::crop(const QImage *source, int left, int top, int right, int bottom, const QColor &backColor)
{
  int sourceWidth = source->width();
  int sourceHeight = source->height();

  // expanded/cropped size
  // positive = expand, negative = crop
  int resultWidth = sourceWidth + left + right;
  int resultHeight = sourceHeight + top + bottom;

  if (resultWidth < 1) {
    resultWidth = 1;
  }

  if (resultHeight < 1) {
    resultHeight = 1;
  }

  QImage result = QImage(resultWidth, resultHeight, source->format());
  result.fill(backColor);

  QPainter painter(&result);
  painter.drawImage(left, top, *source);

  return result;
}

void BitmapHelper::findEmptyArea(const QImage *source, int *left, int *top, int *right, int *bottom)
{
  QRgb background = BitmapHelper::detectBackgroundColor(source).rgba();
  // max possible values by default
  int l = std::numeric_limits<int>::max();
  int t = std::numeric_limits<int>::max();
  int r = 0;
  int b = 0;
  bool picFound = false;

  // search from left/top to bottom/right
  for (int x = 0; x < source->width(); x++) {
    for (int y = 0; y < source->height(); y++) {
      if (source->pixel(x, y) != background) {
        l = qMin(l, x);
        t = qMin(t, y);
        r = qMax(r, x);
        b = qMax(b, y);
        picFound = true;
      }
    }
  }

  if (picFound) {
    *left = l;
    *top = t;
    *right = source->width() - 1 - r;
    *bottom = source->height() - 1 - b;
  } else {
    *left = 0;
    *top = 0;
    *right = 0;
    *bottom = 0;
  }
}

QImage BitmapHelper::scale(const QImage *source, int scale, bool smooth)
{
  int width = source->width();
  int height = source->height();

  QImage result = source->scaled(width * scale, height * scale, Qt::KeepAspectRatio, Qt::FastTransformation);

  QRgb background = BitmapHelper::detectBackgroundColor(source).rgba();

  if (smooth && scale > 1) {
    for (int16_t x = 0; x < width; x++) {
      for (int16_t y = 0; y < height; y++) {
        QRgb current = source->pixel(x, y);

        if (current == background) {

          // Check top left diagonal pixel
          if (x > 0 && y > 0) {
            QRgb top = source->pixel(x, y - 1);
            QRgb left = source->pixel(x - 1, y);
            QRgb topLeft = source->pixel(x - 1, y - 1);

            if (left != background && top != background) {
              for (int j = 0; j < scale - 1; j++) {
                for (int i = 0; i < scale - 1 - j; i++) {
                  result.setPixel(scale * x + i, scale * y + j, topLeft == background ? left : topLeft);
                }
              }
            }
          }

          // Check top right diagonal pixel
          if (x < width - 1 && y > 0) {
            QRgb top = source->pixel(x, y - 1);
            QRgb right = source->pixel(x + 1, y);
            QRgb topRight = source->pixel(x + 1, y + 1);

            if (right != background && top != background) {
              for (int j = 0; j < scale - 1; j++) {
                for (int i = 0; i < scale - 1 - j; i++) {
                  result.setPixel(scale * x + (scale - 1) - i, scale * y + j, topRight == background ? right : topRight);
                }
              }
            }
          }

          // Check bottom left diagonal pixel
          if (x > 0 && y < height - 1) {
            QRgb top = source->pixel(x, y + 1);
            QRgb left = source->pixel(x - 1, y);
            QRgb bottomLeft = source->pixel(x - 1, y + 1);

            if (left != background && top != background) {
              for (int j = 0; j < scale - 1; j++) {
                for (int i = 0; i < scale - 1 - j; i++) {
                  result.setPixel(scale * x + i, scale * y + (scale - 1) - j, bottomLeft == background ? left : bottomLeft);
                }
              }
            }
          }

          // Check bottom right diagonal pixel
          if (x < width - 1 && y < height - 1) {
            QRgb bottom = source->pixel(x, y + 1);
            QRgb right = source->pixel(x + 1, y);
            QRgb bottomRight = source->pixel(x + 1, y + 1);

            if (right != background && bottom != background) {
              for (int j = 0; j < scale - 1; j++) {
                for (int i = 0; i < scale - 1 - j; i++) {
                  result.setPixel(scale * x + (scale - 1) - i, scale * y + (scale - 1) - j, bottomRight == background ? right : bottomRight);
                }
              }
            }
          }

        }
      }
    }
  }

  return result;
}

QImage BitmapHelper::drawGrid(const QImage *source, int scale)
{
  QImage result = QImage(*source);

  if (scale > 5) {
    QPixmap pixmap = QPixmap::fromImage(result);

    QPainter painter(&pixmap);

    painter.setPen(QColor("silver"));

    for (int x = 0; x < pixmap.width(); x += scale) {
      painter.drawLine(x, 0, x, pixmap.height());
    }

    painter.drawLine(pixmap.width() - 1, 0, pixmap.width() - 1, pixmap.height());

    for (int y = 0; y < pixmap.height(); y += scale) {
      painter.drawLine(0, y, pixmap.width(), y);
    }

    painter.drawLine(0, pixmap.height() - 1, pixmap.width(), pixmap.height() - 1);

    result = pixmap.toImage();
  }

  return result;
}

QImage BitmapHelper::drawSelection(const QImage *source, const QPainterPath &selectedPath, int scale)
{
  QImage image = *source;
  QPixmap pixmap = QPixmap::fromImage(image);
  QPainter painter(&pixmap);

  QColor selectionBorderColor = QColor(0, 0, 248, 128);
  QBrush selectionFillBrush(QColor(64, 128, 248, 128));

  QTransform transform;
  transform = transform.scale((float)scale, (float)scale);
  painter.setTransform(transform);

  painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
  painter.setRenderHint(QPainter::Antialiasing, false);
  painter.setRenderHint(QPainter::HighQualityAntialiasing, false);
  painter.setPen(selectionBorderColor);
  painter.setBrush(selectionFillBrush);

  painter.fillPath(selectedPath, selectionFillBrush);

  if (scale > 5) {
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::HighQualityAntialiasing, true);

    QPainterPathStroker stroker;
    float offset = 1.0f / (float)scale;
    stroker.setWidth(offset * 2.0f);
    stroker.setJoinStyle(Qt::RoundJoin);
    stroker.setDashPattern(Qt::DashLine);

    QPainterPath strokedPath = stroker.createStroke(selectedPath);
    painter.fillPath(strokedPath, QBrush(selectionBorderColor));
  }

  return pixmap.toImage();
}

QImage BitmapHelper::drawPixel(const QImage *source, int x, int y, const QColor &color)
{
  QPixmap pixmap = QPixmap::fromImage(*source);

  QPainter painter(&pixmap);
  painter.setPen(color);
  painter.drawPoint(x, y);

  return pixmap.toImage();
}

QColor BitmapHelper::detectBackgroundColor(const QImage *image)
{
  QColor color1 = BitmapHelper::fromRgba(image->pixel(0, 0));
  QColor color2 = BitmapHelper::fromRgba(image->pixel(image->width() - 1, 0));
  QColor color3 = BitmapHelper::fromRgba(image->pixel(0, image->height() - 1));
  QColor color4 = BitmapHelper::fromRgba(image->pixel(image->width() - 1, image->height() - 1));

  int c1 = 0, c2 = 0, c3 = 0, c4 = 0;

  c1 += color1 == color2 ? 1 : 0;
  c1 += color1 == color3 ? 1 : 0;
  c1 += color1 == color4 ? 1 : 0;

  c2 += color2 == color1 ? 1 : 0;
  c2 += color2 == color3 ? 1 : 0;
  c2 += color2 == color4 ? 1 : 0;

  c3 += color3 == color1 ? 1 : 0;
  c3 += color3 == color2 ? 1 : 0;
  c3 += color3 == color4 ? 1 : 0;

  c4 += color4 == color1 ? 1 : 0;
  c4 += color4 == color2 ? 1 : 0;
  c4 += color4 == color3 ? 1 : 0;

  int cmax = qMax(qMax(c3, c4), qMax(c1, c2));

  if (cmax == c3) {
    return color3;
  } else if (cmax == c4) {
    return color4;
  } else if (cmax == c1) {
    return color1;
  } else {
    return color2;
  }
}

QImage BitmapHelper::fromSvg(const QString &path, int size)
{
  QSvgRenderer renderer(path);

  QImage image(size, size, QImage::Format_ARGB32);
  image.fill(0x00ffffff);
  QPainter painter(&image);
  renderer.render(&painter);

  return image;
}

QColor BitmapHelper::fromRgba(QRgb value)
{
  return QColor(qRed(value), qGreen(value), qBlue(value), qAlpha(value));
}

} // namespace Conversion
} // namespace Parsing
