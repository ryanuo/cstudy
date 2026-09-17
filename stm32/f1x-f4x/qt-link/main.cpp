#include <QDebug>
#include <QFont>
#include <QGuiApplication>
#include <QImage>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QTimer>

#include "SerialLink.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QQuickStyle::setStyle("Basic");   // QtShadcn 自绘 token 的前提，必做

    // 中文界面：显式指定字体族，避免 macOS 上回落到 "Sans Serif" 去做别名匹配
    // Menlo 排最前：十六进制/时间戳/CRC 全部走等宽（ShadcnTable 的单元格字体在库里写死，
    // 只能靠应用默认字体族改），中文没有等宽字形会自动回落到 PingFang SC
    QFont appFont = app.font();
    appFont.setFamilies({"Menlo", "PingFang SC", "Helvetica Neue", "sans-serif"});
    app.setFont(appFont);

    QQmlApplicationEngine engine;

    // 串口链路实例：QML 侧统一用 serialLink.xxx 访问（避免与子组件属性重名）
    SerialLink serialLink;
    engine.rootContext()->setContextProperty("serialLink", &serialLink);

    engine.loadFromModule("QtLink", "Main");
    if (engine.rootObjects().isEmpty())
        return -1;

    // 调试用（见 README）：QTLINK_GRAB=1 启动会把窗口渲染结果存成 /tmp/qtlink_shot.png
    // 离屏软件渲染还需 QTSHADCN_SCREENSHOT=1，否则 ShadcnCard 的 GPU 阴影层不上屏、卡片空白
    if (qEnvironmentVariableIsSet("QTLINK_GRAB")) {
        QTimer::singleShot(2600, &app, [&engine, &serialLink]() {
            if (engine.rootObjects().isEmpty())
                return;
            auto *win = qobject_cast<QQuickWindow *>(engine.rootObjects().first());
            if (!win)
                return;
            const QImage img = win->grabWindow();
            qInfo() << "[shot] saved =" << img.save("/tmp/qtlink_shot.png") << img.size();
        });
    }

    return app.exec();
}
