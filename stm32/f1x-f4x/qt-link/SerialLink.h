#pragma once

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QVariantList>
#include <QVariantMap>

// 串口链路：接收 F407 上送的数据包 → CRC-16/MODBUS 校验 → 校验通过回 "OK"
//
// 自动识别两种上送格式（互不干扰，可同时存在）：
//   1) 二进制帧（推荐）：FF D0 D1 D2 D3 CRC_L CRC_H FE
//   2) 文本行（兼容现有 F407 固件的 USART1_Printf 输出）：RX: 05 06 07 08  CRC=1A2B
//
// 其余文本（F407 Ready / F407 alive / PC cmd: n）只进原始日志，不计入统计。
class SerialLink : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool connected READ isConnected NOTIFY connectedChanged)
    Q_PROPERTY(QString portName READ portName WRITE setPortName NOTIFY portNameChanged)
    Q_PROPERTY(int portIndex READ portIndex NOTIFY portsChanged)
    Q_PROPERTY(QVariantList ports READ ports NOTIFY portsChanged)
    Q_PROPERTY(int baudRate READ baudRate WRITE setBaudRate NOTIFY baudRateChanged)
    Q_PROPERTY(bool autoReply READ autoReply WRITE setAutoReply NOTIFY autoReplyChanged)
    Q_PROPERTY(int totalCount READ totalCount NOTIFY statsChanged)
    Q_PROPERTY(int okCount READ okCount NOTIFY statsChanged)
    Q_PROPERTY(int failCount READ failCount NOTIFY statsChanged)
    Q_PROPERTY(int replyCount READ replyCount NOTIFY statsChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)

public:
    explicit SerialLink(QObject *parent = nullptr);

    bool isConnected() const { return m_connected; }

    QString portName() const { return m_portName; }
    void setPortName(const QString &name);

    int portIndex() const;
    QVariantList ports() const { return m_ports; }

    int baudRate() const { return m_baudRate; }
    void setBaudRate(int baud) { if (m_baudRate != baud) { m_baudRate = baud; emit baudRateChanged(); } }

    bool autoReply() const { return m_autoReply; }
    void setAutoReply(bool on);

    int totalCount() const { return m_total; }
    int okCount() const { return m_ok; }
    int failCount() const { return m_fail; }
    int replyCount() const { return m_reply; }
    QString lastError() const { return m_lastError; }

    Q_INVOKABLE bool open();
    Q_INVOKABLE void close();
    Q_INVOKABLE void refreshPorts();
    Q_INVOKABLE void clearStats();
    Q_INVOKABLE void sendText(const QString &text);
    Q_INVOKABLE void sendHex(const QString &hex);
    // 离线自测：把一段十六进制文本当成"串口收到的字节"喂进解析器（不需要真机接线）
    Q_INVOKABLE void injectHex(const QString &hex);

signals:
    void connectedChanged();
    void portNameChanged();
    void portsChanged();
    void baudRateChanged();
    void autoReplyChanged();
    void statsChanged();
    void lastErrorChanged();
    void packetReceived(const QVariantMap &packet);
    void lineReceived(const QString &text);
    void sent(const QString &text);

private slots:
    void onReadyRead();
    void onSerialError(QSerialPort::SerialPortError error);

private:
    void feed(const QByteArray &chunk);
    void feedText(const QByteArray &chunk);
    void handleLine(const QString &line);
    void handleFrame(const QByteArray &data, quint16 recvCrc, bool hasCrc,
                     const QString &source, const QString &raw);
    bool writePayload(const QByteArray &payload);
    void setLastError(const QString &message);

    static quint16 crc16Modbus(const QByteArray &data);

    QSerialPort *m_serial = nullptr;
    QVariantList m_ports;
    QString m_portName;
    int m_baudRate = 9600;
    bool m_connected = false;
    bool m_autoReply = true;
    int m_total = 0;
    int m_ok = 0;
    int m_fail = 0;
    int m_reply = 0;
    QString m_lastError;
    QByteArray m_buf;       // 二进制帧扫描缓冲
    QByteArray m_lineBuf;   // 文本行缓冲
};
