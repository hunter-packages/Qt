/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Copyright (C) 2016 Ruslan Baratov
** Contact: http://www.qt.io/licensing/
**
** This file is part of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
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
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qandroidviewfindersettingscontrol.h"
#include "qandroidcamerasession.h"

QT_BEGIN_NAMESPACE

QAndroidViewfinderSettingsControl2::QAndroidViewfinderSettingsControl2(QAndroidCameraSession *session)
    : m_cameraSession(session)
{
}

QList<QCameraViewfinderSettings> QAndroidViewfinderSettingsControl2::supportedViewfinderSettings() const
{
    QList<QCameraViewfinderSettings> viewfinderSettings;

    QList<QSize> previewSizes = m_cameraSession->getSupportedPreviewSizes();
    QList<QVideoFrame::PixelFormat> pixelFormats = m_cameraSession->getSupportedPixelFormats();
    QList<AndroidCamera::FpsRange> fpsRanges = m_cameraSession->getSupportedPreviewFpsRange();

    for (int i = 0; i < previewSizes.size(); ++i) {
        for (int j = 0; j < pixelFormats.size(); ++j) {
            for (int k = 0; k < fpsRanges.size(); ++k) {
                QCameraViewfinderSettings s;
                s.setResolution(previewSizes.at(i));
                s.setPixelAspectRatio(previewSizes.at(i)); // FIXME
                s.setPixelFormat(pixelFormats.at(j));
                s.setMinimumFrameRate(fpsRanges.at(k).getMinReal());
                s.setMaximumFrameRate(fpsRanges.at(k).getMaxReal());
                viewfinderSettings << s;
            }
        }
    }
    return viewfinderSettings;
}

QCameraViewfinderSettings QAndroidViewfinderSettingsControl2::viewfinderSettings() const
{
    QCameraViewfinderSettings settings;
    settings.setResolution(m_cameraSession->viewfinderSize());
    settings.setPixelFormat(m_cameraSession->previewFormat());
    settings.setMinimumFrameRate(m_cameraSession->minimumPreviewFrameRate());
    settings.setMaximumFrameRate(m_cameraSession->maximumPreviewFrameRate());
    return settings;
}

void QAndroidViewfinderSettingsControl2::setViewfinderSettings(const QCameraViewfinderSettings &settings)
{
    m_cameraSession->adjustViewfinderSize(settings.resolution());
    m_cameraSession->setPreviewFormat(settings.pixelFormat());
    m_cameraSession->setPreviewFrameRate(settings.minimumFrameRate(), settings.maximumFrameRate());
}

QT_END_NAMESPACE

#include "moc_qandroidviewfindersettingscontrol.cpp"
