#include "SerialLink.h"

#include <QDateTime>
#include <QRegularExpression>

namespace {
constexpr int kMaxBuffer = 4096;

QString hexByte(char b)
{
    return QStringLiteral("%1").arg(static_cast<quint8>(b), 2, 16, QLatin1Char('0')).toUpper();
}

QString hexWord(quint16 w)
{
    return QStringLiteral("%1").arg(w, 4, 16, QLatin1Char('0')).toUpper();
}

// 板载 CH340 / 常见 USB-TTL 优先排在前面，方便直接选中
bool isUsbSerial(const QSerialPortInfo &info)
{
    const QString name = info.portName().toLower();
    return name.contains(QStringLiteral("wchusb"))
        || name.contains(QStringLiteral("usbserial"))
        || name.contains(QStringLiteral("usbmodem"))
        || name.contains(QStringLiteral("slab"));
}

// macOS 的这几个是虚拟口（调试台 / 蓝牙 / 无线调试），排到最后，避免默认选中它们
bool isVirtualPort(const QSerialPortInfo &info)
{
    const QString name = info.portName().toLower();
    return name.contains(QStringLiteral("debug-console"))
        || name.contains(QStringLiteral("bluetooth"))
        || name.contains(QStringLiteral("wlan-debug"))
        || name.contains(QStringLiteral("incoming-port"));
}
} // namespace

SerialLink::SerialLink(QObject *parent)
    : QObject(parent)
    , m_serial(new QSerialPort(this))
{
    connect(m_serial, &QSerialPort::readyRead, this, &SerialLink::onReadyRead);
    connect(m_serial, &QSerialPort::errorOccurred, this, &SerialLink::onSerialError);
    refreshPorts();
}

// ───────────────────────── 端口管理 ─────────────────────────

void SerialLink::refreshPorts()
{
    const auto infos = QSerialPortInfo::availablePorts();

    QVariantList usbPorts;
    QVariantList otherPorts;
    QVariantList virtualPorts;
    for (const QSerialPortInfo &info : infos) {
        QVariantMap item;
        const QString description = info.description();
        item.insert(QStringLiteral("name"), info.portName());
        item.insert(QStringLiteral("label"),
                    description.isEmpty() ? info.portName()
                                          : QStringLiteral("%1 · %2").arg(info.portName(), description));
        if (isVirtualPort(info))
            virtualPorts.append(item);
        else if (isUsbSerial(info))
            usbPorts.append(item);
        else
            otherPorts.append(item);
    }

    m_ports = usbPorts + otherPorts + virtualPorts;

    // 没有手动指定过端口时，默认选第一个可用串口
    if (m_portName.isEmpty() && !m_ports.isEmpty())
        m_portName = m_ports.first().toMap().value(QStringLiteral("name")).toString();

    emit portsChanged();
    if (!m_ports.isEmpty())
        emit portNameChanged();
}

void SerialLink::setPortName(const QString &name)
{
    if (m_portName == name)
        return;
    m_portName = name;
    emit portNameChanged();
}

int SerialLink::portIndex() const
{
    for (int i = 0; i < m_ports.size(); ++i) {
        if (m_ports.at(i).toMap().value(QStringLiteral("name")).toString() == m_portName)
            return i;
    }
    return -1;
}

bool SerialLink::open()
{
    if (m_serial->isOpen())
        close();

    if (m_portName.isEmpty()) {
        setLastError(QStringLiteral("没有可用串口，请插好 USB-TTL 后点「刷新」"));
        return false;
    }

    m_serial->setPortName(m_portName);
    m_serial->setBaudRate(m_baudRate);
    m_serial->setDataBits(QSerialPort::Data8);
    m_serial->setParity(QSerialPort::NoParity);
    m_serial->setStopBits(QSerialPort::OneStop);
    m_serial->setFlowControl(QSerialPort::NoFlowControl);

    if (!m_serial->open(QIODevice::ReadWrite)) {
        setLastError(QStringLiteral("打开 %1 失败：%2").arg(m_portName, m_serial->errorString()));
        return false;
    }

    m_buf.clear();
    m_lineBuf.clear();
    setLastError(QString());
    m_connected = true;
    emit connectedChanged();
    emit lineReceived(QStringLiteral("[系统] 已打开 %1 @ %2 8N1").arg(m_portName).arg(m_baudRate));
    return true;
}

void SerialLink::close()
{
    if (!m_serial->isOpen())
        return;
    m_serial->close();
    m_connected = false;
    emit connectedChanged();
    emit lineReceived(QStringLiteral("[系统] 已关闭 %1").arg(m_portName));
}

void SerialLink::clearStats()
{
    m_total = m_ok = m_fail = m_reply = 0;
    emit statsChanged();
}

