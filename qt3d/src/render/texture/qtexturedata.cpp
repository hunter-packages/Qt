/****************************************************************************
**
** Copyright (C) 2014 Klaralvdalens Datakonsult AB (KDAB).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Qt3D module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qtexturedata_p.h"

#include <QDebug>
#include <QFileInfo>
#include <QFile>
#include <qendian.h>

QT_BEGIN_NAMESPACE

namespace Qt3DRender {

QTexImageDataPrivate::QTexImageDataPrivate()
    : m_width(-1)
    , m_height(-1)
    , m_depth(-1)
    , m_isCompressed(false)
    , m_format(QOpenGLTexture::RGBA8_UNorm)
{
}

QTexImageData::QTexImageData()
    : d_ptr(new QTexImageDataPrivate())
{
}

QTexImageData::QTexImageData(QTexImageDataPrivate &dd)
    : d_ptr(&dd)
{
}

QTexImageData::~QTexImageData()
{
    cleanup();
    delete d_ptr;
}

QTexImageData &QTexImageData::operator=(const QTexImageData &other)
{
    Q_D(QTexImageData);
    d->m_width = other.d_ptr->m_width;
    d->m_height = other.d_ptr->m_height;
    d->m_depth = other.d_ptr->m_depth;
    d->m_isCompressed = other.d_ptr->m_isCompressed;
    d->m_pixelFormat = other.d_ptr->m_pixelFormat;
    d->m_pixelType = other.d_ptr->m_pixelType;
    d->m_data = other.d_ptr->m_data;

    return *this;
}

void QTexImageData::cleanup()
{
    Q_D(QTexImageData);
    d->m_width = -1;
    d->m_height = -1;
    d->m_depth = -1;
    d->m_isCompressed = false;
    d->m_data.clear();
}

bool QTexImageData::isCompressed() const
{
    Q_D(const QTexImageData);
    return d->m_isCompressed;
}

int QTexImageData::width() const
{
    Q_D(const QTexImageData);
    return d->m_width;
}

int QTexImageData::height() const
{
    Q_D(const QTexImageData);
    return d->m_height;
}

int QTexImageData::depth() const
{
    Q_D(const QTexImageData);
    return d->m_depth;
}

QOpenGLTexture::TextureFormat QTexImageData::format() const
{
    Q_D(const QTexImageData);
    return d->m_format;
}

void QTexImageData::setImage(const QImage &image)
{
    Q_D(QTexImageData);
    d->m_width = image.width();
    d->m_height = image.height();
    d->m_depth = 1;

    QImage glImage = image.convertToFormat(QImage::Format_RGBA8888);
    Q_ASSERT_X(glImage.bytesPerLine() == (glImage.width() * glImage.depth() + 7) / 8,
               "QTexImageData::setImage", "glImage is not packed"); // QTBUG-48330
    QByteArray imageBytes((const char*) glImage.constBits(), glImage.byteCount());
    setData(imageBytes, QOpenGLTexture::RGBA, QOpenGLTexture::UInt8);
    d->m_format = QOpenGLTexture::RGBA8_UNorm;
}

void QTexImageData::setData(const QByteArray &data, QOpenGLTexture::PixelFormat fmt,
                            QOpenGLTexture::PixelType ptype)
{
    Q_D(QTexImageData);
    d->m_isCompressed = false;
    d->m_data = data;
    d->m_pixelFormat = fmt;
    d->m_pixelType = ptype;
}

bool QTexImageData::setCompressedFile(const QString &source)
{
    Q_D(QTexImageData);
    const bool isPkm = QFileInfo(source).suffix() == QStringLiteral("pkm");
    if (!isPkm)
        return false;

    QFile f(source);
    if (!f.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open" << source;
        return false;
    }

    // ETC1 in PKM, as generated by f.ex. Android's etc1tool
    static const char pkmMagic[] = { 'P', 'K', 'M', ' ', '1', '0' };
    const int pkmHeaderSize = 6 + 2 + 4 * 2;
    QByteArray header = f.read(pkmHeaderSize);
    if (header.size() >= pkmHeaderSize && !qstrncmp(header.constData(), pkmMagic, 6)) {
        d->m_format = QOpenGLTexture::RGB8_ETC1; // may get changed to RGB8_ETC2 later on
        // get the extended (multiple of 4) width and height
        d->m_width = qFromBigEndian(*(reinterpret_cast<const quint16 *>(header.constData() + 6 + 2)));
        d->m_height = qFromBigEndian(*(reinterpret_cast<const quint16 *>(header.constData() + 6 + 2 + 2)));
        d->m_depth = 1;
        QByteArray data = f.readAll();
        if (data.size() < (d->m_width / 4) * (d->m_height / 4) * 8)
            qWarning() << "Unexpected end of ETC1 data in" << source;
        setData(data, QOpenGLTexture::RGBA, QOpenGLTexture::UInt8);
        d->m_isCompressed = true;
        return true;
    }

    return false;
}

QByteArray QTexImageData::data() const
{
    Q_D(const QTexImageData);
    return d->m_data;
}

QOpenGLTexture::PixelFormat QTexImageData::pixelFormat() const
{
    Q_D(const QTexImageData);
    return d->m_pixelFormat;
}

QOpenGLTexture::PixelType QTexImageData::pixelType() const
{
    Q_D(const QTexImageData);
    return d->m_pixelType;
}

} // namespace Qt3DRender

QT_END_NAMESPACE