#pragma once

#include <QDateTime>
#include <QHBoxLayout>
#include <QLabel>

#include "Item.hpp"

namespace Bloom::Widgets
{
    class RegisterHistoryItem: public Item
    {
    Q_OBJECT
        QHBoxLayout* layout = new QHBoxLayout(this);
        QLabel* dateLabel = new QLabel(this);
        QLabel* valueLabel = new QLabel(this);

    public:
        RegisterHistoryItem(
            const Targets::TargetMemoryBuffer& registerValue,
            const QDateTime& changeDate,
            QWidget *parent
        );
    };
}
