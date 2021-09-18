#include "Item.hpp"

#include <QStyle>

using namespace Bloom::Widgets;

Item::Item(const Targets::TargetMemoryBuffer& registerValue, QWidget* parent):
ClickableWidget(parent), registerValue(registerValue) {
    auto onClick = [this] {
        this->setSelected(true);
    };

    this->connect(this, &ClickableWidget::clicked, this, onClick);
    this->connect(this, &ClickableWidget::rightClicked, this, onClick);

    this->setSelected(false);
}

void Item::setSelected(bool selected) {
    this->setProperty("selected", selected);
    this->style()->unpolish(this);
    this->style()->polish(this);

    if (selected) {
        emit this->selected(this);
    }
}
