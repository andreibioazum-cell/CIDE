#include "icons.h"
#include "theme.h"

#include <QHash>
#include <QPainter>
#include <QPainterPath>
#include <QPolygonF>
#include <QFileInfo>
#include <functional>
#include <QtGlobal>

namespace Icons {

namespace {

struct CacheKey {
    int kind;
    QRgb color;
    bool operator==(const CacheKey &o) const { return kind == o.kind && color == o.color; }
};
inline uint qHash(const CacheKey &k, uint seed) { return qHash(k.kind, seed) ^ qHash(k.color, seed); }

QPixmap render(const std::function<void(QPainter &)> &paint, const QColor &color) {
    const int px = 96; /* render large; QIcon scales down crisply */
    QPixmap pixmap(px, px);
    pixmap.setDevicePixelRatio(1);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);
    painter.scale(px / 24.0, px / 24.0);
    QPen pen(color);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    painter.setPen(pen);
    painter.setBrush(color);
    paint(painter);
    painter.end();
    return pixmap;
}

void bar(QPainter &p, qreal x, qreal y, qreal w, qreal h) {
    p.drawRoundedRect(QRectF(x, y, w, h), h / 2, h / 2);
}

void stroke(QPainter &p, const QPolygonF &points, qreal width) {
    QPen pen = p.pen();
    const QPen old = pen;
    pen.setWidthF(width);
    p.setPen(pen);
    p.drawPolyline(points);
    p.setPen(old);
}

void tri(QPainter &p, const QPolygonF &points) {
    p.drawPolygon(points);
}

void iconMenu(QPainter &p) {
    bar(p, 3.5, 6.5, 17, 2);
    bar(p, 3.5, 11, 17, 2);
    bar(p, 3.5, 15.5, 17, 2);
}

void iconMoreVert(QPainter &p) {
    p.drawEllipse(QPointF(12, 5.2), 1.9, 1.9);
    p.drawEllipse(QPointF(12, 12), 1.9, 1.9);
    p.drawEllipse(QPointF(12, 18.8), 1.9, 1.9);
}

void iconEdit(QPainter &p) {
    p.save();
    p.translate(12, 12);
    p.rotate(-45);
    QPainterPath path;
    path.addRoundedRect(QRectF(-1.8, -8.2, 3.6, 11.2), 0.8, 0.8);
    path.addRect(QRectF(-2.5, -8.6, 5, 1.8));
    path.moveTo(-1.8, 3);
    path.lineTo(1.8, 3);
    path.lineTo(0, 6.8);
    path.closeSubpath();
    p.drawPath(path);
    p.restore();
}

void iconPlayArrow(QPainter &p) {
    tri(p, QPolygonF() << QPointF(8, 5.2) << QPointF(19.4, 12) << QPointF(8, 18.8));
}

void iconSave(QPainter &p) {
    QPainterPath path;
    path.setFillRule(Qt::OddEvenFill);
    path.moveTo(4, 4);
    path.lineTo(15.5, 4);
    path.lineTo(20, 8.5);
    path.lineTo(20, 20);
    path.lineTo(4, 20);
    path.closeSubpath();
    path.addRect(QRectF(7, 5, 8, 3.2));   /* slot */
    path.addRect(QRectF(8, 13, 8, 5));    /* label */
    p.drawPath(path);
}

void iconFolder(QPainter &p) {
    QPainterPath path;
    path.moveTo(3, 5.6);
    path.lineTo(9.4, 5.6);
    path.lineTo(11.6, 8);
    path.lineTo(21, 8);
    path.lineTo(21, 18.6);
    path.lineTo(3, 18.6);
    path.closeSubpath();
    p.drawPath(path);
}

void iconFolderOpen(QPainter &p) {
    QPainterPath path;
    path.moveTo(3, 5.6);
    path.lineTo(9.4, 5.6);
    path.lineTo(11.6, 8);
    path.lineTo(19, 8);
    path.lineTo(19, 10);
    path.lineTo(5, 10);
    path.closeSubpath();
    p.drawPath(path);
    QPainterPath front;
    front.moveTo(3, 19);
    front.lineTo(5.6, 11);
    path.setFillRule(Qt::WindingFill);
    front.lineTo(21.4, 11);
    front.lineTo(18.8, 19);
    front.closeSubpath();
    p.drawPath(front);
}

void iconClose(QPainter &p) {
    stroke(p, QPolygonF() << QPointF(6.2, 6.2) << QPointF(17.8, 17.8), 2.4);
    stroke(p, QPolygonF() << QPointF(17.8, 6.2) << QPointF(6.2, 17.8), 2.4);
}

void iconHistory(QPainter &p) {
    QPen pen = p.pen();
    pen.setWidthF(2.2);
    p.setPen(pen);
    p.drawArc(QRectF(4.2, 4.2, 15.6, 15.6), 40 * 16, 290 * 16);
    tri(p, QPolygonF() << QPointF(3.6, 8.4) << QPointF(3.6, 3.4) << QPointF(8, 6.2));
    stroke(p, QPolygonF() << QPointF(12, 7.4) << QPointF(12, 12) << QPointF(15.4, 13.8), 2);
}

void iconSearch(QPainter &p) {
    QPen pen = p.pen();
    pen.setWidthF(2.4);
    p.setPen(pen);
    p.setBrush(Qt::NoBrush);
    p.drawEllipse(QRectF(4.2, 4.2, 11.2, 11.2));
    p.drawLine(QPointF(13.6, 13.6), QPointF(19.6, 19.6));
}

void iconCode(QPainter &p) {
    stroke(p, QPolygonF() << QPointF(9, 7) << QPointF(4.2, 12) << QPointF(9, 17), 2.2);
    stroke(p, QPolygonF() << QPointF(15, 7) << QPointF(19.8, 12) << QPointF(15, 17), 2.2);
}

void iconTerminal(QPainter &p) {
    QPen pen = p.pen();
    pen.setWidthF(2);
    p.setPen(pen);
    p.setBrush(Qt::NoBrush);
    p.drawRoundedRect(QRectF(3, 4.5, 18, 15), 2, 2);
    stroke(p, QPolygonF() << QPointF(7, 9.2) << QPointF(9.6, 11.8) << QPointF(7, 14.4), 1.8);
    p.drawLine(QPointF(11.6, 14.4), QPointF(15.2, 14.4));
}

void iconApps(QPainter &p) {
    for (int c = 0; c < 3; ++c)
        for (int r = 0; r < 3; ++r)
            p.drawRoundedRect(QRectF(4 + c * 6, 4 + r * 6, 4.4, 4.4), 1.2, 1.2);
}

void iconSettings(QPainter &p) {
    p.save();
    p.translate(12, 12);
    for (int k = 0; k < 8; ++k) {
        p.save();
        p.rotate(k * 45);
        p.drawRect(QRectF(-1.3, -9.6, 2.6, 3.6));
        p.restore();
    }
    p.restore();
    QPen pen = p.pen();
    pen.setWidthF(3);
    p.setPen(pen);
    p.setBrush(Qt::NoBrush);
    p.drawEllipse(QPointF(12, 12), 5.6, 5.6);
}

void iconHelp(QPainter &p) {
    QFont font = p.font();
    font.setBold(true);
    font.setPixelSize(15);
    p.setFont(font);
    p.drawText(QRectF(0, 0, 24, 24), Qt::AlignCenter, QStringLiteral("?"));
}

void iconExit(QPainter &p) {
    QPen pen = p.pen();
    pen.setWidthF(2.2);
    p.setPen(pen);
    p.setBrush(Qt::NoBrush);
    QPainterPath path;
    path.moveTo(13.5, 4.8);
    path.lineTo(19.6, 4.8);
    path.lineTo(19.6, 19.2);
    path.lineTo(13.5, 19.2);
    p.drawPath(path);
    p.drawLine(QPointF(3.5, 12), QPointF(14.5, 12));
    tri(p, QPolygonF() << QPointF(10.6, 7.8) << QPointF(16, 12) << QPointF(10.6, 16.2));
}

void iconDocuments(QPainter &p) {
    QPen pen = p.pen();
    pen.setWidthF(1.8);
    p.setPen(pen);
    p.setBrush(Qt::NoBrush);
    QPainterPath back;
    back.moveTo(10.6, 3.4);
    back.lineTo(18.4, 3.4);
    back.lineTo(18.4, 14.2);
    back.lineTo(8.4, 14.2);
    back.lineTo(8.4, 5.6);
    back.closeSubpath();
    p.drawPath(back);
    QPainterPath front;
    front.moveTo(5.6, 8.4);
    front.lineTo(12.6, 8.4);
    front.lineTo(15.4, 11);
    front.lineTo(15.4, 20.6);
    front.lineTo(5.6, 20.6);
    front.closeSubpath();
    p.drawPath(front);
}

void iconExtension(QPainter &p) {
    QPainterPath path;
    path.setFillRule(Qt::WindingFill);
    path.addRoundedRect(QRectF(5.5, 6.5, 13, 11), 1.6, 1.6);
    path.moveTo(14.2, 6.5);
    path.arcTo(QRectF(11.8, 3.2, 4.8, 4.8), 180, -180);
    path.closeSubpath();
    path.moveTo(10.2, 17.5);
    path.arcTo(QRectF(7.6, 14.6, 5.2, 5.2), 90, -180);
    path.closeSubpath();
    p.drawPath(path);
}

void iconNotifications(QPainter &p) {
    QPainterPath path;
    path.moveTo(6.2, 16.2);
    path.lineTo(6.2, 10.8);
    path.quadTo(6.2, 5, 12, 5);
    path.quadTo(17.8, 5, 17.8, 10.8);
    path.lineTo(17.8, 16.2);
    path.closeSubpath();
    p.drawPath(path);
    QPen pen = p.pen();
    pen.setWidthF(2.2);
    p.setPen(pen);
    p.drawLine(QPointF(4.6, 17.6), QPointF(19.4, 17.6));
    p.setBrush(p.pen().color());
    p.drawEllipse(QPointF(12, 19.6), 1.4, 1.4);
}

void iconUndo(QPainter &p) {
    QPen pen = p.pen();
    pen.setWidthF(2.2);
    p.setPen(pen);
    p.setBrush(Qt::NoBrush);
    p.drawArc(QRectF(5, 6, 14, 14), -30 * 16, -240 * 16);
    tri(p, QPolygonF() << QPointF(14.4, 8.9) << QPointF(19.4, 5.6) << QPointF(19.4, 11.6));
}

void iconRedo(QPainter &p) {
    p.save();
    p.translate(24, 0);
    p.scale(-1, 1);
    iconUndo(p);
    p.restore();
}

void iconKeyboardTab(QPainter &p) {
    QPen pen = p.pen();
    pen.setWidthF(2.2);
    p.setPen(pen);
    p.drawLine(QPointF(4, 12), QPointF(14.4, 12));
    tri(p, QPolygonF() << QPointF(12, 7.6) << QPointF(19.2, 12) << QPointF(12, 16.4));
    pen.setWidthF(2.4);
    p.setPen(pen);
    p.drawLine(QPointF(20, 6.4), QPointF(20, 17.6));
}

void chevron(QPainter &p, qreal x1, qreal y1, qreal x2, qreal y2, qreal x3, qreal y3) {
    stroke(p, QPolygonF() << QPointF(x1, y1) << QPointF(x2, y2) << QPointF(x3, y3), 2.6);
}

void iconArrowUp(QPainter &p) { chevron(p, 7.4, 14, 12, 8.6, 16.6, 14); }
void iconArrowDown(QPainter &p) { chevron(p, 7.4, 10, 12, 15.4, 16.6, 10); }
void iconArrowLeft(QPainter &p) { chevron(p, 14, 7.4, 8.6, 12, 14, 16.6); }
void iconArrowRight(QPainter &p) { chevron(p, 10, 7.4, 15.4, 12, 10, 16.6); }

void iconMoveLineUp(QPainter &p) {
    chevron(p, 7.4, 10.4, 12, 5.8, 16.6, 10.4);
    QPen pen = p.pen();
    pen.setWidthF(2.2);
    p.setPen(pen);
    p.drawLine(QPointF(6.8, 17.6), QPointF(17.2, 17.6));
}

void iconMoveLineDown(QPainter &p) {
    chevron(p, 7.4, 13.6, 12, 18.2, 16.6, 13.6);
    QPen pen = p.pen();
    pen.setWidthF(2.2);
    p.setPen(pen);
    p.drawLine(QPointF(6.8, 6.4), QPointF(17.2, 6.4));
}

void iconCopyLineUp(QPainter &p) {
    QPen pen = p.pen();
    pen.setWidthF(2.2);
    p.setPen(pen);
    p.drawLine(QPointF(6.8, 5.4), QPointF(17.2, 5.4));
    chevron(p, 7.4, 16, 12, 11.4, 16.6, 16);
}

void iconCopyLineDown(QPainter &p) {
    QPen pen = p.pen();
    pen.setWidthF(2.2);
    p.setPen(pen);
    p.drawLine(QPointF(6.8, 18.6), QPointF(17.2, 18.6));
    chevron(p, 7.4, 8, 12, 12.6, 16.6, 8);
}

void iconPaste(QPainter &p) {
    QPen pen = p.pen();
    pen.setWidthF(2);
    p.setPen(pen);
    p.setBrush(Qt::NoBrush);
    p.drawRoundedRect(QRectF(5.6, 4.4, 12.8, 16), 1.5, 1.5);
    p.setBrush(p.pen().color());
    p.drawRoundedRect(QRectF(9, 2.6, 6, 3.6), 1, 1);
}

void iconTextFormat(QPainter &p) {
    QFont font = p.font();
    font.setBold(true);
    font.setPixelSize(14);
    p.setFont(font);
    p.drawText(QRectF(0, 0, 24, 24), Qt::AlignCenter, QStringLiteral("A"));
}

void iconAdd(QPainter &p) {
    bar(p, 5, 11, 14, 2);
    bar(p, 11, 5, 2, 14);
}

void iconDocText(QPainter &p) {
    QPen pen = p.pen();
    pen.setWidthF(2);
    p.setPen(pen);
    p.setBrush(Qt::NoBrush);
    QPainterPath path;
    path.moveTo(6, 3.4);
    path.lineTo(13.8, 3.4);
    path.lineTo(18, 7.8);
    path.lineTo(18, 20.6);
    path.lineTo(6, 20.6);
    path.closeSubpath();
    p.drawPath(path);
    p.drawLine(QPointF(13.8, 3.4), QPointF(13.8, 7.8));
    p.drawLine(QPointF(13.8, 7.8), QPointF(18, 7.8));
    pen.setWidthF(1.6);
    p.setPen(pen);
    p.drawLine(QPointF(8.6, 11.4), QPointF(15.4, 11.4));
    p.drawLine(QPointF(8.6, 14.4), QPointF(15.4, 14.4));
    p.drawLine(QPointF(8.6, 17.4), QPointF(13, 17.4));
}

void iconTune(QPainter &p) {
    QPen pen = p.pen();
    pen.setWidthF(2);
    p.setPen(pen);
    p.drawLine(QPointF(4, 7), QPointF(20, 7));
    p.drawLine(QPointF(4, 12), QPointF(20, 12));
    p.drawLine(QPointF(4, 17), QPointF(20, 17));
    p.drawEllipse(QPointF(15, 7), 2.4, 2.4);
    p.drawEllipse(QPointF(8, 12), 2.4, 2.4);
    p.drawEllipse(QPointF(17, 17), 2.4, 2.4);
}

void iconInfo(QPainter &p) {
    QPen pen = p.pen();
    pen.setWidthF(2);
    p.setPen(pen);
    p.setBrush(Qt::NoBrush);
    p.drawEllipse(QRectF(4.4, 4.4, 15.2, 15.2));
    p.setBrush(p.pen().color());
    p.drawRoundedRect(QRectF(11, 8, 2, 6), 1, 1);
    p.drawEllipse(QPointF(12, 16.8), 1.1, 1.1);
}

void iconPin(QPainter &p) {
    p.save();
    p.translate(12, 12);
    p.rotate(-35);
    QPainterPath path;
    path.addRoundedRect(QRectF(-4, -7.6, 8, 7), 3.2, 3.2);
    p.drawPath(path);
    QPen pen = p.pen();
    pen.setWidthF(2);
    p.setPen(pen);
    p.drawLine(QPointF(0, -0.4), QPointF(0, 9));
    p.restore();
}

void iconLastPage(QPainter &p) {
    chevron(p, 7, 6.4, 12.6, 12, 7, 17.6);
    QPen pen = p.pen();
    pen.setWidthF(2.4);
    p.setPen(pen);
    p.drawLine(QPointF(17.4, 6.4), QPointF(17.4, 17.6));
}

void iconFirstPage(QPainter &p) {
    p.save();
    p.translate(24, 0);
    p.scale(-1, 1);
    iconLastPage(p);
    p.restore();
}

void iconCompareArrows(QPainter &p) {
    QPen pen = p.pen();
    pen.setWidthF(2);
    p.setPen(pen);
    p.drawLine(QPointF(4, 8.6), QPointF(17, 8.6));
    tri(p, QPolygonF() << QPointF(14.2, 5.2) << QPointF(19.8, 8.6) << QPointF(14.2, 12));
    p.drawLine(QPointF(20, 15.4), QPointF(7, 15.4));
    tri(p, QPolygonF() << QPointF(9.8, 12) << QPointF(4.2, 15.4) << QPointF(9.8, 18.8));
}

void iconSubdirArrowLeft(QPainter &p) {
    QPen pen = p.pen();
    pen.setWidthF(2.2);
    p.setPen(pen);
    p.setBrush(Qt::NoBrush);
    QPainterPath path;
    path.moveTo(16.4, 4.2);
    path.lineTo(16.4, 12.6);
    path.lineTo(9, 12.6);
    p.drawPath(path);
    tri(p, QPolygonF() << QPointF(12, 9.2) << QPointF(6.2, 12.6) << QPointF(12, 16));
}

void iconPalette(QPainter &p) {
    QPainterPath path;
    path.setFillRule(Qt::OddEvenFill);
    path.addEllipse(QPointF(12, 12), 8.4, 8.4);
    path.addEllipse(QPointF(15.2, 15.4), 2.4, 2.4);
    path.addEllipse(QPointF(8.8, 8.4), 1.5, 1.5);
    path.addEllipse(QPointF(13.2, 7), 1.5, 1.5);
    path.addEllipse(QPointF(16.6, 10.4), 1.5, 1.5);
    p.drawPath(path);
}

void iconShare(QPainter &p) {
    p.drawEllipse(QPointF(6.8, 12.4), 2.3, 2.3);
    p.drawEllipse(QPointF(17.2, 5.6), 2.3, 2.3);
    p.drawEllipse(QPointF(17.2, 19.2), 2.3, 2.3);
    QPen pen = p.pen();
    pen.setWidthF(1.8);
    p.setPen(pen);
    p.drawLine(QPointF(8.8, 11.3), QPointF(15.2, 6.7));
    p.drawLine(QPointF(8.8, 13.5), QPointF(15.2, 18.1));
}

void iconOpenInBrowser(QPainter &p) {
    QPen pen = p.pen();
    pen.setWidthF(2);
    p.setPen(pen);
    p.setBrush(Qt::NoBrush);
    p.drawRoundedRect(QRectF(3, 4.6, 18, 14.8), 2, 2);
    p.drawLine(QPointF(12, 16), QPointF(12, 9));
    tri(p, QPolygonF() << QPointF(8.8, 11.8) << QPointF(12, 7.8) << QPointF(15.2, 11.8));
}

void iconHome(QPainter &p) {
    QPainterPath path;
    path.setFillRule(Qt::OddEvenFill);
    path.moveTo(2.8, 11.2);
    path.lineTo(12, 3.6);
    path.lineTo(21.2, 11.2);
    path.lineTo(18.6, 11.2);
    path.lineTo(18.6, 20.4);
    path.lineTo(5.4, 20.4);
    path.lineTo(5.4, 11.2);
    path.closeSubpath();
    path.addRect(QRectF(10.2, 14.6, 3.6, 5.8));
    p.drawPath(path);
}

void iconChevronUp(QPainter &p) { chevron(p, 7.4, 14.6, 12, 9.4, 16.6, 14.6); }
void iconChevronDown(QPainter &p) { chevron(p, 7.4, 9.4, 12, 14.6, 16.6, 9.4); }

void iconGlobe(QPainter &p) {
    QPen pen = p.pen();
    pen.setWidthF(1.8);
    p.setPen(pen);
    p.setBrush(Qt::NoBrush);
    p.drawEllipse(QRectF(4, 4, 16, 16));
    p.drawEllipse(QRectF(8.4, 4, 7.2, 16));
    p.drawLine(QPointF(4.4, 12), QPointF(19.6, 12));
}

void iconCommandPalette(QPainter &p) {
    QPen pen = p.pen();
    pen.setWidthF(2.2);
    p.setPen(pen);
    p.drawLine(QPointF(5, 18.4), QPointF(19, 18.4));
    p.drawLine(QPointF(5, 12), QPointF(15.6, 12));
    tri(p, QPolygonF() << QPointF(13, 8) << QPointF(18.6, 12) << QPointF(13, 16));
}

void iconControlKey(QPainter &p) { chevron(p, 7.6, 8.6, 12, 4.4, 16.4, 8.6); }

void iconFileGlyph(QPainter &p) {
    QPainterPath path;
    path.moveTo(6, 3.2);
    path.lineTo(14.2, 3.2);
    path.lineTo(18.4, 7.6);
    path.lineTo(18.4, 20.8);
    path.lineTo(6, 20.8);
    path.closeSubpath();
    p.drawPath(path);
    p.setPen(Qt::NoPen);
    p.drawRect(QRectF(13.6, 3.2, 4.8, 4.4));
}

void paint(Icon kind, QPainter &p) {
    switch (kind) {
    case Menu: iconMenu(p); break;
    case MoreVert: iconMoreVert(p); break;
    case Edit: iconEdit(p); break;
    case PlayArrow: iconPlayArrow(p); break;
    case Save: iconSave(p); break;
    case Folder: iconFolder(p); break;
    case FolderOpen: iconFolderOpen(p); break;
    case Close: iconClose(p); break;
    case History: iconHistory(p); break;
    case Search: iconSearch(p); break;
    case Code: iconCode(p); break;
    case Terminal: iconTerminal(p); break;
    case Apps: iconApps(p); break;
    case Settings: iconSettings(p); break;
    case Help: iconHelp(p); break;
    case Exit: iconExit(p); break;
    case Documents: iconDocuments(p); break;
    case Extension: iconExtension(p); break;
    case Notifications: iconNotifications(p); break;
    case Undo: iconUndo(p); break;
    case Redo: iconRedo(p); break;
    case KeyboardTab: iconKeyboardTab(p); break;
    case ArrowUp: iconArrowUp(p); break;
    case ArrowDown: iconArrowDown(p); break;
    case ArrowLeft: iconArrowLeft(p); break;
    case ArrowRight: iconArrowRight(p); break;
    case MoveLineUp: iconMoveLineUp(p); break;
    case MoveLineDown: iconMoveLineDown(p); break;
    case CopyLineUp: iconCopyLineUp(p); break;
    case CopyLineDown: iconCopyLineDown(p); break;
    case Paste: iconPaste(p); break;
    case TextFormat: iconTextFormat(p); break;
    case Add: iconAdd(p); break;
    case DocText: iconDocText(p); break;
    case Tune: iconTune(p); break;
    case Info: iconInfo(p); break;
    case Pin: iconPin(p); break;
    case LastPage: iconLastPage(p); break;
    case FirstPage: iconFirstPage(p); break;
    case CompareArrows: iconCompareArrows(p); break;
    case SubdirArrowLeft: iconSubdirArrowLeft(p); break;
    case Palette: iconPalette(p); break;
    case Share: iconShare(p); break;
    case OpenInBrowser: iconOpenInBrowser(p); break;
    case Home: iconHome(p); break;
    case ChevronUp: iconChevronUp(p); break;
    case ChevronDown: iconChevronDown(p); break;
    case Globe: iconGlobe(p); break;
    case CommandPalette: iconCommandPalette(p); break;
    case ControlKey: iconControlKey(p); break;
    case FileGlyph: iconFileGlyph(p); break;
    }
}

} // namespace

