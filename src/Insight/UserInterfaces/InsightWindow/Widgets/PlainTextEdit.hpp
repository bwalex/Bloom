#pragma once

#include <QPlainTextEdit>
#include <QContextMenuEvent>

namespace Bloom::Widgets
{
    class PlainTextEdit: public QPlainTextEdit
    {
        Q_OBJECT

    public:
        explicit PlainTextEdit(QWidget* parent = nullptr);

    protected:
        void contextMenuEvent(QContextMenuEvent* event) override;
    };
}