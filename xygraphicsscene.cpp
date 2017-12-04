﻿#include "xygraphicsscene.h"
#include "xyrectgraphicsitem.h"
#include "xypathgraphicsitem.h"
#include "xyellipsegraphicsitem.h"
#include "xylinegraphicsitem.h"
#include "xyarrowsgraphicsitem.h"
#include "xytextgraphicsitem.h"
#include <QGraphicsView>
#include <QApplication>

XYGraphicsScene::XYGraphicsScene(qreal x, qreal y, qreal width, qreal height, QObject *parent)
    : QGraphicsScene(x, y, width, height, parent)
{
    meShape = ELLIPSE;
    haveKeyboardItem = NULL;
    textEdit = NULL;
}

XYGraphicsScene::~XYGraphicsScene()
{

}

XYGraphicsScene::SHAPE XYGraphicsScene::getShape()
{
    return meShape;
}

void XYGraphicsScene::setShape(XYGraphicsScene::SHAPE shape)
{
    meShape = shape;
}

void XYGraphicsScene::setItemMovable(bool movable)
{
    XYMovableGraphicsItem::acceptMouse = movable;
}

void XYGraphicsScene::setTextEdit(QTextEdit *textEdit)
{
    this->textEdit = textEdit;
    if (this->textEdit != NULL)
    {
        this->textEdit->setVisible(false);
        connect(this->textEdit, SIGNAL(textChanged()), this, SLOT(setItemText()));
    }
}

void XYGraphicsScene::savePixmap(const QString &path)
{
    QPixmap pixmap;
    QPainter painter(&pixmap);
    this->render(&painter);
    pixmap.save(path);
}

void XYGraphicsScene::showTextEdit(XYTextGraphicsItem *item)
{
    if (textEdit != NULL)
    {
        haveKeyboardItem = item;
        qreal x_offset = 0;
        qreal y_offset = 0;
        if (!views().isEmpty())
        {
            x_offset = views().at(0)->width();
            y_offset = views().at(0)->height();
        }
        if (x_offset > width())
        {
            x_offset = (x_offset - width()) / 2;
        }
        if (y_offset > height())
        {
            y_offset = (y_offset - height()) / 2;
        }
        textEdit->resize(item->sceneBoundingRect().width(),
                         item->sceneBoundingRect().height());
        textEdit->move(item->sceneBoundingRect().x() + x_offset,
                       item->sceneBoundingRect().y() + y_offset);
        textEdit->setText(((XYTextGraphicsItem *)item)->msText);
        textEdit->setFocus();
        textEdit->setVisible(true);
    }
}

void XYGraphicsScene::setItemText()
{
    if (textEdit != NULL && haveKeyboardItem != NULL)
    {
        ((XYTextGraphicsItem *)haveKeyboardItem)->msText = textEdit->toPlainText();
    }
}

bool XYGraphicsScene::event(QEvent *event)
{
    static XYMovableGraphicsItem *item = NULL;
    QGraphicsScene::event(event);
    if (event->isAccepted())
    {
        return true;
    }

    QGraphicsSceneMouseEvent *mouse_event = (QGraphicsSceneMouseEvent *)event;
    if (event->type() == QEvent::GraphicsSceneMousePress)
    {
        if (mouse_event->button() == Qt::LeftButton)
        {
            item = getCurDrawshapeItem();
            setGraphicsItemStartPos(item, mouse_event->scenePos());
            if (item)
            {
                this->addItem(item);
            }
        }
        else if (mouse_event->button() == Qt::RightButton)
        {
            if (item)
            {
                this->removeItem(item);
                delete item;
                item = NULL;
            }
        }
    }
    else if (event->type() == QEvent::GraphicsSceneMouseRelease)
    {
        if (mouse_event->button() == Qt::LeftButton)
        {
            if (item)
            {
                setGraphicsItemMovePos(item, mouse_event->scenePos());
                this->removeItem(item);
                if (item->isValid())
                {
                    this->addItem(item);
                    setGraphicsItemEndPos(item, mouse_event->scenePos());
                }
                else
                {
                    delete item;
                }
                item = NULL;
            }
        }
    }
    else if (event->type() == QEvent::GraphicsSceneMouseMove)
    {
        if (item)
        {
            setGraphicsItemMovePos(item, mouse_event->scenePos());
            update(this->sceneRect());
        }
    }
    event->accept();
    return true;
}

void XYGraphicsScene::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if (mouseEvent->button() == Qt::LeftButton)
    {
        QList<QGraphicsItem *> items = this->items();
        for (int i = 0; i < items.size(); ++i)
        {
            if (((XYMovableGraphicsItem *)items.at(i))->selected)
            {
                ((XYMovableGraphicsItem *)items.at(i))->mouseMoveEvent(mouseEvent);
            }
        }
    }
    QGraphicsScene::mouseMoveEvent(mouseEvent);
}

void XYGraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    QGraphicsItem *item = itemAt(mouseEvent->scenePos(), QTransform());

    switch (meShape)
    {
    case DELETE:
        if (item != NULL)
        {
            removeItem(item);
            delete item;
        }
        return;
    default:
        break;
    }
    if (item &&
        item->type() == XYTextGraphicsItem::XYTEXT)
    {
        haveKeyboardItem = (XYTextGraphicsItem *)item;
        if (meShape != CURSOR)
        {
            showTextEdit(haveKeyboardItem);
        }
    }
    else if (textEdit != NULL && !textEdit->isHidden())
    {
        textEdit->setVisible(false);
    }

    if (!(qApp->keyboardModifiers() & Qt::ControlModifier))
    {
        QList<QGraphicsItem *> items = this->items();
        for (int i = 0; i < items.size(); ++i)
        {
            if (items.at(i) != item)
            {
                ((XYMovableGraphicsItem *)items.at(i))->selected = false;
            }
        }
    }
    update(sceneRect());
    QGraphicsScene::mousePressEvent(mouseEvent);
}

QPen XYGraphicsScene::getCurPen()
{
    QPen pen;
    pen.setColor(QColor("blue"));
    return pen;
}

QBrush XYGraphicsScene::getCurBrush()
{
    QBrush brush;
    brush.setStyle(Qt::SolidPattern);
    brush.setColor(QColor("green"));
    return brush;
}

XYMovableGraphicsItem *XYGraphicsScene::getCurDrawshapeItem()
{
    XYMovableGraphicsItem *item = NULL;
    switch (meShape)
    {
    case RECT:
        item = new XYRectGraphicsItem;
        break;
    case PATH:
        item = new XYPathGraphicsItem;
        break;
    case ELLIPSE:
        item = new XYEllipseGraphicsItem;
        break;
    case LINE:
        item = new XYLineGraphicsItem;
        break;
    case ARROWS:
        item = new XYArrowsGraphicsItem;
        break;
    case TEXT:
        item = new XYTextGraphicsItem;
        break;
    default:
        break;
    }

    if (item)
    {
        item->setPen(getCurPen());
        item->setBrush(getCurBrush());
    }
    return item;
}

void XYGraphicsScene::setGraphicsItemStartPos(XYMovableGraphicsItem *item, const QPointF &pos)
{
    switch (meShape)
    {
    case PATH:
    {
        XYPathGraphicsItem *pathItem = static_cast<XYPathGraphicsItem *>(item);
        if (pathItem)
        {
            pathItem->moPath.moveTo(pos);
        }
        break;
    }
    case RECT:
    case ELLIPSE:
    case LINE:
    case TEXT:
        break;
    case ARROWS:
    {
        XYArrowsGraphicsItem *arrowsItem = static_cast<XYArrowsGraphicsItem *>(item);
        arrowsItem->endPos = pos;
        break;
    }
    default:
        break;
    }
    XYMovableGraphicsItem *moveItem = static_cast<XYMovableGraphicsItem *>(item);
    if (moveItem)
    {
        moveItem->startCreateItem(pos);
    }
}

void XYGraphicsScene::setGraphicsItemEndPos(XYMovableGraphicsItem *item, const QPointF &pos)
{
    XYMovableGraphicsItem *moveItem = static_cast<XYMovableGraphicsItem *>(item);
    if (moveItem)
    {
        moveItem->endCreateItem(pos);
    }
}

void XYGraphicsScene::setGraphicsItemMovePos(XYMovableGraphicsItem *item, const QPointF &pos)
{
    item->endPos = pos;
    switch (meShape)
    {
    case RECT:
    {
        XYRectGraphicsItem *rectItem = static_cast<XYRectGraphicsItem *>(item);
        if (rectItem)
        {
            rectItem->moRect = QRectF(qMin(rectItem->startPos.x(), pos.x()),
                                      qMin(rectItem->startPos.y(), pos.y()),
                                      qAbs(rectItem->startPos.x() - pos.x()),
                                      qAbs(rectItem->startPos.y() - pos.y()));
        }
        break;
    }
    case PATH:
    {
        XYPathGraphicsItem *pathItem = static_cast<XYPathGraphicsItem *>(item);
        if (pathItem)
        {
            pathItem->moPath.lineTo(pos);
        }
        break;
    }
    case ELLIPSE:
    {
        XYEllipseGraphicsItem *ellipseItem = static_cast<XYEllipseGraphicsItem *>(item);
        if (ellipseItem)
        {
            ellipseItem->moRect = QRectF(qMin(ellipseItem->startPos.x(), pos.x()),
                                      qMin(ellipseItem->startPos.y(), pos.y()),
                                      qAbs(ellipseItem->startPos.x() - pos.x()),
                                      qAbs(ellipseItem->startPos.y() - pos.y()));
        }
        break;
    }
    case LINE:
    {
        XYLineGraphicsItem *lineItem = static_cast<XYLineGraphicsItem *>(item);
        if (lineItem)
        {
            lineItem->moLine = QLineF(lineItem->startPos, pos);
        }
        break;
    }
    case ARROWS:
    case TEXT:
    {
        XYMovableGraphicsItem *generalItem = static_cast<XYMovableGraphicsItem *>(item);
        if (generalItem)
        {
            generalItem->duringCreateItem(pos);
        }
        break;
    }
    default:
        break;
    }
}