QIcon icon(Icon kind, const QColor &color) {
    static QHash<CacheKey, QIcon> cache;
    const CacheKey key{int(kind), color.rgba()};
    auto it = cache.find(key);
    if (it != cache.end()) return *it;
    const QPixmap pixmap = render([kind](QPainter &p) { paint(kind, p); }, color);
    const QIcon result(pixmap);
    cache.insert(key, result);
    return result;
}

QColor fileIconColor(const QString &fileName) {
    const QString suffix = QFileInfo(fileName).suffix().toLower();
    if (suffix.isEmpty()) return QColor(0x90, 0xa4, 0xae);
    if (suffix == QLatin1String("html") || suffix == QLatin1String("htm")) return QColor(0xe3, 0x4f, 0x26);
    if (suffix == QLatin1String("js") || suffix == QLatin1String("mjs")) return QColor(0xff, 0xca, 0x28);
    if (suffix == QLatin1String("ts")) return QColor(0x42, 0xa5, 0xf5);
    if (suffix == QLatin1String("jsx")) return QColor(0x26, 0xc6, 0xda);
    if (suffix == QLatin1String("tsx")) return QColor(0x4d, 0xd0, 0xe1);
    if (suffix == QLatin1String("css")) return QColor(0x29, 0xb6, 0xf6);
    if (suffix == QLatin1String("scss") || suffix == QLatin1String("sass")) return QColor(0xec, 0x40, 0x7a);
    if (suffix == QLatin1String("json")) return QColor(0xfb, 0xc0, 0x2d);
    if (suffix == QLatin1String("md")) return QColor(0x78, 0x90, 0x9c);
    if (suffix == QLatin1String("py")) return QColor(0x66, 0xbb, 0x6a);
    if (suffix == QLatin1String("c") || suffix == QLatin1String("h")) return QColor(0x5c, 0x6b, 0xc0);
    if (suffix == QLatin1String("cpp") || suffix == QLatin1String("cc") || suffix == QLatin1String("hpp")) return QColor(0xef, 0x53, 0x50);
    if (suffix == QLatin1String("java")) return QColor(0xff, 0xa7, 0x26);
    if (suffix == QLatin1String("php")) return QColor(0x7e, 0x57, 0xc2);
    if (suffix == QLatin1String("xml")) return QColor(0x8d, 0x6e, 0x63);
    if (suffix == QLatin1String("yml") || suffix == QLatin1String("yaml")) return QColor(0x26, 0xa6, 0x9a);
    if (suffix == QLatin1String("sh") || suffix == QLatin1String("bash")) return QColor(0xa1, 0x88, 0x7f);
    if (suffix == QLatin1String("txt")) return QColor(0x90, 0xa4, 0xae);
    return QColor(0x90, 0xa4, 0xae);
}

QIcon fileIcon(const QString &fileName, bool isDir) {
    if (isDir) return icon(Folder, QColor(0x9a, 0xa5, 0xb1));
    return icon(FileGlyph, fileIconColor(fileName));
}

} // namespace Icons
