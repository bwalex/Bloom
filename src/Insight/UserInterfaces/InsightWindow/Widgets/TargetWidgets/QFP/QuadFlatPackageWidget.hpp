#pragma once

#include <QWidget>
#include <vector>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "../TargetPackageWidget.hpp"
#include "PinWidget.hpp"
#include "BodyWidget.hpp"
#include "src/Targets/TargetVariant.hpp"

namespace Bloom::Widgets::InsightTargetWidgets::Qfp
{
    /**
     * QuadFlatPackageWidget implements a custom Qt widget for Quad-flat package variants.
     */
    class QuadFlatPackageWidget: public TargetPackageWidget
    {
    Q_OBJECT
    private:
        QVBoxLayout* layout = nullptr;
        QHBoxLayout* horizontalLayout = nullptr;
        QHBoxLayout* topPinLayout = nullptr;
        QVBoxLayout* rightPinLayout = nullptr;
        QHBoxLayout* bottomPinLayout = nullptr;
        QVBoxLayout* leftPinLayout = nullptr;
        BodyWidget* bodyWidget = nullptr;

    protected:
        void paintEvent(QPaintEvent* event) override;
        void drawWidget(QPainter& painter);

    public:
        QuadFlatPackageWidget(const Targets::TargetVariant& targetVariant, QObject* insightWindowObj, QWidget* parent);
    };
}
