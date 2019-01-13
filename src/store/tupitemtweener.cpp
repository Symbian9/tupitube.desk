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

#include "tupitemtweener.h"
#include "tupsvg2qt.h"

struct TupItemTweener::Private
{
    /* Private() : frames(0) {} */
    
    QString name;
    TupItemTweener::Type type;
    int initFrame;
    int initLayer;
    int initScene;

    int frames;
    QPointF originPoint;

    // Position Tween
    QString path;
    QString intervals;

    // Rotation Tween
    TupItemTweener::RotationType rotationType;
    TupItemTweener::RotateDirection rotateDirection;
    double rotateSpeed;
    int rotateLoop;
    int rotateReverseLoop;
    int rotateStartDegree;
    int rotateEndDegree;

    // Scale Tween
    TupItemTweener::TransformAxes scaleAxes;
    double initialXScaleFactor;
    double initialYScaleFactor;
    double scaleFactor;
    int scaleIterations;
    int scaleLoop;
    int scaleReverseLoop;

    // Shear Tween
    TupItemTweener::TransformAxes shearAxes;
    double shearFactor;
    int shearIterations;
    int shearLoop;
    int shearReverseLoop;

    // Opacity Tween
    double initOpacityFactor;
    double endOpacityFactor;
    int opacityIterations;
    int opacityLoop;
    int opacityReverseLoop;

    // Color Tween
    FillType colorFillType;
    QColor initialColor;
    QColor endingColor;
    int colorIterations;
    int colorLoop;
    int colorReverseLoop;

    QList<TupItemTweener::Type> tweenList;

    QHash<int, TupTweenerStep *> steps; // TODO: remove when Qt 4.3

    inline TupTweenerStep *step(int step)
    {
        TupTweenerStep *currentStep = steps[step];
        if (!currentStep) {
            currentStep = new TupTweenerStep(step);
            steps.insert(step, currentStep);
        }
        
        return currentStep;
    }
};

TupItemTweener::TupItemTweener() : k(new Private)
{
    k->frames = 0;
    k->initialXScaleFactor = 1;
    k->initialYScaleFactor = 1;
}

TupItemTweener::~TupItemTweener()
{
    qDeleteAll(k->steps);
    delete k;
}

QString TupItemTweener::name()
{
    return k->name;
}

TupItemTweener::Type TupItemTweener::type()
{
    return k->type;
}

void TupItemTweener::addStep(const TupTweenerStep &step)
{
    int counter = step.index();

    if (step.has(TupTweenerStep::Position))
        setPosAt(counter, step.position());

    if (step.has(TupTweenerStep::Rotation)) 
        setRotationAt(counter, step.rotation());
    
    if (step.has(TupTweenerStep::Scale))
        setScaleAt(counter, step.horizontalScale(), step.verticalScale());
    
    if (step.has(TupTweenerStep::Shear))
        setShearAt(counter, step.horizontalShear(), step.verticalShear());
    
    if (step.has(TupTweenerStep::Opacity))
        setOpacityAt(counter, step.opacity());

    if (step.has(TupTweenerStep::Coloring))
        setColorAt(counter, step.color());
}

TupTweenerStep * TupItemTweener::stepAt(int index)
{
    return k->step(index);
}

TupTweenerStep * TupItemTweener::lastStep()
{
    int index = k->steps.size() - 1;
    return k->step(index);
}

void TupItemTweener::setPosAt(int index, const QPointF &pos)
{
    k->step(index)->setPosition(pos);
}

void TupItemTweener::setRotationAt(int index, double angle)
{
    k->step(index)->setRotation(angle);
}

void TupItemTweener::setScaleAt(int index, double sx, double sy)
{
    k->step(index)->setScale(sx, sy);
}

void TupItemTweener::setShearAt(int index, double sx, double sy)
{
    k->step(index)->setShear(sx, sy);
}

void TupItemTweener::setOpacityAt(int index, double opacity)
{
    k->step(index)->setOpacity(opacity);
}

void TupItemTweener::setColorAt(int index, const QColor &color)
{
    k->step(index)->setColor(color);
}

void TupItemTweener::setFrames(int frames)
{
    k->frames = frames;
}

int TupItemTweener::frames() const
{
    return k->frames;
}

int TupItemTweener::initFrame()
{
    return k->initFrame;
}

int TupItemTweener::initLayer()
{
    return k->initLayer;
}

int TupItemTweener::initScene()
{
    return k->initScene;
}

QPointF TupItemTweener::transformOriginPoint()
{
    return k->originPoint;
}

