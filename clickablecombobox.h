#pragma once
#include <QComboBox>
#include <QMouseEvent>

class ClickableComboBox : public QComboBox {
    Q_OBJECT

public:
    explicit ClickableComboBox(QWidget* parent = nullptr);
    ~ClickableComboBox();

protected:
    virtual void mousePressEvent(QMouseEvent* event);

signals:
    void clicked();
};
