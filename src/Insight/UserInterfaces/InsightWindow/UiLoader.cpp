#include "UiLoader.hpp"

#include <QtUiTools>

#include "Widgets/RotatableLabel.hpp"
#include "Widgets/SlidingHandleWidget.hpp"
#include "Widgets/SvgWidget.hpp"
#include "Widgets/SvgToolButton.hpp"
#include "Widgets/ExpandingWidget.hpp"
#include "Widgets/ExpandingHeightScrollAreaWidget.hpp"
#include "Widgets/TargetRegistersPane/TargetRegistersPaneWidget.hpp"

#include "src/Logger/Logger.hpp"

using namespace Bloom;
using namespace Bloom::Widgets;

UiLoader::UiLoader(QObject* parent): QUiLoader(parent) {
    this->customWidgetConstructorsByWidgetName = decltype(this->customWidgetConstructorsByWidgetName) {
        {
            "RotatableLabel",
            [this](QWidget* parent, const QString& name) {
                auto widget = new RotatableLabel("", parent);
                widget->setObjectName(name);
                widget->setStyleSheet(parent->styleSheet());
                return widget;
            }
        },
        {
            "SlidingHandleWidget",
            [this](QWidget* parent, const QString& name) {
                auto widget = new SlidingHandleWidget(parent);
                widget->setObjectName(name);
                widget->setStyleSheet(parent->styleSheet());
                return widget;
            }
        },
        {
            "ExpandingWidget",
            [this](QWidget* parent, const QString& name) {
                auto widget = new ExpandingWidget(parent);
                widget->setObjectName(name);
                widget->setStyleSheet(parent->styleSheet());
                return widget;
            }
        },
        {
            "ExpandingHeightScrollAreaWidget",
            [this](QWidget* parent, const QString& name) {
                auto widget = new ExpandingHeightScrollAreaWidget(parent);
                widget->setObjectName(name);
                widget->setStyleSheet(parent->styleSheet());
                return widget;
            }
        },
        {
            "SvgWidget",
            [this](QWidget* parent, const QString& name) {
                auto widget = new SvgWidget(parent);
                widget->setObjectName(name);
                widget->setStyleSheet(parent->styleSheet());
                return widget;
            }
        },
        {
            "SvgToolButton",
            [this](QWidget* parent, const QString& name) {
                auto widget = new SvgToolButton(parent);
                widget->setObjectName(name);
                widget->setStyleSheet(parent->styleSheet());
                return widget;
            }
        },
    };
}

QWidget* UiLoader::createWidget(const QString& className, QWidget* parent, const QString& name) {
    if (this->customWidgetConstructorsByWidgetName.contains(className)) {
        // This is a custom widget - call the mapped constructor
        return this->customWidgetConstructorsByWidgetName.at(className)(parent, name);
    }

    return QUiLoader::createWidget(className, parent, name);
}