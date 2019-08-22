#include "mainwidget.h"
#include "ui_mainwidget.h"

MainWidget::MainWidget(QWidget* parent) : QWidget(parent), ui(new Ui::MainWidget) {
    ui->setupUi(this);
    //定时器
    sendTimer = new QTimer(this);

    //点击
    QObject::connect(ui->comCombo, SIGNAL(clicked()), this, SLOT(scanSerialPort()));
    QObject::connect(ui->comSwitchButton, SIGNAL(clicked()), this, SLOT(switchSerialPort()));
    QObject::connect(ui->sendDataButton, SIGNAL(clicked()), this, SLOT(sendData()));
    QObject::connect(ui->sendFileButton, SIGNAL(clicked()), this, SLOT(sendFile()));
    QObject::connect(ui->clearReceiverButton, SIGNAL(clicked()), this, SLOT(clearReceiver()));
    QObject::connect(ui->saveReceiverButton, SIGNAL(clicked()), this, SLOT(saveReceiver()));
    //选择
    QObject::connect(ui->comCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(setComComboToolTip(int)));
    QObject::connect(ui->encodingCombo, SIGNAL(currentTextChanged(QString)), this, SLOT(setEncoding(QString)));
    //状态改变
    QObject::connect(ui->sendingTimerCheck, SIGNAL(stateChanged(int)), this, SLOT(setTimer(int)));
    QObject::connect(ui->showHexCheck, SIGNAL(stateChanged(int)), this, SLOT(setReceiverShowHex(int)));
    //串口
    QObject::connect(&serial, SIGNAL(readyRead()), this, SLOT(readSerial()));
    //定时器
    QObject::connect(sendTimer, SIGNAL(timeout()), this, SLOT(sendData()));

    scanSerialPort();                               //首次检测
    setEncoding(ui->encodingCombo->currentText());  //设置默认编码
}

MainWidget::~MainWidget() {
    if (serial.isOpen()) {
        serial.close();  //关闭程序时关闭串口
    }
    delete sendTimer;
    delete ui;
}

void MainWidget::setSerialPort(QString portName, int baudRate, int dataBit, int parityIndex, int stopBitIndex,
                               int flowControlIndex) {
    serial.setPortName(portName);
    serial.setBaudRate(baudRate);
    serial.setDataBits(QSerialPort::DataBits(dataBit));  // combo索引与enum对应, 不switch
    switch (parityIndex) {
        case 0:
            serial.setParity(QSerialPort::NoParity);
            break;
        case 1:
            serial.setParity(QSerialPort::EvenParity);
            break;
        case 2:
            serial.setParity(QSerialPort::OddParity);
            break;
        case 3:
            serial.setParity(QSerialPort::MarkParity);
            break;
        case 4:
            serial.setParity(QSerialPort::SpaceParity);
            break;
    }
    switch (stopBitIndex) {
        case 0:
            serial.setStopBits(QSerialPort::OneStop);
            break;
        case 1:
            serial.setStopBits(QSerialPort::OneAndHalfStop);
            break;
        case 2:
            serial.setStopBits(QSerialPort::TwoStop);
            break;
    }
    serial.setFlowControl(QSerialPort::FlowControl(flowControlIndex));  // combo索引与enum对应, 不switch
}

void MainWidget::lineBreakReplace(QString* text, int index) {
    switch (index) {
        case 0:
            text = &text->replace("\n", "\r\n");
            break;
        case 1:  //默认LF, 不替换
            break;
        case 2:
            text = &text->replace("\n", "\r");
            break;
    }
}

QByteArray MainWidget::getNewLineData() {
    QString newLine = "\n";
    lineBreakReplace(&newLine, ui->lineBreakCombo->currentIndex());
    return newLine.toUtf8();
}

QString MainWidget::getCurrentTime() {
    QDate date = QDate::currentDate();
    QTime time = QTime::currentTime();
    return "[" + date.toString(Qt::ISODate) + " " + time.toString(Qt::ISODate) + "]";
}

