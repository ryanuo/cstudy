#include <QFont>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>

#include "SerialLink.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QQuickStyle::setStyle("Basic");   // QtShadcn 自绘 token 的前提，必做

    // 中文界面：显式指定字体族，避免 macOS 上回落到 "Sans Serif" 去做别名匹配
    QFont appFont = app.font();
    appFont.setFamilies({"PingFang SC", "Helvetica Neue", "sans-serif"});
    app.setFont(appFont);

    QQmlApplicationEngine engine;

    // 串口链路实例：QML 侧统一用 serialLink.xxx 访问（避免与子组件属性重名）
    SerialLink serialLink;
    engine.rootContext()->setContextProperty("serialLink", &serialLink);

    engine.loadFromModule("QtLink", "Main");
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
