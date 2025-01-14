#pragma once

#include <QProxyStyle>

class BloomProxyStyle: public QProxyStyle
{
    Q_OBJECT

public:
    int styleHint(
        StyleHint hint,
        const QStyleOption* option = nullptr,
        const QWidget* widget = nullptr,
        QStyleHintReturn* returnData = nullptr
    ) const override;
};