void SerialLink::setAutoReply(bool on)
{
    if (m_autoReply == on)
        return;
    m_autoReply = on;
    emit autoReplyChanged();
    emit lineReceived(on ? QStringLiteral("[系统] 已开启自动回复（校验通过 → OK / 失败 → ERR）")
                         : QStringLiteral("[系统] 已关闭自动回复"));
}

void SerialLink::setLastError(const QString &message)
{
    if (m_lastError == message)
        return;
    m_lastError = message;
    emit lastErrorChanged();
}

void SerialLink::onSerialError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::NoError)
        return;

    // 拔线 / 掉驱动：直接按断开处理，给用户一句人话
    if (error == QSerialPort::ResourceError || error == QSerialPort::DeviceNotFoundError
        || error == QSerialPort::PermissionError) {
        setLastError(QStringLiteral("串口异常：%1").arg(m_serial->errorString()));
        if (m_serial->isOpen()) {
            m_serial->close();
            m_connected = false;
            emit connectedChanged();
        }
    }
}

// ───────────────────────── 发送 ─────────────────────────

bool SerialLink::writePayload(const QByteArray &payload)
{
    if (!m_serial->isOpen()) {
        setLastError(QStringLiteral("串口未打开，无法发送"));
        return false;
    }
    m_serial->write(payload);
    m_serial->flush();
    return true;
}

void SerialLink::sendText(const QString &text)
{
    if (!m_serial->isOpen()) {
        setLastError(QStringLiteral("串口未打开，无法发送"));
        return;
    }
    QByteArray payload = text.toUtf8();
    if (!payload.endsWith("\r\n"))
        payload += "\r\n";
    if (writePayload(payload))
        emit sent(QStringLiteral("%1").arg(QString::fromUtf8(payload).trimmed()));
}

void SerialLink::sendHex(const QString &hex)
{
    if (!m_serial->isOpen()) {
        setLastError(QStringLiteral("串口未打开，无法发送"));
        return;
    }
    const QByteArray payload = QByteArray::fromHex(hex.toLatin1());
    if (payload.isEmpty()) {
        setLastError(QStringLiteral("十六进制内容无效：%1").arg(hex));
        return;
    }
    if (writePayload(payload))
        emit sent(QStringLiteral("[hex] %1").arg(payload.toHex(' ').toUpper()));
}

void SerialLink::injectHex(const QString &hex)
{
    const QByteArray bytes = QByteArray::fromHex(hex.toLatin1());
    if (bytes.isEmpty()) {
        setLastError(QStringLiteral("自测数据无效：%1").arg(hex));
        return;
    }
    emit lineReceived(QStringLiteral("[自测] 注入 %1 字节：%2")
                          .arg(bytes.size())
                          .arg(QString::fromLatin1(bytes.toHex(' ').toUpper())));
    feed(bytes);
}

// ───────────────────────── 接收 / 解析 ─────────────────────────

void SerialLink::onReadyRead()
{
    const QByteArray chunk = m_serial->readAll();
    if (!chunk.isEmpty())
        feed(chunk);
}

void SerialLink::feed(const QByteArray &chunk)
{
    m_buf.append(chunk);

    // 1) 二进制帧：0xFF + 4 数据 + 2 CRC + 0xFE（定长 8 字节，CRC 不过也能靠包尾定位）
    for (;;) {
        const int start = m_buf.indexOf(static_cast<char>(0xFF));
        if (start < 0)
            break;

        if (start > 0) {                       // 包头之前的字节属于文本
            feedText(m_buf.left(start));
            m_buf.remove(0, start);
        }
        if (m_buf.size() < 8)                  // 帧还没收全，等下一批
            break;

        const QByteArray frame = m_buf.left(8);
        if (static_cast<quint8>(frame.at(7)) == 0xFE) {
            const QByteArray data = frame.mid(1, 4);
            const quint16 recvCrc = static_cast<quint16>(
                (static_cast<quint8>(frame.at(6)) << 8) | static_cast<quint8>(frame.at(5)));
            handleFrame(data, recvCrc, true, QStringLiteral("二进制帧"),
                        frame.toHex(' ').toUpper());
            m_buf.remove(0, 8);
        } else {
            m_buf.remove(0, 1);                // 伪包头，丢一个字节重新同步
        }
    }

    // 2) 剩下的按文本处理，只取到最后一个换行为止，尾部残行留到下一批
    if (!m_buf.isEmpty()) {
        const int nl = m_buf.lastIndexOf('\n');
        if (nl >= 0) {
            feedText(m_buf.left(nl + 1));
            m_buf.remove(0, nl + 1);
        }
    }

    if (m_buf.size() > kMaxBuffer)
        m_buf.remove(0, m_buf.size() - kMaxBuffer);
}

