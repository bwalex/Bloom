#include "ByteItem.hpp"

#include <QPainter>
#include <QStyle>

#include "src/Logger/Logger.hpp"

using namespace Bloom::Widgets;

ByteItem::ByteItem(
    std::size_t byteIndex,
    std::uint32_t address,
    std::optional<ByteItem*>& hoveredByteItem
): QGraphicsItem(nullptr), byteIndex(byteIndex), address(address), hoveredByteItem(hoveredByteItem) {
    this->setAcceptHoverEvents(true);

    this->addressHex = "0x" + QString::number(this->address, 16).rightJustified(8, '0').toUpper();
    this->relativeAddressHex = "0x" + QString::number(this->byteIndex, 16).rightJustified(8, '0').toUpper();

    this->setSelected(false);
}

void ByteItem::setValue(unsigned char value) {
    this->valueChanged = this->valueInitialised && this->value != value;

    this->value = value;
    this->hexValue = QString::number(this->value, 16).rightJustified(2, '0').toUpper();
    this->asciiValue = (this->value >= 32 && this->value <= 126)
        ? std::optional(QString(QChar(this->value))) : std::nullopt;

    this->valueInitialised = true;
}

void ByteItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    painter->setRenderHints(QPainter::RenderHint::Antialiasing | QPainter::RenderHint::SmoothPixmapTransform, true);
    painter->setPen(Qt::PenStyle::NoPen);

    static const auto widgetRect = this->boundingRect();

    if (this->hoveredByteItem.has_value()) {
        const auto hoveredWidget = this->hoveredByteItem.value();

        if (hoveredWidget->currentColumnIndex == this->currentColumnIndex
            || hoveredWidget->currentRowIndex == this->currentRowIndex
        ){
            painter->setBrush(QColor(0x8E, 0x8B, 0x83, hoveredWidget == this ? 70 : 30));
            painter->drawRect(widgetRect);
        }
    }

    auto textColor = QColor(this->valueChanged ? "#547fba" : "#afb1b3");

    if (this->valueInitialised) {
        if (!this->isEnabled()) {
            textColor.setAlpha(100);
        }

        painter->setPen(textColor);
        painter->drawText(widgetRect, Qt::AlignCenter, this->hexValue);

    } else {
        textColor.setAlpha(100);
        painter->setPen(textColor);

        static const auto placeholderString = QString("??");
        painter->drawText(widgetRect, Qt::AlignCenter, placeholderString);
    }
}