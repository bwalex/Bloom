#include <QPainter>

#include "BodyWidget.hpp"
#include "src/Logger/Logger.hpp"

using namespace Bloom::Widgets::InsightTargetWidgets::Qfp;

void BodyWidget::paintEvent(QPaintEvent* event) {
    auto painter = QPainter(this);
    this->drawWidget(painter);
}

void BodyWidget::drawWidget(QPainter& painter) {
    painter.setRenderHints(QPainter::RenderHint::Antialiasing | QPainter::RenderHint::SmoothPixmapTransform, true);

    auto targetBodyColor = this->getBodyColor();
    auto backgroundColor = QColor("#373835");

    if (!this->isEnabled()) {
        targetBodyColor.setAlpha(this->getDisableAlphaLevel());
    }

    painter.setPen(Qt::PenStyle::NoPen);
    painter.setBrush(targetBodyColor);

    auto containerGeometry = this->geometry();
    auto targetBodyWidth = containerGeometry.width() - 16;
    auto targetBodyHeight = containerGeometry.height() - 16;

    auto targetBodyPoint = QPoint(
        (containerGeometry.width() / 2) - (targetBodyWidth / 2),
        (containerGeometry.height() / 2) - (targetBodyHeight / 2)
    );

    painter.drawRect(
        targetBodyPoint.x(),
        targetBodyPoint.y(),
        targetBodyWidth,
        targetBodyHeight
    );

    painter.setBrush(backgroundColor);
    painter.drawEllipse(QRectF(
        targetBodyPoint.x() + 13,
        targetBodyPoint.y() + 13,
        20,
        20
    ));
}