void MainWidget::scanSerialPort() {
    serialPortList = QSerialPortInfo::availablePorts();
    QString text = ui->comCombo->currentText();  //记录输入的内容以便清除后重写入
    ui->comCombo->clear();
    foreach (const QSerialPortInfo& info, MainWidget::serialPortList) { ui->comCombo->addItem(info.portName()); }
    if (!text.isEmpty()) {  //防止首次检测时置空
        ui->comCombo->setCurrentText(text);
    }
}

void MainWidget::switchSerialPort() {
    if (serial.isOpen()) {
        serial.close();
        ui->comSwitchButton->setText("打开串口");
        ui->comCombo->setEnabled(true);
        ui->baudRateCombo->setEnabled(true);
        ui->dataBitCombo->setEnabled(true);
        ui->parityCombo->setEnabled(true);
        ui->stopBitCombo->setEnabled(true);
        ui->flowControlCombo->setEnabled(true);
        setTimer(Qt::Unchecked);  //关闭定时发送
    } else {
        QString port = ui->comCombo->currentText();
        foreach (const QSerialPortInfo& info, MainWidget::serialPortList) {  //判断输入的端口号是否存在
            if (info.portName() == port) {
                setSerialPort(port, ui->baudRateCombo->currentText().toInt(), ui->dataBitCombo->currentText().toInt(),
                              ui->parityCombo->currentIndex(), ui->stopBitCombo->currentIndex(),
                              ui->flowControlCombo->currentIndex());
                if (!serial.open(QIODevice::ReadWrite)) {
                    QMessageBox::critical(this, "错误", "端口打开失败");  //打开失败, 提示错误
                    return;
                }
                ui->comSwitchButton->setText("关闭串口");
                ui->comCombo->setEnabled(false);
                ui->baudRateCombo->setEnabled(false);
                ui->dataBitCombo->setEnabled(false);
                ui->parityCombo->setEnabled(false);
                ui->stopBitCombo->setEnabled(false);
                ui->flowControlCombo->setEnabled(false);
                return;  //打开完成, 退出函数
            }
        }
        QMessageBox::critical(this, "错误", "端口不存在");  //未找到端口号, 提示错误
    }
}

void MainWidget::sendData() {
    if (serial.isOpen()) {
        QString text = ui->sendingEdit->toPlainText();
        //空行处理
        if (text.isEmpty()) {
            return;
        }
        QByteArray data;
        //发送HEX
        if (ui->sendingHexCheck->isChecked()) {
            data = QByteArray::fromHex(text.toUtf8());
        } else {
            lineBreakReplace(&text, ui->lineBreakCombo->currentIndex());  //换行符转换
            data = textCodec->fromUnicode(text);                          //编码转换
        }
        //发送新行
        if (ui->sendingNewLineCheck->isChecked()) {
            data.append(getNewLineData());
        }
        //显示发送
        if (ui->showSendingCheck->isChecked()) {
            //显示时间
            if (ui->showTimeCheck->isChecked()) {
                receiverData.append(getNewLineData());
                receiverData.append(getCurrentTime());
            }
            receiverData.append("<-");
            receiverData.append(data);
            //自动换行
            if (ui->autoLineBreakCheck->isChecked()) {
                receiverData.append(getNewLineData());
            }
            //显示HEX
            if (ui->showHexCheck->isChecked()) {
                setReceiverShowHex(Qt::Checked);
            } else {
                setReceiverShowHex(Qt::Unchecked);
            }
        }
        serial.write(data);  //发送
        //自动清空
        if (ui->autoClearCheck->isChecked()) {
            ui->sendingEdit->clear();
        }
        sendingNumber += data.size();  //计数
        ui->sendingNumberEdit->setText(QString::number(sendingNumber));
    } else {
        QMessageBox::critical(this, "错误", "端口未打开");  //未打开串口, 提示错误
    }
}

