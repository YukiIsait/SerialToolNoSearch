#include "mainwidget.h"
#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setStyle("Fusion");
    MainWidget widget;
    widget.show();

    return app.exec();
}
