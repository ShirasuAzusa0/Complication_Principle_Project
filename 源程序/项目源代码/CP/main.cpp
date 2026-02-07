/************************************************************************
 * @Copyright (c) 2025 LDL. All rights reserved.
 *
 * @FileName: main.cpp
 * @Brief: 应用程序入口文件，负责初始化Qt环境、高DPI配置、窗口尺寸设置并启动主窗口
 * @Module: 主界面模块
 *
 * @Current Version: 1.1.0
 * @Author: 袁知本
 * @Created Date: 2025/10/5
 *
 * @Revision History:
 *   Version  Date        Author      Description
 *   -------  ----------  ----------  -------------------
 *   1.0.0    2025/10/5   袁知本       初始版本创建
 *   1.1.0    2025/10/30  袁知本       调整更合理的屏幕尺寸
 *   2.0.0    2026/1/8    袁知本       重构编译原理课设项目布局结构
 ***********************************************************************/

#include "widget.h"
#include <QApplication>
#include <QScreen>
#include <QRect>

int main(int argc, char *argv[])
{
    // 保留高DPI支持，但限制过度缩放
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    // 关键：设置缩放策略为"舍入到整数倍"避免模糊
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::RoundPreferFloor
    );

    // 环境变量精确控制
    //qputenv("QT_SCALE_FACTOR", "1.5");  // 强制1:1缩放
    //qputenv("QT_FONT_DPI", "96");       // 固定字体DPI

    QApplication a(argc, argv);
    Widget w;

    // 获取屏幕尺寸并设置合理大小
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect rect = screen->availableGeometry();
    w.resize(rect.width() * 0.72, rect.height() * 0.73); // 用80%屏幕大小

    w.show();
    return a.exec();
}
