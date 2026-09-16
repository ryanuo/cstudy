import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Basic
import QtShadcn

// 上位机（PC）监测台：
//   F103 --(USART1)-- F407 USART2 --(转发)-- F407 USART1 --(USB-TTL)-- 本程序
// 收到 F407 上送的数据包 → 本地重算 CRC-16/MODBUS 校验 → 显示 → 校验通过回 "OK"（失败回 "ERR"）
ApplicationWindow {
    id: root
    width: 1180
    height: 760
    minimumWidth: 1000
    minimumHeight: 640
    visible: true
    title: qsTr("F103 → F407 数据监测台")
    color: theme.background

    // 主题入口（必做）：窗口内所有 QtShadcn 组件共用同一套 token，切 mode 全局随动
    QtShadcnTheme { id: theme }

    // ── 界面数据 ──
    property var packetRows: []           // 表格行（最新在最上面）
    property var selectedPacket: ({})     // 当前选中的包（详情面板）
    property string rawLog: ""            // 原始日志（最新在最上面）
    readonly property int maxRows: 300

    function logLine(text) {
        var next = text + "\n" + rawLog
        rawLog = next.length > 24000 ? next.substring(0, 24000) : next
    }

    function fieldOf(key) {
        var value = selectedPacket[key]
        if (value === undefined || value === null || value === "")
            return "—"
        return String(value)
    }

    Component.onCompleted: {
        portSelect.currentIndex = serialLink.portIndex
        logLine("[系统] 就绪 · 协议 FF D0 D1 D2 D3 CRC_L CRC_H FE（同时兼容文本行 RX: xx xx xx xx  CRC=xxxx）")
    }

    // ── 内联子组件：统计卡片（不引用外层 id，颜色由调用处传入）──
    component StatCard: ShadcnCard {
        property string title: ""
        property string value: "0"
        property color valueColor: "#111827"

        size: ShadcnCard.Size.Small

        Column {
            width: parent.width
            spacing: 2
            ShadcnLabel { text: title; size: ShadcnLabel.Size.Small; variant: ShadcnLabel.Variant.Muted }
            ShadcnLabel { text: value; size: ShadcnLabel.Size.Large; color: valueColor }
        }
    }

    // ── 内联子组件：详情面板的一行（值可鼠标选中复制）──
    // 注：这个 Qt 的 Text 没有文本选择能力，TextEdit 也没有 textInteractionFlags 属性
    //     （实测 dylib 里根本没这个属性名），可选中的只读文本用 readOnly + selectByMouse
    component DetailRow: RowLayout {
        property string labelText: ""
        property string valueText: ""
        property color valueColor: "#111827"

        spacing: 8
        ShadcnLabel {
            text: labelText
            size: ShadcnLabel.Size.Small
            variant: ShadcnLabel.Variant.Muted
            Layout.preferredWidth: 70
        }
        TextEdit {
            text: valueText
            readOnly: true
            selectByMouse: true
            persistentSelection: true
            wrapMode: TextEdit.WrapAnywhere
            color: valueColor
            selectionColor: Qt.rgba(0.2, 0.4, 0.8, 0.25)
            font.pixelSize: 12
            Layout.fillWidth: true
            height: Math.max(18, contentHeight)
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 14

        // ───────────────────────── 1. 标题 ─────────────────────────
        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            ShadcnIcon { name: "monitor"; size: 22 }
            ShadcnLabel { text: qsTr("F103 → F407 数据监测台"); size: ShadcnLabel.Size.Large }
            ShadcnBadge { text: qsTr("CRC-16/MODBUS"); variant: ShadcnBadge.Variant.Secondary }

            Item { Layout.fillWidth: true }

            ShadcnButton {
                text: theme.mode === "dark" ? qsTr("浅色") : qsTr("深色")
                iconName: theme.mode === "dark" ? "sun" : "moon"
                variant: ShadcnButton.Variant.Outline
                size: ShadcnButton.Size.Small
                onClicked: theme.mode = theme.mode === "dark" ? "light" : "dark"
            }
        }

        // ───────────────────────── 2. 连接与控制（重要入口在顶部）─────────────────────────
        ShadcnCard {
            size: ShadcnCard.Size.Small
            Layout.fillWidth: true

            Column {
                width: parent.width
                spacing: 10

                // 2.1 串口 / 波特率 / 连接
                RowLayout {
                    width: parent.width
                    spacing: 8

                    ShadcnLabel { text: qsTr("串口"); size: ShadcnLabel.Size.Small; variant: ShadcnLabel.Variant.Muted }

                    ShadcnSelect {
                        id: portSelect
                        Layout.fillWidth: true
                        model: serialLink.ports
                        textRole: "label"
                        onActivated: serialLink.portName = serialLink.ports[currentIndex].name
                    }

                    ShadcnButton {
                        text: qsTr("刷新")
                        iconName: "refresh-cw"
                        variant: ShadcnButton.Variant.Outline
                        size: ShadcnButton.Size.Small
                        onClicked: serialLink.refreshPorts()
                    }

                    ShadcnLabel { text: qsTr("波特率"); size: ShadcnLabel.Size.Small; variant: ShadcnLabel.Variant.Muted }

                    ShadcnSelect {
                        id: baudSelect
                        Layout.preferredWidth: 110
                        model: ["9600", "19200", "38400", "57600", "115200"]
                        currentIndex: 0
                        onActivated: serialLink.baudRate = parseInt(currentText)
                    }

                    ShadcnButton {
                        text: serialLink.connected ? qsTr("断开") : qsTr("连接")
                        variant: serialLink.connected ? ShadcnButton.Variant.Destructive
                                                     : ShadcnButton.Variant.Primary
                        size: ShadcnButton.Size.Small
                        onClicked: serialLink.connected ? serialLink.close() : serialLink.open()
                    }

                    ShadcnStatusDot {
                        status: serialLink.connected ? ShadcnStatusDot.Status.Online
                                                     : ShadcnStatusDot.Status.Offline
                    }
                    ShadcnLabel {
                        text: serialLink.connected ? serialLink.portName : qsTr("未连接")
                        size: ShadcnLabel.Size.Small
                    }
                }

                // 2.2 自动回复 / 手动发送
                RowLayout {
                    width: parent.width
                    spacing: 8

                    ShadcnLabel { text: qsTr("自动回复"); size: ShadcnLabel.Size.Small; variant: ShadcnLabel.Variant.Muted }
                    ShadcnSwitch {
                        id: autoReplySwitch
                        size: ShadcnSwitch.Size.Small
                        checked: serialLink.autoReply
                        onToggled: serialLink.autoReply = checked
                    }
                    ShadcnLabel {
                        text: qsTr("校验通过 → 回 OK，失败 → 回 ERR")
                        size: ShadcnLabel.Size.Small
                        variant: ShadcnLabel.Variant.Muted
                    }

                    Item { Layout.fillWidth: true }

                    ShadcnInput {
                        id: manualInput
                        Layout.preferredWidth: 240
                        enabled: serialLink.connected
                        placeholderText: qsTr("手动发送文本（回车即发）")
                        onAccepted: if (text.length > 0) { serialLink.sendText(text); text = "" }
                    }
                    ShadcnButton {
                        text: qsTr("发送")
                        iconName: "send"
                        size: ShadcnButton.Size.Small
                        enabled: serialLink.connected && manualInput.text.length > 0
                        onClicked: { serialLink.sendText(manualInput.text); manualInput.text = "" }
                    }
                }

                // 2.3 链路自检
                RowLayout {
                    width: parent.width
                    spacing: 8

                    ShadcnLabel { text: qsTr("链路自检"); size: ShadcnLabel.Size.Small; variant: ShadcnLabel.Variant.Muted }

                    ShadcnButton {
                        text: qsTr("发送 0xAA")
                        variant: ShadcnButton.Variant.Outline
                        size: ShadcnButton.Size.Small
                        enabled: serialLink.connected
                        onClicked: serialLink.sendHex("AA")
                    }
                    ShadcnButton {
                        text: qsTr("发送 0x55")
                        variant: ShadcnButton.Variant.Outline
                        size: ShadcnButton.Size.Small
                        enabled: serialLink.connected
                        onClicked: serialLink.sendHex("55")
                    }
                    ShadcnLabel {
                        text: qsTr("（F407 固件里 AA → 回 CC 并点亮 LED1，55 → 回 DD 并熄灭）")
                        size: ShadcnLabel.Size.Small
                        variant: ShadcnLabel.Variant.Muted
                    }

                    ShadcnButton {
                        text: qsTr("自测·正确包")
                        variant: ShadcnButton.Variant.Ghost
                        size: ShadcnButton.Size.Small
                        onClicked: serialLink.injectHex("FF 01 02 03 04 A1 2B FE")
                    }
                    ShadcnButton {
                        text: qsTr("自测·错误包")
                        variant: ShadcnButton.Variant.Ghost
                        size: ShadcnButton.Size.Small
                        onClicked: serialLink.injectHex("FF 01 02 03 04 00 00 FE")
                    }

                    Item { Layout.fillWidth: true }

                    ShadcnButton {
                        text: qsTr("清空统计")
                        iconName: "trash-2"
                        variant: ShadcnButton.Variant.Outline
                        size: ShadcnButton.Size.Small
                        enabled: serialLink.totalCount > 0
                        onClicked: { serialLink.clearStats(); packetRows = []; selectedPacket = ({}) }
                    }
                }

                ShadcnAlert {
                    width: parent.width
                    visible: serialLink.lastError !== ""
                    variant: ShadcnAlert.Variant.Destructive
                    title: qsTr("串口错误")
                    description: serialLink.lastError
                }
            }
        }

        // ───────────────────────── 3. 统计 ─────────────────────────
        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            StatCard {
                title: qsTr("收到总数")
                value: String(serialLink.totalCount)
                valueColor: theme.foreground
            }
            StatCard {
                title: qsTr("校验通过")
                value: String(serialLink.okCount)
                valueColor: theme.success
            }
            StatCard {
                title: qsTr("校验失败")
                value: String(serialLink.failCount)
                valueColor: theme.destructive
            }
            StatCard {
                title: qsTr("已回复")
                value: String(serialLink.replyCount)
                valueColor: theme.primary
            }
        }

        // ───────────────────────── 4. 数据表 + 详情/日志 ─────────────────────────
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 12

            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 8

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8
                    ShadcnLabel { text: qsTr("接收到的数据包") }
                    ShadcnBadge {
                        text: qsTr("%1 条").arg(packetRows.length)
                        variant: ShadcnBadge.Variant.Outline
                    }
                    Item { Layout.fillWidth: true }
                    ShadcnLabel {
                        text: qsTr("点任意一行看详情")
                        size: ShadcnLabel.Size.Small
                        variant: ShadcnLabel.Variant.Muted
                    }
                }

                ShadcnTable {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    model: packetModel
                    onRowClicked: function(row) { selectedPacket = packetModel.getRow(row) }
                }

                ShadcnTableModel {
                    id: packetModel
                    columns: [
                        { key: "idx",    title: "#",    width: 54,  align: "left" },
                        { key: "time",   title: qsTr("时间"), width: 110, align: "left" },
                        { key: "d0",     title: "D0",   width: 52,  align: "center" },
                        { key: "d1",     title: "D1",   width: 52,  align: "center" },
                        { key: "d2",     title: "D2",   width: 52,  align: "center" },
                        { key: "d3",     title: "D3",   width: 52,  align: "center" },
                        { key: "crc",    title: "CRC",  width: 70,  align: "center" },
                        { key: "calc",   title: qsTr("重算"), width: 70, align: "center" },
                        { key: "result", title: qsTr("校验"), width: 70, align: "center" },
                        { key: "source", title: qsTr("来源"), width: 90, align: "left" }
                    ]
                    rows: root.packetRows
                }
            }

            ColumnLayout {
                Layout.preferredWidth: 350
                Layout.fillHeight: true
                spacing: 12

                // 4.1 本包详情（字段可鼠标选中复制）
                ShadcnCard {
                    size: ShadcnCard.Size.Small
                    Layout.fillWidth: true

                    Column {
                        width: parent.width
                        spacing: 8

                        ShadcnCardHeader {
                            ShadcnCardTitle { text: qsTr("本包详情") }
                            ShadcnCardDescription { text: qsTr("字段可鼠标选中复制") }
                        }

                        ShadcnBadge {
                            text: selectedPacket.result === undefined
                                  ? qsTr("暂无数据")
                                  : (selectedPacket.ok === true ? qsTr("✓ 校验通过") : qsTr("✗ 校验失败"))
                            variant: selectedPacket.ok === true ? ShadcnBadge.Variant.Default
                                                                : ShadcnBadge.Variant.Destructive
                        }

                        DetailRow { labelText: qsTr("序号");   valueText: root.fieldOf("idx");  valueColor: theme.foreground }
                        DetailRow { labelText: qsTr("时间");   valueText: root.fieldOf("time"); valueColor: theme.foreground }
                        DetailRow { labelText: qsTr("数据");   valueText: root.fieldOf("data"); valueColor: theme.foreground }
                        DetailRow { labelText: qsTr("收到 CRC"); valueText: root.fieldOf("crc"); valueColor: theme.foreground }
                        DetailRow { labelText: qsTr("重算 CRC"); valueText: root.fieldOf("calc"); valueColor: theme.foreground }
                        DetailRow { labelText: qsTr("结果");   valueText: root.fieldOf("result"); valueColor: theme.foreground }
                        DetailRow { labelText: qsTr("来源");   valueText: root.fieldOf("source"); valueColor: theme.foreground }
                        DetailRow { labelText: qsTr("原始");   valueText: root.fieldOf("raw");  valueColor: theme.foreground }
                    }
                }

                // 4.2 原始日志（可选中复制）
                ShadcnCard {
                    size: ShadcnCard.Size.Small
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    Column {
                        width: parent.width
                        spacing: 8

                        ShadcnCardHeader {
                            ShadcnCardTitle { text: qsTr("原始日志") }
                            ShadcnCardDescription { text: qsTr("接收 / 校验 / 回复") }
                        }

                        Flickable {
                            id: logFlick
                            width: parent.width
                            height: 250
                            clip: true
                            contentWidth: width
                            contentHeight: logEdit.contentHeight
                            boundsBehavior: Flickable.StopAtBounds

                            TextEdit {
                                id: logEdit
                                width: logFlick.width
                                text: root.rawLog
                                readOnly: true
                                selectByMouse: true
                                persistentSelection: true
                                wrapMode: TextEdit.WrapAnywhere
                                color: theme.mutedForeground
                                font.family: "Menlo"
                                font.pixelSize: 11
                            }
                        }
                    }
                }
            }
        }
    }

    // ───────────────────────── 串口事件 → 界面 ─────────────────────────
    Connections {
        target: serialLink

        function onPortsChanged() {
            portSelect.currentIndex = serialLink.portIndex
        }

        function onPacketReceived(packet) {
            var row = {
                "idx": packet.idx,
                "time": packet.time,
                "d0": packet.d0,
                "d1": packet.d1,
                "d2": packet.d2,
                "d3": packet.d3,
                "crc": packet.crc,
                "calc": packet.calc,
                "result": packet.result,
                "source": packet.source
            }
            packetRows = [row].concat(packetRows).slice(0, root.maxRows)
            selectedPacket = packet
        }

        function onLineReceived(line) {
            root.logLine(line)
        }

        function onSent(text) {
            root.logLine("[发送] " + text)
        }
    }
}