void MainWidget::sendFile() {
    if (serial.isOpen()) {
        QString filePath = QFileDialog::getOpenFileName(this, "请选择文件", "/");
        if (!filePath.isEmpty()) {
            QFile file(filePath);
            if (!file.open(QIODevice::ReadOnly)) {
                QMessageBox::critical(this, "错误", "文件打开失败");  //打开失败, 提示错误
                return;
            }
            while (!file.atEnd()) {
                serial.write(file.read(512));
            }
            QMessageBox::information(this, "完成", "文件发送成功");
            file.close();
        }
    } else {
        QMessageBox::critical(this, "错误", "端口未打开");  //未打开串口, 提示错误
    }
}

void MainWidget::setTimer(int state) {
    if (state == Qt::Checked) {
        if (serial.isOpen()) {
            Qt::WindowFlags flags = Qt::WindowFlags();
            flags.setFlag(Qt::WindowCloseButtonHint);
            QInputDialog* timerDialog = new QInputDialog(this, flags);
            timerDialog->setInputMode(QInputDialog::IntInput);
            timerDialog->setWindowIcon(this->windowIcon());
            timerDialog->setIntMaximum(2147483647);
            timerDialog->setIntMinimum(1);  //间隔时间必须大于等于1
            timerDialog->setIntValue(1000);
            timerDialog->setIntStep(100);
            timerDialog->setWindowTitle("开启定时发送");
            timerDialog->setLabelText("间隔时间(ms)");
            timerDialog->resize(250, 89);
            timerDialog->setOkButtonText("确认");
            timerDialog->setCancelButtonText("取消");
            if (timerDialog->exec() == QInputDialog::Accepted) {
                sendTimer->start(timerDialog->intValue());
            } else {
                ui->sendingTimerCheck->setCheckState(Qt::Unchecked);
            }
        } else {
            ui->sendingTimerCheck->setCheckState(Qt::Unchecked);
            QMessageBox::critical(this, "错误", "端口未打开");  //未打开串口, 提示错误
        }
    } else {
        if (sendTimer->isActive()) {
            sendTimer->stop();
            ui->sendingTimerCheck->setCheckState(Qt::Unchecked);
        }
    }
}

void MainWidget::setComComboToolTip(int index) {
    if (index >= 0) {                                                   //清空后会出现-1
        ui->comCombo->setToolTip(serialPortList[index].description());  //提示当前选择端口号的信息
    }
}

void MainWidget::setEncoding(QString encoding) {
    MainWidget::textCodec = QTextCodec::codecForName(encoding.toUtf8());
    //编码转换
    setReceiverShowHex(Qt::Unchecked);
}

void MainWidget::setReceiverShowHex(int state) {
    if (state == Qt::Checked) {
        ui->receiverEdit->setText(receiverData.toHex(' ').toUpper());
    } else {
        //编码转换
        QString text = textCodec->toUnicode(receiverData);
        ui->receiverEdit->setText(text);
    }
}

void MainWidget::readSerial() {
    //显示时间
    if (ui->showTimeCheck->isChecked()) {
        receiverData.append(getNewLineData());
        receiverData.append(getCurrentTime());
    }
    //显示发送
    if (ui->showSendingCheck->isChecked()) {
        receiverData.append("->");
    }
    //得到数据
    QByteArray data = serial.readAll();
    receiverData.append(data);
    //计数
    receiverNumber += data.length();
    ui->receiverNumberEdit->setText(QString::number(receiverNumber));
    //自动换行
    if (ui->autoLineBreakCheck->isChecked()) {
        receiverData.append(getNewLineData());
    }
    //显示HEX
    if (ui->showHexCheck->isChecked()) {
        setReceiverShowHex(Qt::Checked);
    } else {
        setReceiverShowHex(Qt::Unchecked);
    }
}

void MainWidget::clearReceiver() {
    receiverData.clear();
    ui->receiverEdit->clear();
    receiverNumber = 0;
    ui->receiverNumberEdit->setText("0");
}

void MainWidget::saveReceiver() {
    QString filePath = QFileDialog::getSaveFileName(this, "保存文件", "/", "Text Files (*.txt);;Binary Files (*.bin)");
    if (!filePath.isEmpty()) {
        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly)) {
            QMessageBox::critical(this, "错误", "文件打开失败");  //打开失败, 提示错误
            return;
        }
        file.write(receiverData);
        file.close();
    }
}
