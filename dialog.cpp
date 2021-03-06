/****************************************************************************
**
** Copyright (C) 2012 Denis Shienkov <denis.shienkov@gmail.com>
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtSerialPort module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "dialog.h"

#include <QComboBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSerialPortInfo>
#include <QSpinBox>
#include <QDebug>

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    m_serialPortLabel(new QLabel(tr("Serial port:"))),
    m_serialPortComboBox(new QComboBox),
    m_waitRequestLabel(new QLabel(tr("Wait request, msec:"))),
    m_waitRequestSpinBox(new QSpinBox),
    m_responseLabel(new QLabel(tr("Response:"))),
    m_responseLineEdit(new QLineEdit(tr("Hello, I'm the receiver."))),
    m_trafficLabel(new QLabel(tr("No traffic."))),
    m_statusLabel(new QLabel(tr("Status: Not running."))),
    m_runButton(new QPushButton(tr("Start")))
{
    m_waitRequestSpinBox->setRange(0, 10000);
    m_waitRequestSpinBox->setValue(20);

    const auto infos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : infos)
        m_serialPortComboBox->addItem(info.portName());

    auto mainLayout = new QGridLayout;
    mainLayout->addWidget(m_serialPortLabel, 0, 0);
    mainLayout->addWidget(m_serialPortComboBox, 0, 1);
    mainLayout->addWidget(m_waitRequestLabel, 1, 0);
    mainLayout->addWidget(m_waitRequestSpinBox, 1, 1);
    mainLayout->addWidget(m_runButton, 0, 2, 2, 1);
    mainLayout->addWidget(m_responseLabel, 2, 0);
    mainLayout->addWidget(m_responseLineEdit, 2, 1, 1, 3);
    mainLayout->addWidget(m_trafficLabel, 3, 0, 1, 4);
    mainLayout->addWidget(m_statusLabel, 4, 0, 1, 5);
    setLayout(mainLayout);

    setWindowTitle(tr("Receiver"));
    m_serialPortComboBox->setFocus();

    m_timer.setSingleShot(true);

    connect(m_runButton, &QPushButton::clicked, this, &Dialog::startReceiver);
    connect(&m_serial, &QSerialPort::readyRead, this, &Dialog::readRequest);
    connect(&m_timer, &QTimer::timeout, this, &Dialog::processTimeout);

    connect(m_responseLineEdit, &QLineEdit::textChanged, this, &Dialog::activateRunButton);
}

void Dialog::startReceiver()
{
    if (m_serial.portName() != m_serialPortComboBox->currentText()) {
        m_serial.close();
        m_serial.setPortName(m_serialPortComboBox->currentText());

        m_serial.setBaudRate(m_serial.Baud9600);
        m_serial.setDataBits(m_serial.Data8);
        m_serial.setStopBits(m_serial.OneStop);
        if (!m_serial.open(QIODevice::ReadOnly)) {
            processError(tr("Can't open %1, error code %2")
                         .arg(m_serial.portName()).arg(m_serial.error()));
            return;
        }
    }

    m_runButton->setEnabled(false);
    m_statusLabel->setText(tr("Status: Running, connected to port %1.")
                           .arg(m_serialPortComboBox->currentText()));
}

void Dialog::readRequest()
{
    if (!m_timer.isActive())
        m_timer.start(m_waitRequestSpinBox->value());
    auto data = m_serial.readAll();
    this->decoder.appendBuffer(data);
    m_request.append(data);
}

void Dialog::processTimeout()
{
    m_trafficLabel->setText(tr("Traffic, transaction #%1:"
                               "\n\r-request: %2"
                               "\n\r-response: %3")
                            .arg(++m_transactionCount).arg(QString::fromLocal8Bit(m_request.toHex()))
                            .arg(m_responseLineEdit->text()));
    qDebug() << QString::fromLocal8Bit(m_request.toHex());
    m_request.clear();
}

void Dialog::activateRunButton()
{
    m_runButton->setEnabled(true);
}

void Dialog::processError(const QString &s)
{
    activateRunButton();
    m_statusLabel->setText(tr("Status: Not running, %1.").arg(s));
    m_trafficLabel->setText(tr("No traffic."));
}
