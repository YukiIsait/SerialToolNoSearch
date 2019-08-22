#include "clickablecombobox.h"

ClickableComboBox::ClickableComboBox(QWidget* parent) : QComboBox(parent) {}

ClickableComboBox::~ClickableComboBox(){};

void ClickableComboBox::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        emit clicked();
    }
    QComboBox::mousePressEvent(event);
}