double TupItemTweener::initXScaleValue()
{
    return k->initialXScaleFactor;
}

double TupItemTweener::initYScaleValue()
{
    return k->initialYScaleFactor;
}

void TupItemTweener::fromXml(const QString &xml)
{
    #ifdef TUP_DEBUG
        QString msg = "TupItemTweener::fromXml() - Tween content: ";
        #ifdef Q_OS_WIN
           qWarning() << msg;
           qWarning() << xml;
        #else
           tWarning() << msg;
           tWarning() << xml;
        #endif
    #endif
    
    QDomDocument doc;

    if (doc.setContent(xml)) {
        QDomElement root = doc.documentElement();

        k->name = root.attribute("name");
        k->type = TupItemTweener::Type(root.attribute("type").toInt());

        k->initFrame = root.attribute("initFrame").toInt();
        k->initLayer = root.attribute("initLayer").toInt();
        k->initScene = root.attribute("initScene").toInt();

        k->frames = root.attribute("frames").toInt();

        QString origin = root.attribute("origin"); // [x,y]
        QStringList list = origin.split(",");
        double x = list.first().toDouble();
        double y = list.last().toDouble();

        k->originPoint = QPointF(x, y); 

        if (k->type == TupItemTweener::Position) {
            k->path = root.attribute("coords");
            k->intervals = root.attribute("intervals");
        } else if (k->type == TupItemTweener::Rotation) {
            k->rotationType = TupItemTweener::RotationType(root.attribute("rotationType").toInt());
            k->rotateSpeed = root.attribute("rotateSpeed").toDouble();
            k->rotateDirection = TupItemTweener::RotateDirection(root.attribute("rotateDirection").toInt());

            if (k->rotationType == TupItemTweener::Partial) {
                k->rotateLoop = root.attribute("rotateLoop").toInt();
                k->rotateStartDegree = root.attribute("rotateStartDegree").toInt();
                k->rotateEndDegree = root.attribute("rotateEndDegree").toInt();
                k->rotateReverseLoop = root.attribute("rotateReverseLoop").toInt();
            }
        } else if (k->type == TupItemTweener::Scale) {
            k->scaleAxes = TupItemTweener::TransformAxes(root.attribute("scaleAxes").toInt());
            k->initialXScaleFactor = root.attribute("initXScaleFactor").toDouble();
            k->initialYScaleFactor = root.attribute("initYScaleFactor").toDouble();
            k->scaleFactor = root.attribute("scaleFactor").toDouble();
            k->scaleIterations = root.attribute("scaleIterations").toInt();
            k->scaleLoop = root.attribute("scaleLoop").toInt();
            k->scaleReverseLoop = root.attribute("scaleReverseLoop").toInt();
        } else if (k->type == TupItemTweener::Shear) {
            k->shearAxes = TupItemTweener::TransformAxes(root.attribute("shearAxes").toInt());
            k->shearFactor = root.attribute("shearFactor").toDouble();
            k->shearIterations = root.attribute("shearIterations").toInt();
            k->shearLoop = root.attribute("shearLoop").toInt();
            k->shearReverseLoop = root.attribute("shearReverseLoop").toInt();
        } else if (k->type == TupItemTweener::Opacity) {
            k->initOpacityFactor = root.attribute("initOpacityFactor").toDouble();
            k->endOpacityFactor = root.attribute("endOpacityFactor").toDouble();
            k->opacityIterations = root.attribute("opacityIterations").toInt();
            k->opacityLoop = root.attribute("opacityLoop").toInt();
            k->opacityReverseLoop = root.attribute("opacityReverseLoop").toInt();
        } else if (k->type == TupItemTweener::Coloring) {
            k->colorFillType = FillType(root.attribute("fillType").toInt());
            QString colorText = root.attribute("initialColor");
            QStringList list = colorText.split(",");
            int red = list.at(0).toInt();
            int green = list.at(1).toInt();
            int blue = list.at(2).toInt();
            k->initialColor = QColor(red, green, blue);

            colorText = root.attribute("endingColor");
            list = colorText.split(",");
            red = list.at(0).toInt();
            green = list.at(1).toInt();
            blue = list.at(2).toInt();
            k->endingColor = QColor(red, green, blue);

            k->colorIterations = root.attribute("colorIterations").toInt();
            k->colorLoop = root.attribute("colorLoop").toInt();
            k->colorReverseLoop = root.attribute("colorReverseLoop").toInt();
        }

        QDomNode node = root.firstChildElement("step");
        while (!node.isNull()) {
               QDomElement e = node.toElement();

               if (!e.isNull()) {
                   if (e.tagName() == "step") {
                       QString stepDoc;
                       {
                           QTextStream ts(&stepDoc);
                           ts << node;
                       }

                       TupTweenerStep *step = new TupTweenerStep(0);
                       step->fromXml(stepDoc);
                       addStep(*step);

                       delete step;
                    }
                }

                node = node.nextSibling();
        }
    }
}

