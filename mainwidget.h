#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#pragma execution_character_set("utf-8")  //让该死的VC支持UTF-8

#include <QDate>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QTextCodec>
#include <QTime>
#include <QTimer>
#include <QWidget>

#include <QSerialPort>
#include <QSerialPortInfo>

namespace Ui {
class MainWidget;
}

class MainWidget : public QWidget {
    Q_OBJECT

public:
    explicit MainWidget(QWidget* parent = nullptr);
    ~MainWidget();

private:
    Ui::MainWidget* ui;

    QList<QSerialPortInfo> serialPortList;
    QSerialPort serial;
    QTextCodec* textCodec;
    QTimer* sendTimer;
    QByteArray receiverData;
    int sendingNumber = 0;
    int receiverNumber = 0;

    void setSerialPort(QString portName, int baudRate, int dataBit, int parityIndex, int stopBitIndex,
                       int flowControlIndex);
    void lineBreakReplace(QString* text, int index);
    QByteArray getNewLineData();
    QString getCurrentTime();

private slots:
    //点击
    void scanSerialPort();
    void switchSerialPort();
    void sendData();
    void sendFile();
    void clearReceiver();
    void saveReceiver();
    //选择
    void setComComboToolTip(int index);
    void setEncoding(QString encoding);
    //状态改变
    void setTimer(int state);
    void setReceiverShowHex(int state);
    //串口
    void readSerial();
};

#endif
