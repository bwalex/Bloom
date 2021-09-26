#include "TargetPackageWidgetContainer.hpp"

using namespace Bloom;
using namespace Bloom::Widgets::InsightTargetWidgets;

TargetPackageWidgetContainer::TargetPackageWidgetContainer(QWidget* parent): QWidget(parent) {
    this->packageWidget = this->findChild<TargetPackageWidget*>();
}

void TargetPackageWidgetContainer::resizeEvent(QResizeEvent* event) {
    if (this->packageWidget == nullptr) {
        return;
    }

    const auto packageSize = this->packageWidget->size();
    this->packageWidget->setGeometry(
        (this->width() / 2) - (packageSize.width() / 2),
        (this->height() / 2) - (packageSize.height() / 2),
        packageSize.width(),
        packageSize.height()
    );
}

void TargetPackageWidgetContainer::setPackageWidget(TargetPackageWidget* packageWidget) {
    this->packageWidget = packageWidget;
}