QDomElement TupItemTweener::toXml(QDomDocument &doc) const
{
    #ifdef TUP_DEBUG
        QString msg1 = "TupItemTweener::toXml() - Saving tween: " + k->name;
        QString msg2 = "TupItemTweener::toXml() - Type: " + QString::number(k->type);
        #ifdef Q_OS_WIN
           qWarning() << msg1;
           qWarning() << msg2;
        #else
           tWarning() << msg1;
           tWarning() << msg2;
        #endif
    #endif

    QDomElement root = doc.createElement("tweening");
    root.setAttribute("name", k->name);
    root.setAttribute("type", k->type);

    root.setAttribute("initFrame", QString::number(k->initFrame));
    root.setAttribute("initLayer", QString::number(k->initLayer));
    root.setAttribute("initScene", QString::number(k->initScene));
    root.setAttribute("frames", QString::number(k->frames));

    root.setAttribute("origin", QString::number(k->originPoint.x()) + "," + QString::number(k->originPoint.y()));

    if (k->type == TupItemTweener::Position) {
        root.setAttribute("coords", k->path);
        root.setAttribute("intervals", k->intervals);
    } else if (k->type == TupItemTweener::Rotation) {
        root.setAttribute("rotationType", k->rotationType);
        root.setAttribute("rotateSpeed", QString::number(k->rotateSpeed));
        root.setAttribute("rotateDirection", k->rotateDirection);

        if (k->rotationType == TupItemTweener::Partial) {
            root.setAttribute("rotateLoop", QString::number(k->rotateLoop));
            root.setAttribute("rotateStartDegree", QString::number(k->rotateStartDegree));
            root.setAttribute("rotateEndDegree", QString::number(k->rotateEndDegree));
            root.setAttribute("rotateReverseLoop", QString::number(k->rotateReverseLoop));
        }
    } else if (k->type == TupItemTweener::Scale) {
        root.setAttribute("scaleAxes", QString::number(k->scaleAxes));
        root.setAttribute("initXScaleFactor", QString::number(k->initialXScaleFactor));
        root.setAttribute("initYScaleFactor", QString::number(k->initialYScaleFactor));
        root.setAttribute("scaleFactor", QString::number(k->scaleFactor));
        root.setAttribute("scaleIterations", QString::number(k->scaleIterations));
        root.setAttribute("scaleLoop", QString::number(k->scaleLoop));
        root.setAttribute("scaleReverseLoop", QString::number(k->scaleReverseLoop));
    } else if (k->type == TupItemTweener::Shear) {
        root.setAttribute("shearAxes", QString::number(k->shearAxes));
        root.setAttribute("shearFactor", QString::number(k->shearFactor));
        root.setAttribute("shearIterations", QString::number(k->shearIterations));
        root.setAttribute("shearLoop", QString::number(k->shearLoop));
        root.setAttribute("shearReverseLoop", QString::number(k->shearReverseLoop));
    } else if (k->type == TupItemTweener::Opacity) {
        root.setAttribute("initOpacityFactor", QString::number(k->initOpacityFactor));
        root.setAttribute("endOpacityFactor", QString::number(k->endOpacityFactor));
        root.setAttribute("opacityIterations", QString::number(k->opacityIterations));
        root.setAttribute("opacityLoop", QString::number(k->opacityLoop));
        root.setAttribute("opacityReverseLoop", QString::number(k->opacityReverseLoop));
    } else if (k->type == TupItemTweener::Coloring) {
        root.setAttribute("fillType", k->colorFillType);
        QString colorText = QString::number(k->initialColor.red()) + "," + QString::number(k->initialColor.green())
                            + "," + QString::number(k->initialColor.blue());
        root.setAttribute("initialColor", colorText);
        colorText = QString::number(k->endingColor.red()) + "," + QString::number(k->endingColor.green())
                    + "," + QString::number(k->endingColor.blue());
        root.setAttribute("endingColor", colorText);
        root.setAttribute("colorIterations", QString::number(k->colorIterations));
        root.setAttribute("colorLoop", QString::number(k->colorLoop));
        root.setAttribute("colorReverseLoop", QString::number(k->colorReverseLoop));
    }
 
    foreach (TupTweenerStep *step, k->steps.values())
        root.appendChild(step->toXml(doc));
    
    return root;
}

