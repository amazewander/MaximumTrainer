/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
****************************************************************************/

#include "stylehelper.h"

#include <QPixmapCache>
#include <QWidget>
#include <QRect>
#include <QPainter>
#include <QApplication>
#include <QPalette>
#include <QStyleOption>
#include <QObject>

qreal StyleHelper::sidebarFontSize()
{
#if defined(Q_OS_MAC)
    return 10;
#else
    return 8;
#endif
}

QColor StyleHelper::panelTextColor(bool lightColored)
{
    //qApp->palette().highlightedText().color();
    if (!lightColored)
        return Qt::white;
    else
        return Qt::black;
}

// Invalid by default, setBaseColor needs to be called at least once
QColor StyleHelper::m_baseColor;
QColor StyleHelper::m_requestedBaseColor;

QColor StyleHelper::baseColor(bool lightColored)
{
    if (!lightColored)
        return m_baseColor;
    else
        return m_baseColor.lighter(230);
}

QColor StyleHelper::borderColor(bool lightColored)
{
    QColor result = baseColor(lightColored);
    result.setHsv(result.hue(),
                  result.saturation(),
                  result.value() / 2);
    return result;
}

// We try to ensure that the actual color used are within
// reasonalbe bounds while generating the actual baseColor
// from the users request.
void StyleHelper::setBaseColor(const QColor &newcolor)
{
    m_requestedBaseColor = newcolor;

    QColor color;
    color.setHsv(newcolor.hue(),
                 newcolor.saturation() * 0.7,
                 64 + newcolor.value() / 3);

    if (color.isValid() && color != m_baseColor) {
        m_baseColor = color;
        foreach (QWidget *w, QApplication::topLevelWidgets())
            w->update();
    }
}


// Draws a cached pixmap with shadow
void StyleHelper::drawIconWithShadow(const QIcon &icon, const QRect &rect,
                                     QPainter *p, QIcon::Mode iconMode, int radius, const QColor &color, const QPoint &offset)
{
    QPixmap cache;
    QString pixmapName = QString::fromLatin1("icon %0 %1 %2").arg(icon.cacheKey()).arg(iconMode).arg(rect.height());

    if (!QPixmapCache::find(pixmapName, cache)) {
        QPixmap px = icon.pixmap(rect.size());
        cache = QPixmap(px.size() + QSize(radius * 2, radius * 2));
        cache.fill(Qt::transparent);

        QPainter cachePainter(&cache);
        if (iconMode == QIcon::Disabled) {
            QImage im = px.toImage().convertToFormat(QImage::Format_ARGB32);
            for (int y=0; y<im.height(); ++y) {
                QRgb *scanLine = (QRgb*)im.scanLine(y);
                for (int x=0; x<im.width(); ++x) {
                    QRgb pixel = *scanLine;
                    char intensity = qGray(pixel);
                    *scanLine = qRgba(intensity, intensity, intensity, qAlpha(pixel));
                    ++scanLine;
                }
            }
            px = QPixmap::fromImage(im);
        }

        // Draw shadow
        QImage tmp(px.size() + QSize(radius * 2, radius * 2 + 1), QImage::Format_ARGB32_Premultiplied);
        tmp.fill(Qt::transparent);

        QPainter tmpPainter(&tmp);
        tmpPainter.setCompositionMode(QPainter::CompositionMode_Source);
        tmpPainter.drawPixmap(QPoint(radius, radius), px);
        tmpPainter.end();

        // blur the alpha channel
        QImage blurred(tmp.size(), QImage::Format_ARGB32_Premultiplied);
        blurred.fill(Qt::transparent);
        QPainter blurPainter(&blurred);
        qt_blurImage(&blurPainter, tmp, radius, false, true);
        blurPainter.end();

        tmp = blurred;

        // blacken the image...
        tmpPainter.begin(&tmp);
        tmpPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        tmpPainter.fillRect(tmp.rect(), color);
        tmpPainter.end();

        tmpPainter.begin(&tmp);
        tmpPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        tmpPainter.fillRect(tmp.rect(), color);
        tmpPainter.end();

        // draw the blurred drop shadow...
        cachePainter.drawImage(QRect(0, 0, cache.rect().width(), cache.rect().height()), tmp);

        // Draw the actual pixmap...
        cachePainter.drawPixmap(QPoint(radius, radius) + offset, px);
        QPixmapCache::insert(pixmapName, cache);
    }

    QRect targetRect = cache.rect();
    targetRect.moveCenter(rect.center());
    p->drawPixmap(targetRect.topLeft() - offset, cache);
}
