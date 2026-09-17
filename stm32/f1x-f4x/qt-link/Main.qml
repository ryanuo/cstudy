import QtQuick
import QtQuick.Effects
import QtQuick.Layouts
import QtQuick.Controls.Basic
import QtShadcn

// 上位机（PC）监测台：
//   F103 --(USART1)-- F407 USART2 --(转发)-- F407 USART1 --(USB-TTL)-- 本程序
// 收到 F407 上送的数据包 → 本地重算 CRC-16/MODBUS 校验 → 显示 → 校验通过回 "OK"（失败回 "ERR"）
ApplicationWindow {
    id: root
    // 初始尺寸取屏幕可用区域（这台屏 1440×900：原来定死 800 高会超出，底部日志看不见）
    // 注意：某些平台在 Component.onCompleted 时 screen.availableWidth 还是 0，必须兜底，
    // 否则 width 会算成 1×1；屏幕信息稍后就绪时绑定会自动重算
    readonly property real _usableW: (screen && screen.availableWidth > 200) ? screen.availableWidth : 1200
    readonly property real _usableH: (screen && screen.availableHeight > 200) ? screen.availableHeight : 820
    width: Math.max(minimumWidth, Math.min(1180, Math.round(_usableW - 40)))
    height: Math.max(minimumHeight, Math.min(820, Math.round(_usableH - 60)))
    minimumWidth: 880
    minimumHeight: 620
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

    // 串口下拉的模型：必须是**真正的 JS 字符串数组**。
    // 坑：ShadcnSelect 的委托用 `Array.isArray(model)` 判断，
    //     C++ 的 QVariantList/QStringList 传到 QML 后 isArray 为 false，
    //     会走 `String(model[textRole])` 分支 → 列表每项显示 "undefined"。
    property var portLabels: []

    function rebuildPortModel() {
        var list = serialLink.ports
        var labels = []
        for (var i = 0; i < list.length; ++i)
            labels.push(String(list[i].label))
        portLabels = labels
        portSelect.currentIndex = serialLink.portIndex
    }

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
        rebuildPortModel()
        logLine("[系统] 就绪 · 协议 FF D0 D1 D2 D3 CRC_L CRC_H FE（同时兼容文本行 RX: xx xx xx xx  CRC=xxxx）")
    }

    // ── 内联子组件：统计卡片（不引用外层 id，颜色由调用处传入）──
    component StatCard: ShadcnCard {
        property string title: ""
        property string value: "0"
        property color valueColor: "#111827"

        size: ShadcnCard.Size.Small
        // ShadcnCard 自带 implicitWidth: 320，作为布局项会变成"最小宽 320"，
        // 4 张卡叠加就把整行顶到 1316 → 内容横向溢出窗口，所以必须放开最小宽度并让它均分宽度
        Layout.fillWidth: true
        Layout.minimumWidth: 0

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
        property bool emphasized: false      // 关键字段（数据）加大加粗
        property bool mono: false            // 时间/十六进制/CRC/报文 → 等宽，纵向对得齐
        property bool boxed: false           // 原始报文 → 浅灰代码块容器

        spacing: 10
        ShadcnLabel {
            text: labelText
            size: ShadcnLabel.Size.Small
            variant: ShadcnLabel.Variant.Muted
            Layout.preferredWidth: 76          // Label 固定宽 + 左对齐 → 标签与值各成一列栅格
            horizontalAlignment: Text.AlignLeft
        }
        TextEdit {
            visible: !boxed
            text: valueText
            readOnly: true
            selectByMouse: true
            persistentSelection: true
            wrapMode: TextEdit.WrapAnywhere
            color: valueColor
            selectionColor: Qt.rgba(0.2, 0.4, 0.8, 0.25)
            font.pixelSize: emphasized ? 15 : 12
            font.bold: emphasized
            font.family: (mono || emphasized) ? "Menlo" : ""   // 空串 = 用默认字体
            Layout.fillWidth: true
            Layout.minimumWidth: 0
            height: Math.max(emphasized ? 22 : 18, contentHeight)
        }
        // boxed：等宽 + 浅灰底 + 圆角 4 + 内边距 6/8，长报文换行不挤压（对齐 shadcn 的 code block）
        Rectangle {
            visible: boxed
            Layout.fillWidth: true
            Layout.minimumWidth: 0
            implicitHeight: boxedValue.implicitHeight + 12
            radius: 4
            color: theme.muted
            border.width: 1
            border.color: theme.border
            TextEdit {
                id: boxedValue
                anchors.fill: parent
                anchors.topMargin: 6
                anchors.bottomMargin: 6
                anchors.leftMargin: 8
                anchors.rightMargin: 8
                text: valueText
                readOnly: true
                selectByMouse: true
                persistentSelection: true
                wrapMode: TextEdit.WrapAnywhere
                color: valueColor
                selectionColor: Qt.rgba(0.2, 0.4, 0.8, 0.25)
                font.family: "Menlo"
                font.pixelSize: 12
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 14
        spacing: 10

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
            id: connCard
            size: ShadcnCard.Size.Small
            Layout.fillWidth: true

            Column {
                width: parent.width
                spacing: 8

                // 2.1 串口 / 波特率 / 连接
                RowLayout {
                    width: parent.width
                    spacing: 8

                    ShadcnLabel { text: qsTr("串口"); size: ShadcnLabel.Size.Small; variant: ShadcnLabel.Variant.Muted }

                    ShadcnSelect {
                        id: portSelect
                        Layout.fillWidth: true
                        Layout.minimumWidth: 0     // 端口名可能很长（cu.wchusbserial1234 · USB-Serial）
                        model: root.portLabels
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
                        Layout.minimumWidth: 120
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
            id: statsRow
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
            id: mainRow
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 12

            ColumnLayout {
                id: tableCol
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 8

                RowLayout {
                    id: tableHeader
                    Layout.fillWidth: true
                    Layout.preferredHeight: 24        // 与右列标题行同高 → 两侧卡片顶边齐平
                    spacing: 8
                    ShadcnLabel { text: qsTr("接收到的数据包") }
                    ShadcnBadge {
                        text: qsTr("%1 条").arg(packetRows.length)
                        variant: ShadcnBadge.Variant.Outline
                        Layout.alignment: Qt.AlignVCenter
                    }
                    Item { Layout.fillWidth: true }
                }

                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.minimumHeight: 120

                    ShadcnTable {
                        id: tableItem
                        anchors.fill: parent
                        model: packetModel
                        onRowClicked: function(row) { selectedPacket = packetModel.getRow(row) }
                    }

                    // 空状态：没数据时给一句提示，免得大片空白看着像坏了
                    ShadcnLabel {
                        anchors.centerIn: parent
                        visible: root.packetRows.length === 0
                        text: qsTr("暂无数据 · 等 F407 上报（收到后点任意一行看详情）")
                        size: ShadcnLabel.Size.Small
                        variant: ShadcnLabel.Variant.Muted
                    }
                }

                ShadcnTableModel {
                    id: packetModel
                    columns: [
                        // D0~D3 合成一列 PAYLOAD (HEX)：列少了间距自然紧凑，也腾出宽度给数据
                        // 宽度按表格实际宽度按比例分配：库的列宽写死后 totalWidth = max(列宽和, 容器宽)，
                        // 列宽和小于容器宽时，表头底线与行分隔线会在表格右侧断掉
                        // 数值列一律 align: "right"（库的 "center" 实际是空操作：Text 只锚 left，
                        // 宽度=内容宽，HCenter 无效），值靠右对齐后表头/内容垂向成一条线
                        { key: "idx",    title: "#",             width: Math.round(tableItem.width * 0.07), align: "right" },
                        { key: "time",   title: qsTr("时间"),     width: Math.round(tableItem.width * 0.18), align: "right" },
                        { key: "data",   title: "PAYLOAD (HEX)", width: Math.round(tableItem.width * 0.38), align: "right" },
                        { key: "crc",    title: "CRC",           width: Math.round(tableItem.width * 0.19), align: "right" },
                        { key: "check",  title: qsTr("校验"),     width: Math.round(tableItem.width * 0.17), align: "right" }
                    ]
                    rows: root.packetRows
                }
            }

            ColumnLayout {
                id: rightCol
                Layout.preferredWidth: 350
                Layout.minimumWidth: 320
                Layout.maximumWidth: 400        // 卡住右列宽度，避免它抢走表格的宽度
                Layout.fillHeight: true
                // 必须显式顶部对齐：这一列的内容高（标题 24 + 卡 216 + 间距）小于 mainRow 高，
                // 实测单靠 fillHeight 不会把本列撑开，RowLayout 会把它垂直居中 → 整列下移 66px
                Layout.alignment: Qt.AlignTop
                spacing: 8

                // 4.1 本包详情：与左列完全同构 —— 标题行（对齐「接收到的数据包」）+ 卡片框（对齐表格框）
                RowLayout {
                    id: detailsHeader
                    Layout.fillWidth: true
                    Layout.preferredHeight: 24        // 与左列标题行同高 → 两侧卡片顶边齐平
                    spacing: 8
                    ShadcnLabel { text: qsTr("本包详情") }
                    Item { Layout.fillWidth: true }
                    // 状态胶囊：成功=浅绿底深绿字（比实心黑温和，不抢主视觉）；失败=浅红底深红字
                    Rectangle {
                        id: statusPill
                        Layout.alignment: Qt.AlignVCenter
                        readonly property bool noData: selectedPacket.result === undefined
                        readonly property bool pass: selectedPacket.result === "通过"

                        implicitWidth: statusPillText.implicitWidth + 18
                        implicitHeight: 22
                        radius: 999
                        color: noData ? theme.muted
                                      : (pass ? Qt.rgba(theme.success.r, theme.success.g, theme.success.b, 0.14)
                                              : Qt.rgba(theme.destructive.r, theme.destructive.g, theme.destructive.b, 0.14))

                        Text {
                            id: statusPillText
                            anchors.centerIn: parent
                            text: statusPill.noData
                                  ? qsTr("暂无数据")
                                  : (statusPill.pass ? qsTr("✓ 校验通过") : qsTr("✗ 校验失败"))
                            color: statusPill.noData ? theme.mutedForeground
                                                     : (statusPill.pass ? theme.success : theme.destructive)
                            font.pixelSize: 12
                            font.weight: Font.Medium
                        }
                    }
                }

                // 卡片外观 + 与表格同一种描边（theme.border）：原来 ShadcnCard 的 5% 淡环
                // 跟表格的实线边框是两种强度，导致「主次区域划分不清晰」
                Rectangle {
                    id: detailsCard
                    Layout.fillWidth: true
                    implicitHeight: detailsCol.implicitHeight + 28      // 贴内容高度，不拉满（避免卡内大片空白）
                    color: theme.card
                    radius: theme.radius
                    border.width: 1
                    border.color: theme.border
                    clip: true

                    // 与 ShadcnCard 同款阴影，保持卡片观感
                    layer.enabled: !ThemeManager.screenshotMode()
                    layer.effect: MultiEffect {
                        shadowEnabled: true
                        shadowBlur: 0.4
                        shadowVerticalOffset: 2
                        shadowColor: Qt.rgba(0, 0, 0, theme.mode === "dark" ? 0.35 : 0.08)
                    }

                    Column {
                        id: detailsCol
                        anchors.fill: parent
                        anchors.margins: 14
                        spacing: 6

                        DetailRow { labelText: qsTr("序号");     valueText: root.fieldOf("idx");    valueColor: theme.foreground; mono: true }
                        DetailRow { labelText: qsTr("接收时间"); valueText: root.fieldOf("time");   valueColor: theme.foreground; mono: true }
                        DetailRow {
                            labelText: qsTr("数据")
                            valueText: root.fieldOf("data")
                            valueColor: theme.foreground
                            emphasized: true                    // 最核心的信息，放大加粗等宽
                        }
                        DetailRow { labelText: qsTr("CRC 收到"); valueText: root.fieldOf("crc");    valueColor: theme.foreground; mono: true }
                        DetailRow { labelText: qsTr("CRC 重算"); valueText: root.fieldOf("calc");   valueColor: theme.foreground; mono: true }
                        DetailRow { labelText: qsTr("结果");     valueText: root.fieldOf("result"); valueColor: theme.foreground }
                        DetailRow { labelText: qsTr("来源");     valueText: root.fieldOf("source"); valueColor: theme.foreground }
                        DetailRow {
                            labelText: qsTr("原始报文")
                            valueText: root.fieldOf("raw")
                            valueColor: theme.foreground
                            boxed: true                         // 浅灰代码块容器，长报文换行不挤压
                        }
                    }
                }

            }
        }

        // ───────────────────────── 5. 原始日志条（整宽 · 3 行高 · 可选中复制）─────────────────────────
        Rectangle {
            id: logCard
            Layout.fillWidth: true
            Layout.preferredHeight: 46 + 18 + 6 + 20      // 正文 3×~15px + 紧凑标题行 + 间距 + 上下内边距
            color: theme.card
            radius: theme.radius
            border.width: 1
            border.color: theme.border                     // 与表格/详情卡同一种描边
            clip: true

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 6

                RowLayout {
                    Layout.fillWidth: true
                    Layout.minimumHeight: 18
                    ShadcnLabel {
                        text: qsTr("原始日志")
                        size: ShadcnLabel.Size.Small
                        variant: ShadcnLabel.Variant.Muted
                    }
                    Item { Layout.fillWidth: true }
                    ShadcnLabel {
                        text: qsTr("接收 / 校验 / 回复")
                        size: ShadcnLabel.Size.Small
                        variant: ShadcnLabel.Variant.Muted
                    }
                }

                Flickable {
                    id: logFlick
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.minimumWidth: 0     // 同上：Flickable 的隐式宽来自内容，必须允许收缩
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

    // ───────────────────────── 串口事件 → 界面 ─────────────────────────
    Connections {
        target: serialLink

        function onPortsChanged() {
            root.rebuildPortModel()
        }

        function onPacketReceived(packet) {
            // 注意：右侧「本包详情」是用 packetModel.getRow(row) 取的数据（点击行时），
            // 所以这里必须放**完整**字段；表格只显示 columns 里声明的那几列
            var row = {
                "idx": packet.idx,
                "time": packet.time,
                "data": packet.data,        // ← 详情「数据」行读的就是它，漏了会永远显示 "—"
                "d0": packet.d0,
                "d1": packet.d1,
                "d2": packet.d2,
                "d3": packet.d3,
                "crc": packet.crc,
                "calc": packet.calc,
                "result": packet.result,
                "check": packet.ok ? "✓" : "✗ 失败",   // 表格用：正常态弱化成一个勾，异常态才显眼
                "ok": packet.ok,
                "source": packet.source,
                "raw": packet.raw
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