QGraphicsPathItem *TupItemTweener::graphicsPath() const
{
    QGraphicsPathItem *item = new QGraphicsPathItem();
    QPainterPath path;
    TupSvg2Qt::svgpath2qtpath(k->path, path);

    item->setPath(path);

    return item;
}

void TupItemTweener::setGraphicsPath(const QString &path)
{
    k->path = path;
}

QList<int> TupItemTweener::intervals()
{
    QList<int> sections;
    QStringList list = k->intervals.split(",");
    foreach (QString section, list)
             sections << section.toInt();

    return sections;
}

QString TupItemTweener::tweenType()
{
    QString type = ""; 
    switch (k->type) {
        case TupItemTweener::Position :
            type = QString(tr("Position"));
            break;
        case TupItemTweener::Rotation :
            type = QString(tr("Rotation"));
            break;
        case TupItemTweener::Scale :
            type = QString(tr("Scale"));
            break;
        case TupItemTweener::Shear :
            type = QString(tr("Shear"));
            break;
        case TupItemTweener::Opacity :
            type = QString(tr("Opacity"));
            break;
        case TupItemTweener::Coloring :
            type = QString(tr("Coloring"));
            break;
        case TupItemTweener::Papagayo :
            type = QString(tr("Papagayo Lip-sync"));
            break;
    }

    return type;
}

TupItemTweener::RotationType TupItemTweener::tweenRotationType()
{
    return k->rotationType;
}

double TupItemTweener::tweenRotateSpeed()
{
    return k->rotateSpeed;
}

bool TupItemTweener::tweenRotateLoop()
{
    return k->rotateLoop;
}

TupItemTweener::RotateDirection TupItemTweener::tweenRotateDirection()
{
    return k->rotateDirection;
}

int TupItemTweener::tweenRotateStartDegree()
{
    return k->rotateStartDegree;
}

int TupItemTweener::tweenRotateEndDegree()
{
    return k->rotateEndDegree;
}

bool TupItemTweener::tweenRotateReverseLoop()
{
    return k->rotateReverseLoop;
}

TupItemTweener::TransformAxes TupItemTweener::tweenScaleAxes()
{
    return k->scaleAxes;
}

double TupItemTweener::tweenScaleFactor()
{
    return k->scaleFactor;
}

int TupItemTweener::tweenScaleIterations()
{
    return k->scaleIterations;
}

int TupItemTweener::tweenScaleLoop()
{
    return k->scaleLoop;
}

int TupItemTweener::tweenScaleReverseLoop()
{
    return k->scaleReverseLoop;
}

TupItemTweener::TransformAxes TupItemTweener::tweenShearAxes()
{
    return k->shearAxes;
}

double TupItemTweener::tweenShearFactor()
{
    return k->shearFactor;
}

int TupItemTweener::tweenShearIterations()
{
    return k->shearIterations;
}

int TupItemTweener::tweenShearLoop()
{
    return k->shearLoop;
}

int TupItemTweener::tweenShearReverseLoop()
{
    return k->shearReverseLoop;
}

double TupItemTweener::tweenOpacityInitialFactor()
{
    return k->initOpacityFactor;
}

double TupItemTweener::tweenOpacityEndingFactor()
{
    return k->endOpacityFactor;
}

int TupItemTweener::tweenOpacityIterations()
{
    return k->opacityIterations;
}

int TupItemTweener::tweenOpacityLoop() 
{
    return k->opacityLoop;
}

int TupItemTweener::tweenOpacityReverseLoop()
{
    return k->opacityReverseLoop;
}

TupItemTweener::FillType TupItemTweener::tweenColorFillType()
{
    return k->colorFillType;
}

QColor TupItemTweener::tweenInitialColor()
{
    return k->initialColor;
}

QColor TupItemTweener::tweenEndingColor()
{
    return k->endingColor;
}

int TupItemTweener::tweenColorIterations()
{
    return k->colorIterations; 
}

int TupItemTweener::tweenColorLoop()
{
    return k->colorLoop;
}

int TupItemTweener::tweenColorReverseLoop()
{
    return k->colorReverseLoop;
}

bool TupItemTweener::contains(TupItemTweener::Type type)
{
    for (int i=0; i < k->tweenList.size(); i++) {
         if (k->tweenList.at(i) == type)
             return true;
    }

    return false; 
}