void SerialLink::feedText(const QByteArray &chunk)
{
    m_lineBuf.append(chunk);

    int nl = -1;
    while ((nl = m_lineBuf.indexOf('\n')) >= 0) {
        const QByteArray rawLine = m_lineBuf.left(nl);
        m_lineBuf.remove(0, nl + 1);
        handleLine(QString::fromLatin1(rawLine).trimmed());
    }

    if (m_lineBuf.size() > kMaxBuffer)
        m_lineBuf.remove(0, m_lineBuf.size() - kMaxBuffer);
}

void SerialLink::handleLine(const QString &line)
{
    if (line.isEmpty())
        return;

    emit lineReceived(QStringLiteral("← %1").arg(line));

    // 兼容 F407 现有固件：RX: 05 06 07 08  CRC=1A2B
    static const QRegularExpression re(
        QStringLiteral("^RX:\\s*([0-9A-Fa-f]{2})\\s+([0-9A-Fa-f]{2})\\s+"
                       "([0-9A-Fa-f]{2})\\s+([0-9A-Fa-f]{2})\\s*"
                       "(?:CRC\\s*=\\s*([0-9A-Fa-f]{1,4}))?"),
        QRegularExpression::CaseInsensitiveOption);

    const QRegularExpressionMatch m = re.match(line);
    if (!m.hasMatch())
        return;                                // 其它文本（Ready / alive / PC cmd）只进日志

    QByteArray data;
    for (int i = 1; i <= 4; ++i)
        data.append(static_cast<char>(m.captured(i).toUInt(nullptr, 16)));

    const QString crcText = m.captured(5);
    const bool hasCrc = !crcText.isEmpty();
    const quint16 recvCrc = hasCrc ? static_cast<quint16>(crcText.toUInt(nullptr, 16)) : 0;

    handleFrame(data, recvCrc, hasCrc, QStringLiteral("文本行"), line);
}

void SerialLink::handleFrame(const QByteArray &data, quint16 recvCrc, bool hasCrc,
                             const QString &source, const QString &raw)
{
    if (data.size() < 4)
        return;

    const QByteArray payload = data.left(4);
    const quint16 calcCrc = crc16Modbus(payload);
    const bool ok = hasCrc ? (calcCrc == recvCrc) : true;

    ++m_total;
    ok ? ++m_ok : ++m_fail;

    QVariantMap packet;
    packet.insert(QStringLiteral("idx"), m_total);
    packet.insert(QStringLiteral("time"), QDateTime::currentDateTime().toString(QStringLiteral("HH:mm:ss.zzz")));
    packet.insert(QStringLiteral("data"), payload.toHex(' ').toUpper());
    packet.insert(QStringLiteral("d0"), hexByte(payload.at(0)));
    packet.insert(QStringLiteral("d1"), hexByte(payload.at(1)));
    packet.insert(QStringLiteral("d2"), hexByte(payload.at(2)));
    packet.insert(QStringLiteral("d3"), hexByte(payload.at(3)));
    packet.insert(QStringLiteral("crc"), hasCrc ? hexWord(recvCrc) : QStringLiteral("--"));
    packet.insert(QStringLiteral("calc"), hexWord(calcCrc));
    packet.insert(QStringLiteral("ok"), ok);
    packet.insert(QStringLiteral("result"), ok ? QStringLiteral("通过") : QStringLiteral("失败"));
    packet.insert(QStringLiteral("source"), source);
    packet.insert(QStringLiteral("raw"), raw);

    emit packetReceived(packet);
    emit statsChanged();
    emit lineReceived(QStringLiteral("[校验] %1  CRC=%2  计算=%3  → %4")
                          .arg(packet.value(QStringLiteral("data")).toString(),
                               packet.value(QStringLiteral("crc")).toString(),
                               packet.value(QStringLiteral("calc")).toString(),
                               ok ? QStringLiteral("通过") : QStringLiteral("失败")));

    if (m_autoReply) {
        const QByteArray reply = ok ? QByteArrayLiteral("OK\r\n") : QByteArrayLiteral("ERR\r\n");
        // 只有真的写出去了，才计入「已回复」
        if (writePayload(reply)) {
            ++m_reply;
            emit statsChanged();
            emit sent(QString::fromLatin1(reply).trimmed());
        }
    }
}

// ───────────────────────── CRC-16/MODBUS ─────────────────────────
// 多项式 0x8005（反射 0xA001），初值 0xFFFF，与 F103/F407 端实现一致
quint16 SerialLink::crc16Modbus(const QByteArray &data)
{
    quint16 crc = 0xFFFF;
    for (char ch : data) {
        crc ^= static_cast<quint8>(ch);
        for (int bit = 0; bit < 8; ++bit)
            crc = (crc & 0x0001) ? static_cast<quint16>((crc >> 1) ^ 0xA001)
                                 : static_cast<quint16>(crc >> 1);
    }
    return crc;
}
