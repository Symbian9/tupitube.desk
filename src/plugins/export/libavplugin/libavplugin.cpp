/***************************************************************************
 *   Project TUPITUBE DESK                                                *
 *   Project Contact: info@maefloresta.com                                 *
 *   Project Website: http://www.maefloresta.com                           *
 *   Project Leader: Gustav Gonzalez <info@maefloresta.com>                *
 *                                                                         *
 *   Developers:                                                           *
 *   2010:                                                                 *
 *    Gustavo Gonzalez / xtingray                                          *
 *                                                                         *
 *   KTooN's versions:                                                     * 
 *                                                                         *
 *   2006:                                                                 *
 *    David Cuadrado                                                       *
 *    Jorge Cuadrado                                                       *
 *   2003:                                                                 *
 *    Fernado Roldan                                                       *
 *    Simena Dinas                                                         *
 *                                                                         *
 *   Copyright (C) 2010 Gustav Gonzalez - http://www.maefloresta.com       *
 *   License:                                                              *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#include "libavplugin.h"

LibavPlugin::LibavPlugin()
{
}

LibavPlugin::~LibavPlugin()
{
}

QString LibavPlugin::key() const
{
    return tr("Video Formats");
}

TupExportInterface::Formats LibavPlugin::availableFormats()
{
    // SQA: MPEG codec was removed because it crashes. Check the issue.
    // TupExportInterface::MPEG 

    // SQA: Obsolete formats
    // | TupExportInterface::SWF | TupExportInterface::ASF

    return TupExportInterface::WEBM | TupExportInterface::MP4 | TupExportInterface::AVI |
           TupExportInterface::MOV | TupExportInterface::GIF;
}

TMovieGeneratorInterface::Format LibavPlugin::videoFormat(TupExportInterface::Format format)
{
    switch (format) {
        case TupExportInterface::MP4:
            {
                return TLibavMovieGenerator::MP4;
            }
        break;
        case TupExportInterface::GIF:
            {
                return TLibavMovieGenerator::GIF;
            }
        break;
        case TupExportInterface::AVI:
            {
                return TLibavMovieGenerator::AVI;
            }
        break;

        /* SQA: MPEG codec was removed because it crashes. Check the issue
        case TupExportInterface::MPEG:
            {
                return TLibavMovieGenerator::MPEG;
            }
        break;
        */

        case TupExportInterface::MOV:
            {
                return TLibavMovieGenerator::MOV;
            }
        break;
        case TupExportInterface::WEBM:
            {
                return TLibavMovieGenerator::WEBM;
            }
        break;

        /* SQA: Obsolete formats
        case TupExportInterface::SWF:
            {
                return TLibavMovieGenerator::SWF;
            }
        break;
        case TupExportInterface::ASF:
            {
                return TLibavMovieGenerator::ASF;
            }
        break;
        */

        case TupExportInterface::PNG:
        case TupExportInterface::JPEG:
        case TupExportInterface::XPM:
        // case TupExportInterface::SMIL:
        case TupExportInterface::NONE:
            {
                return TLibavMovieGenerator::NONE;
            }
        default:
            {
                return TLibavMovieGenerator::NONE;
            }
        break;
    }

    return TLibavMovieGenerator::NONE;
}

bool LibavPlugin::exportToFormat(const QColor color, const QString &filePath, const QList<TupScene *> &scenes, TupExportInterface::Format fmt, 
                                 const QSize &size, const QSize &newSize, int fps, TupLibrary *library)
{
    Q_UNUSED(newSize);

    qreal duration = 0;
    foreach (TupScene *scene, scenes)
        duration += (qreal) scene->framesCount() / (qreal) fps;

    TMovieGeneratorInterface::Format format = videoFormat(fmt);
    if (format == TLibavMovieGenerator::NONE)
        return false;

    // SQA: Get sound files from library and pass them as QList to TLibavMovieGenerator 
    TLibavMovieGenerator *generator = new TLibavMovieGenerator(format, size, fps, duration);
    TupAnimationRenderer renderer(color, library);
    {
        if (!generator->validMovieHeader()) {
            errorMsg = generator->getErrorMsg();
            #ifdef TUP_DEBUG
                QString msg = "LibavPlugin::exportToFormat() - [ Fatal Error ] - Can't create video -> " + filePath;
                #ifdef Q_OS_WIN
                    qDebug() << msg;
                #else
                    tError() << msg;
                #endif
            #endif
            delete generator;

            return false;
        }

        QPainter painter(generator);
        painter.setRenderHint(QPainter::Antialiasing, true);
        foreach (TupScene *scene, scenes) {
            renderer.setScene(scene, size);
            int i = 0;
            while (renderer.nextPhotogram()) {
                #ifdef TUP_DEBUG
                    QString msg = "LibavPlugin::exportToFormat() - Rendering frame -> " + QString::number(i);
                    i++;
                    #ifdef Q_OS_WIN 
                        qDebug() << msg;
                    #else
                        tWarning() << msg;
                    #endif
                #endif
                renderer.render(&painter);
                generator->nextFrame();
                generator->reset();
            }
        }
    }

    generator->saveMovie(filePath);
    delete generator;

    return true;
}

bool LibavPlugin::exportFrame(int frameIndex, const QColor color, const QString &filePath, TupScene *scene, const QSize &size, TupLibrary *library)
{
    Q_UNUSED(frameIndex);
    Q_UNUSED(color);
    Q_UNUSED(filePath);
    Q_UNUSED(scene);
    Q_UNUSED(size);
    Q_UNUSED(library);

    return false;
}

QString LibavPlugin::getExceptionMsg() const {
    return errorMsg;
}
